"""Rules mirrored from source/Engine/Cloth/BoneCloth.cpp (see DESIGN.md section 2).

Pure numpy, no bpy: everything here is unit-tested outside Blender. All positions are in one consistent space (the add-on
uses Blender world space, Z up); `up` is passed in. Bone matrices are 4x4, column-vector convention, bone axis = local +Y.
"""
import numpy as np

ENGINE_COMMIT = "bdacf806f"

SKINNED_WEIGHT = 0.05       # a bone with vertices above this weight carries geometry
TIP_WEIGHT = 0.25           # vertices that reach a strip's end
JOIN_TOLERANCE = 0.05       # a strip continues into one whose top is this far off its line, per length
SIDE_TOLERANCE = 0.02       # strip tops nearer the middle than this are neither left nor right (metres)
MAX_BONE_INFLUENCE = 4
MAX_SKINNED_BONES = 256
FALLBACK_COLLIDER_RADIUS = 0.06
LAYER_MIN_ANGLE = np.radians(0.5)


class Skeleton:
    """names, parent (index or -1; parents come before children) and rest_world (n,4,4) in the working space."""

    def __init__(self, names, parent, rest_world):
        self.names = list(names)
        self.parent = list(parent)
        self.rest_world = np.asarray(rest_world, dtype=float)
        self.index = {n: i for i, n in enumerate(self.names)}
        self.children = [[] for _ in self.names]
        for i, p in enumerate(self.parent):
            if p >= 0:
                self.children[p].append(i)

    def head(self, i):
        return self.rest_world[i][:3, 3]


class Vertices:
    """position (n,3) at rest, bones (n,4) int (-1 = unused), weights (n,4)."""

    def __init__(self, position, bones, weights):
        self.position = np.asarray(position, dtype=float).reshape(-1, 3)
        self.bones = np.asarray(bones, dtype=int).reshape(-1, MAX_BONE_INFLUENCE)
        self.weights = np.asarray(weights, dtype=float).reshape(-1, MAX_BONE_INFLUENCE)

    def dominant(self):
        """Per vertex the bone with the largest weight (-1 for none)."""
        w = np.where(self.bones >= 0, self.weights, -1.0)
        k = np.argmax(w, axis=1)
        idx = np.arange(len(k))
        out = self.bones[idx, k]
        return np.where(w[idx, k] > 0.0, out, -1)


class Joint:
    def __init__(self, bone, above):
        self.bone = bone
        self.above = above
        self.tip_local = np.zeros(3)
        self.along = 0.0


class Strip:
    def __init__(self):
        self.joints = []
        self.side = 0
        self.tip_length = 0.0

    @property
    def bones(self):
        return [j.bone for j in self.joints]


def skinned_counts(skeleton, verts, threshold=SKINNED_WEIGHT):
    counts = np.zeros(len(skeleton.names), dtype=int)
    mask = (verts.bones >= 0) & (verts.weights >= threshold)
    np.add.at(counts, verts.bones[mask], 1)
    return counts


def find_chains(skeleton, verts, roots):
    """BuildStrips part 1: a chain runs down from each child of a root that carries geometry, through the child carrying
    the most; chains whose last bone points straight at another's top are joined."""
    skinned = skinned_counts(skeleton, verts)
    chains = []
    for root in roots:
        for top in skeleton.children[root]:
            if skinned[top] == 0:
                continue
            chain = []
            bone = top
            while bone is not None:
                chain.append(bone)
                nxt, most = None, 0
                for child in skeleton.children[bone]:
                    if skinned[child] > most:
                        most, nxt = skinned[child], child
                bone = nxt
            chains.append(chain)

    joined = True
    while joined:
        joined = False
        for a in range(len(chains)):
            to_last = np.linalg.inv(skeleton.rest_world[chains[a][-1]])
            for b in range(len(chains)):
                if a == b:
                    continue
                top = (to_last @ np.append(skeleton.head(chains[b][0]), 1.0))[:3]
                if top[1] <= 0.0 or np.hypot(top[0], top[2]) > JOIN_TOLERANCE * top[1]:
                    continue
                chains[a].extend(chains[b])
                del chains[b]
                joined = True
                break
            if joined:
                break
    return chains


def join_candidates(skeleton, chains):
    """Pairs (a, b, offset_ratio) where chain a's last bone points near chain b's top but misses the engine tolerance."""
    out = []
    for a in range(len(chains)):
        to_last = np.linalg.inv(skeleton.rest_world[chains[a][-1]])
        for b in range(len(chains)):
            if a == b:
                continue
            top = (to_last @ np.append(skeleton.head(chains[b][0]), 1.0))[:3]
            if top[1] <= 0.0:
                continue
            ratio = np.hypot(top[0], top[2]) / top[1]
            if JOIN_TOLERANCE < ratio <= 0.25:
                out.append((a, b, float(ratio)))
    return out


def side_axes(forward, up):
    forward = np.asarray(forward, float)
    forward = forward / np.linalg.norm(forward)
    left = np.cross(up, forward)
    n = np.linalg.norm(left)
    return forward, (left / n if n > 1e-6 else np.array([1.0, 0.0, 0.0]))


def build_strips(skeleton, verts, roots, forward, up, tip_length=0.0, follow_root=60.0, follow_tip=8.0):
    """Full port of BuildStrips. Returns a list of Strip, or raises ValueError (the engine returns false)."""
    chains = find_chains(skeleton, verts, roots)
    if not chains:
        raise ValueError("no bone under the roots has vertices skinned to it")
    _, left = side_axes(forward, up)
    centre = np.zeros(3)
    for chain in chains:
        centre += skeleton.head(chain[0]) / len(chains)

    strips = []
    for chain in chains:
        strip = Strip()
        count = len(chain)
        for k, bone in enumerate(chain):
            above = chain[k - 1] if k > 0 else skeleton.parent[bone]
            joint = Joint(bone, above)
            joint.along = k / (count - 1) if count > 1 else 0.0
            if k + 1 < count:
                joint.tip_local = (np.linalg.inv(skeleton.rest_world[bone]) @ np.append(skeleton.head(chain[k + 1]), 1.0))[:3]
            strip.joints.append(joint)

        last = strip.joints[-1]
        axis_local = np.array([0.0, 1.0, 0.0])
        if count > 1 and np.linalg.norm(strip.joints[count - 2].tip_local) > 1e-6:
            prev = strip.joints[count - 2].tip_local
            axis_local = prev / np.linalg.norm(prev)
        axis = skeleton.rest_world[last.bone][:3, :3] @ axis_local
        unit = np.linalg.norm(axis)
        if unit < 1e-8:
            raise ValueError("bone '%s' has no size" % skeleton.names[last.bone])
        axis = axis / unit
        head = skeleton.head(last.bone)
        length = tip_length
        if length <= 0.0:
            sel = (verts.bones == last.bone) & (verts.weights >= TIP_WEIGHT)
            rows = np.where(sel.any(axis=1))[0]
            if len(rows):
                length = max(0.0, float(np.max((verts.position[rows] - head) @ axis)))
            if length < 0.01:
                length = float(np.linalg.norm(head - skeleton.head(chain[-2]))) if count > 1 else 0.1
        last.tip_local = axis_local * (length / unit)
        strip.tip_length = length

        sideways = float(np.dot(skeleton.head(chain[0]) - centre, left))
        strip.side = 1 if sideways > SIDE_TOLERANCE else (-1 if sideways < -SIDE_TOLERANCE else 0)
        strips.append(strip)
    return strips


def _on_the_way(skeleton, candidate, bone, to):
    current = skeleton.parent[to]
    while current >= 0:
        if current == candidate:
            return True
        if current == bone:
            break
        current = skeleton.parent[current]
    return False


def fit_collider(skeleton, verts, bone, to, radius=0.0, end_radius=0.0):
    """BuildColliders: returns (radius, end_radius). A radius of 0 is fitted from the vertices of the limb."""
    if radius > 0.0 and end_radius > 0.0:
        return radius, end_radius
    has_children = [len(c) > 0 for c in skeleton.children]
    start, end = skeleton.head(bone), skeleton.head(to)
    axis = end - start
    length2 = float(np.dot(axis, axis))
    dominant = verts.dominant()
    near_start, near_end, everything = [], [], []
    for i in range(len(verts.position)):
        d = dominant[i]
        if d < 0:
            continue
        parent = skeleton.parent[d]
        if not _on_the_way(skeleton, d, bone, to) and (
                has_children[d] or parent < 0 or not _on_the_way(skeleton, parent, bone, to)):
            continue
        t = float(np.clip(np.dot(verts.position[i] - start, axis) / length2, 0.0, 1.0)) if length2 > 1e-12 else 0.0
        distance = float(np.linalg.norm(verts.position[i] - (start + axis * t)))
        everything.append(distance)
        if t <= 0.3:
            near_start.append(distance)
        if t >= 0.7:
            near_end.append(distance)

    def median(values):
        values = sorted(values)
        return values[len(values) // 2]   # nth_element at size/2, like the engine

    fallback = median(everything) if everything else FALLBACK_COLLIDER_RADIUS
    if radius <= 0.0:
        radius = median(near_start) if near_start else fallback
    if end_radius <= 0.0:
        end_radius = median(near_end) if near_end else fallback
    return radius, end_radius


def layer_uses_strip(strip, rotation_angles):
    """C10: a strip uses a wind layer when the clip turns any of its bones by 0.5 degrees or more.
    rotation_angles: dict bone index -> radians of the first-to-last difference."""
    return any(rotation_angles.get(j.bone, 0.0) >= LAYER_MIN_ANGLE for j in strip.joints)


def limit_influences(bones, weights, keep=MAX_BONE_INFLUENCE):
    """Keep the `keep` largest weights of one vertex and renormalise (what the exporter does)."""
    order = np.argsort(-np.asarray(weights))[:keep]
    b = [bones[i] for i in order]
    w = np.asarray([weights[i] for i in order], dtype=float)
    total = w.sum()
    return b, (w / total if total > 0 else w)
