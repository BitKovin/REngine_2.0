"""Design-review pictures of a map or prefab file (a painter's-algorithm flat renderer; not the engine, not lit): prefabs expanded (yaw + translation), faces coloured by the mean colour of their
texture, shaded by a fixed sun, optional cut-away (drop every brush that starts above a height: roofs, ceilings) and markers for the sim entities (door orange, slot purple, container brown,
item prop yellow, spawn green, site footprint outlined). Used to look at layouts and interiors without the editor's pane; the editor and the game remain the real look.
"""
import json
import math
import os
import sys

from PIL import Image, ImageDraw

from kit import GAME_DATA, yaw_xz

_TEX_CACHE = {}
MARK = {"sim_door": (255, 150, 40), "sim_slot": (190, 110, 255), "sim_container": (150, 100, 50), "sim_item_prop": (255, 230, 60), "sim_spawn": (60, 255, 90),
        "light_point": (255, 255, 160)}


def tex_color(name):
    if name in _TEX_CACHE:
        return _TEX_CACHE[name]
    c = (150, 150, 150)
    for ext in (".jpg", ".png", ".tga"):
        p = os.path.join(GAME_DATA, "textures", name + ext)
        if os.path.exists(p):
            try:
                c = Image.open(p).convert("RGB").resize((1, 1)).getpixel((0, 0))
            except Exception:
                pass
            break
    _TEX_CACHE[name] = c
    return c


def clip_near(poly, near=0.1):
    """Sutherland-Hodgman against the camera's near plane (camera space, z forward)."""
    out = []
    for i, a in enumerate(poly):
        b = poly[(i + 1) % len(poly)]
        ina, inb = a[2] >= near, b[2] >= near
        if ina:
            out.append(a)
        if ina != inb:
            t = (near - a[2]) / (b[2] - a[2])
            out.append((a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t, near))
    return out


def load(rel):
    with open(os.path.join(GAME_DATA, rel.replace("/", os.sep)), encoding="utf-8") as f:
        return json.load(f)


def expand(doc, yaw=0.0, org=(0.0, 0.0, 0.0), depth=0):
    """Entities (dicts with world vertices and origin) of a document and everything it includes."""
    out = []
    def T(p):
        x, z = yaw_xz(yaw, p[0], p[2])
        return (org[0] + x, org[1] + p[1], org[2] + z)
    for e in doc["entities"]:
        brushes = []
        for b in e.get("brushes", []):
            brushes.append({"op": b["op"], "v": [T(v) for v in b["vertices"]], "faces": b["faces"]})
        o = e.get("origin", [0, 0, 0])
        out.append({"cls": e["classname"], "brushes": brushes, "origin": T(o), "kv": e.get("keyvalues", {}), "yaw": (e.get("angles") or [0, 0, 0])[1] + yaw})
    for i in doc.get("prefabInstances", []):
        child = load(i["file"])
        a = i.get("angles", [0, 0, 0])[1]
        x, z = yaw_xz(yaw, i["origin"][0], i["origin"][2])
        out += expand(child, yaw + a, (org[0] + x, org[1] + i["origin"][1], org[2] + z), depth + 1)
    return out


def render(doc, eye, target, out, size=(900, 600), fov=50.0, cut_y=None, markers=True, sun=(-0.45, -0.8, -0.35), sky=(150, 175, 205), ground_y=0.06, cut_markers_above=None):
    ents = expand(doc)
    W, H = size
    f = (H / 2) / math.tan(math.radians(fov / 2))
    fw = [target[i] - eye[i] for i in range(3)]
    n = math.sqrt(sum(c * c for c in fw)); fw = [c / n for c in fw]
    up0 = (0, 1, 0)
    rt = [fw[1] * up0[2] - fw[2] * up0[1], fw[2] * up0[0] - fw[0] * up0[2], fw[0] * up0[1] - fw[1] * up0[0]]
    n = math.sqrt(sum(c * c for c in rt)); rt = [c / n for c in rt]
    up = [rt[1] * fw[2] - rt[2] * fw[1], rt[2] * fw[0] - rt[0] * fw[2], rt[0] * fw[1] - rt[1] * fw[0]]
    sl = math.sqrt(sum(c * c for c in sun)); sun = [c / sl for c in sun]

    def cam(p):
        d = [p[i] - eye[i] for i in range(3)]
        return (sum(d[i] * rt[i] for i in range(3)), sum(d[i] * up[i] for i in range(3)), sum(d[i] * fw[i] for i in range(3)))

    def proj(c):
        return (W / 2 + f * c[0] / c[2], H / 2 - f * c[1] / c[2])

    polys = []
    sites = []
    for e in ents:
        if e["cls"] == "sim_site":
            for b in e["brushes"]:
                sites.append(b["v"])
        if e["cls"] in ("light_point",) or e["cls"].startswith("sim_") and not e["brushes"]:
            continue
        item = e["cls"] == "sim_item_prop"
        for b in e["brushes"]:
            if b["op"] != "add" or (e["cls"].startswith("sim_") and e["cls"] != "sim_item_prop") or e["cls"] == "trigger":
                continue
            if cut_y is not None and min(v[1] for v in b["v"]) >= cut_y - 1e-3:
                continue
            for fc in b["faces"]:
                pts = [b["v"][i] for i in fc["loop"]]
                a = [pts[1][i] - pts[0][i] for i in range(3)]
                c = [pts[2][i] - pts[0][i] for i in range(3)]
                nm = [a[1] * c[2] - a[2] * c[1], a[2] * c[0] - a[0] * c[2], a[0] * c[1] - a[1] * c[0]]
                ln = math.sqrt(sum(x * x for x in nm))
                if ln < 1e-9:
                    continue
                nm = [x / ln for x in nm]
                cen = [sum(p[i] for p in pts) / len(pts) for i in range(3)]
                if sum(nm[i] * (eye[i] - cen[i]) for i in range(3)) <= 0:
                    continue
                col = tex_color(fc.get("texture") or "dev/grey")
                if item:
                    col = tuple(min(255, int(x * 1.25 + 20)) for x in col)
                sh = 0.55 + 0.75 * max(0.0, -sum(nm[i] * sun[i] for i in range(3)))
                col = tuple(min(255, int(x * sh)) for x in col)
                cp = clip_near([cam(p) for p in pts])
                if len(cp) < 3:
                    continue
                d = math.sqrt(sum((cen[i] - eye[i]) ** 2 for i in range(3)))
                flat = max(p[1] for p in pts) <= ground_y and nm[1] > 0.9
                polys.append((flat, d, [proj(q) for q in cp], col))
    polys.sort(key=lambda t: (not t[0], -t[1]))
    img = Image.new("RGB", size, sky)
    dr = ImageDraw.Draw(img)
    for _, _, pp, col in polys:
        dr.polygon(pp, fill=col, outline=tuple(int(x * 0.7) for x in col))
    if markers:
        for sv in sites:
            ys = min(v[1] for v in sv) + 0.05
            xs = [v[0] for v in sv]; zs = [v[2] for v in sv]
            ring = [(min(xs), ys, min(zs)), (max(xs), ys, min(zs)), (max(xs), ys, max(zs)), (min(xs), ys, max(zs))]
            cp = [cam(p) for p in ring]
            if all(q[2] > 0.1 for q in cp):
                pp = [proj(q) for q in cp]
                dr.line(pp + [pp[0]], fill=(255, 140, 20), width=2)
        for e in ents:
            col = MARK.get(e["cls"])
            if not col or (e["brushes"] and e["cls"] != "sim_item_prop"):
                continue
            if e["cls"] == "sim_item_prop":
                continue
            if cut_markers_above is not None and e["origin"][1] > cut_markers_above:
                continue
            c = cam(e["origin"])
            if c[2] < 0.1:
                continue
            x, y = proj(c)
            r = 5 if e["cls"] != "sim_door" else 7
            dr.ellipse([x - r, y - r, x + r, y + r], fill=col, outline=(20, 20, 20))
            if e["cls"] in ("sim_door", "sim_slot", "sim_spawn"):
                a = math.radians(e["yaw"])
                tip = cam((e["origin"][0] + math.sin(a) * 0.8, e["origin"][1], e["origin"][2] + math.cos(a) * 0.8))
                if tip[2] > 0.1:
                    tx, ty = proj(tip)
                    dr.line([x, y, tx, ty], fill=(20, 20, 20), width=2)
    img.save(out)
    return out


def view(rel_or_doc, out, eye, target, **kw):
    doc = rel_or_doc if isinstance(rel_or_doc, dict) else load(rel_or_doc)
    return render(doc, eye, target, out, **kw)


if __name__ == "__main__":
    # python render.py <map rel path> <out png> ex ey ez tx ty tz [cut_y]
    a = sys.argv
    kw = {"cut_y": float(a[9])} if len(a) > 9 else {}
    view(a[1], a[2], tuple(map(float, a[3:6])), tuple(map(float, a[6:9])), **kw)


def orbit(doc, center, dist, yaw_deg, pitch_deg, out, **kw):
    """A picture from `dist` metres away around `center`: yaw 0 looks from +Z towards -Z, pitch is the angle above the horizon."""
    y, p = math.radians(yaw_deg), math.radians(pitch_deg)
    eye = (center[0] + dist * math.sin(y) * math.cos(p), center[1] + dist * math.sin(p), center[2] + dist * math.cos(y) * math.cos(p))
    return render(doc, eye, center, out, **kw)


def bounds(doc):
    vs = [v for e in expand(doc) for b in e["brushes"] for v in b["v"]]
    return [min(v[i] for v in vs) for i in range(3)], [max(v[i] for v in vs) for i in range(3)]


def contact_sheet(files, out, cols, tile=(300, 260), yaw=30, pitch=28, names=True, sky=(150, 175, 205)):
    """One tile per prefab file, each framed by its own bounds, on a grass pad."""
    from PIL import ImageFont
    rows = (len(files) + cols - 1) // cols
    sheet = Image.new("RGB", (cols * tile[0], rows * tile[1]), (40, 40, 44))
    try:
        font = ImageFont.truetype("arial.ttf", 14)
    except Exception:
        font = None
    for i, rel in enumerate(files):
        doc = load(rel)
        lo, hi = bounds(doc)
        c = [(lo[k] + hi[k]) / 2 for k in range(3)]
        r = max(hi[0] - lo[0], hi[1] - lo[1], hi[2] - lo[2])
        doc = {"entities": doc["entities"] + [{"classname": "func_group", "brushes": [{"op": "add", "vertices": [[-60, -1, -60], [60, -1, -60], [60, 0, -60], [-60, 0, -60], [-60, -1, 60], [60, -1, 60], [60, 0, 60], [-60, 0, 60]],
                                                                                    "faces": [{"loop": l, "texture": "Ground/grass"} for l in [[0, 3, 2, 1], [4, 5, 6, 7], [0, 4, 7, 3], [1, 2, 6, 5], [0, 1, 5, 4], [3, 7, 6, 2]]]}]}],
               "prefabInstances": doc.get("prefabInstances", [])}
        t = f"{os.path.dirname(out)}/_tile.png"
        orbit(doc, c, r * 1.7 + 0.8, yaw, pitch, t, size=tile, fov=45, sky=sky)
        im = Image.open(t)
        if names:
            ImageDraw.Draw(im).text((6, 4), os.path.basename(rel).replace(".map.json", ""), fill=(255, 255, 255), font=font)
        sheet.paste(im, ((i % cols) * tile[0], (i // cols) * tile[1]))
    sheet.save(out)
    return out
