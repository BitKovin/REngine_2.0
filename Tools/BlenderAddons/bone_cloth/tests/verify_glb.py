"""Reads an exported .glb the way the engine's loader does (skeleton = every node under the scene roots, weights from the
skinned primitives) and applies the cloth rules of engine_contract. Needs numpy only.

python tests/verify_glb.py model.glb      (reads model.glb.skmm beside it)"""
import json
import os
import struct
import sys

import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
import engine_contract as ec  # noqa: E402

COMP = {5120: np.int8, 5121: np.uint8, 5122: np.int16, 5123: np.uint16, 5125: np.uint32, 5126: np.float32}
NCOMP = {"SCALAR": 1, "VEC2": 2, "VEC3": 3, "VEC4": 4, "MAT4": 16}


class Glb:
    def __init__(self, path):
        d = open(path, "rb").read()
        length = struct.unpack("<I", d[12:16])[0]
        self.j = json.loads(d[20:20 + length])
        off = 20 + length
        blen = struct.unpack("<I", d[off:off + 4])[0]
        self.bin = d[off + 8:off + 8 + blen]

    def accessor(self, i):
        a = self.j["accessors"][i]
        bv = self.j["bufferViews"][a["bufferView"]]
        dt = COMP[a["componentType"]]
        n = NCOMP[a["type"]]
        start = bv.get("byteOffset", 0) + a.get("byteOffset", 0)
        stride = bv.get("byteStride", n * np.dtype(dt).itemsize)
        raw = np.frombuffer(self.bin, dtype=np.uint8, count=stride * a["count"], offset=start) if False else None
        out = np.zeros((a["count"], n), dtype=dt)
        item = np.dtype(dt).itemsize * n
        for k in range(a["count"]):
            out[k] = np.frombuffer(self.bin, dtype=dt, count=n, offset=start + k * stride)
        if a.get("normalized"):
            out = out.astype(np.float64) / np.iinfo(dt).max
        return out


def node_local(node):
    if "matrix" in node:
        return np.array(node["matrix"], dtype=float).reshape(4, 4).T
    t = np.array(node.get("translation", [0, 0, 0]), dtype=float)
    q = node.get("rotation", [0, 0, 0, 1])
    s = np.array(node.get("scale", [1, 1, 1]), dtype=float)
    x, y, z, w = q
    R = np.array([[1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)],
                  [2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)],
                  [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)]])
    M = np.eye(4)
    M[:3, :3] = R * s
    M[:3, 3] = t
    return M


def load(path):
    g = Glb(path)
    j = g.j
    nodes = j["nodes"]
    parent_of = {}
    for i, n in enumerate(nodes):
        for c in n.get("children", []):
            parent_of[c] = i
    order = []

    def dfs(i, p):
        order.append((i, p))
        for c in nodes[i].get("children", []):
            dfs(c, i)

    scene = j["scenes"][j.get("scene", 0)]
    for r in scene["nodes"]:
        dfs(r, -1)
    index = {n: k for k, (n, _) in enumerate(order)}
    names = [nodes[n].get("name", "") for n, _ in order]
    parent = [index[p] if p >= 0 else -1 for _, p in order]
    world = np.zeros((len(order), 4, 4))
    for k, (n, p) in enumerate(order):
        local = node_local(nodes[n])
        world[k] = (world[index[p]] if p >= 0 else np.eye(4)) @ local
    skel = ec.Skeleton(names, parent, world)

    P, B, W = [], [], []
    for ni, n in enumerate(nodes):
        if "mesh" not in n or "skin" not in n:
            continue
        skin = j["skins"][n["skin"]]
        joints = skin["joints"]
        inv = g.accessor(skin["inverseBindMatrices"]).reshape(-1, 4, 4).transpose(0, 2, 1)
        for prim in j["meshes"][n["mesh"]]["primitives"]:
            at = prim["attributes"]
            if "JOINTS_0" not in at:
                continue
            pos = g.accessor(at["POSITION"]).astype(float)
            jn = g.accessor(at["JOINTS_0"]).astype(int)
            wt = g.accessor(at["WEIGHTS_0"]).astype(float)
            out = np.zeros_like(pos)
            tot = np.zeros(len(pos))
            bones = np.full(jn.shape, -1, dtype=int)
            for k in range(4):
                bi = np.array([index[joints[x]] for x in jn[:, k]])
                bones[:, k] = np.where(wt[:, k] > 0, bi, -1)
                for v in range(len(pos)):
                    if wt[v, k] > 0:
                        m = world[bi[v]] @ inv[jn[v, k]]
                        out[v] += wt[v, k] * (m @ np.append(pos[v], 1.0))[:3]
                        tot[v] += wt[v, k]
            out = out / np.maximum(tot, 1e-9)[:, None]
            P.append(out)
            B.append(bones)
            W.append(wt)
    verts = ec.Vertices(np.vstack(P), np.vstack(B), np.vstack(W))
    return g, skel, verts


def clip_angles(g, skel, name):
    """Rotation angle of last*first^-1 per node of a clip, radians."""
    j = g.j
    nodes = j["nodes"]
    index = {}
    for k, n in enumerate(skel.names):
        index.setdefault(n, k)
    for a in j.get("animations", []):
        if a.get("name") != name:
            continue
        out = {}
        for ch in a["channels"]:
            if ch["target"]["path"] != "rotation":
                continue
            s = a["samplers"][ch["sampler"]]
            q = g.accessor(s["output"]).astype(float)
            first, last = q[0], q[-1]
            dot = abs(float(np.dot(first / np.linalg.norm(first), last / np.linalg.norm(last))))
            nm = nodes[ch["target"]["node"]].get("name", "")
            out[index.get(nm, -1)] = 2.0 * np.arccos(min(1.0, dot))
        return out
    return None


def verify(path):
    g, skel, verts = load(path)
    skmm = json.load(open(path + ".skmm", encoding="utf-8"))
    entry = skmm["cloth"][0]
    roots = [skel.index[r] for r in entry["roots"]]
    f = entry["forward"]
    fwd_blender = np.array([f["x"], -f["z"], f["y"]])      # engine (x, y, z) = blender (x, z, -y) -> blender = (x, -z, y)
    # the engine works in its own Y-up model space: use it directly
    strips = ec.build_strips(skel, verts, roots, np.array([f["x"], f["y"], f["z"]]), np.array([0.0, 1.0, 0.0]),
                             entry.get("tipLength", 0.0))
    report = {"strips": [], "colliders": [], "clips": {}}
    for s in strips:
        report["strips"].append(([skel.names[x.bone] for x in s.joints], s.side, round(s.tip_length, 3)))
    for c in entry.get("colliders", []):
        r = ec.fit_collider(skel, verts, skel.index[c["bone"]], skel.index[c["to"]], c["radius"], c["endRadius"])
        report["colliders"].append((c["bone"], c["to"], round(r[0], 3), round(r[1], 3)))
    for key in ("openClip", "volumeClip"):
        if entry.get(key):
            ang = clip_angles(g, skel, entry[key])
            if ang is None:
                report["clips"][key] = "MISSING in file"
            else:
                report["clips"][key] = sum(1 for s in strips if ec.layer_uses_strip(s, ang))
    return report


if __name__ == "__main__":
    r = verify(sys.argv[1])
    print("strips:", len(r["strips"]))
    for s in r["strips"]:
        print("  ", s)
    print("colliders:", r["colliders"])
    print("clips (strips using each layer):", r["clips"])
