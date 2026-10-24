"""Cut-away pictures of the enterable buildings: python pics_interiors.py <out dir>"""
import sys
import render
from PIL import Image

CUTS = {"house_open": (2.82, 13, 12, 72), "bakery": (3.12, 16, 12, 72), "mill": (3.52, 16, 12, 72), "tavern": (5.62, 20, 12, 74)}
GROUND = {"classname": "func_group", "brushes": [{"op": "add", "vertices": [[-60, -1, -60], [60, -1, -60], [60, 0, -60], [-60, 0, -60], [-60, -1, 60], [60, -1, 60], [60, 0, 60], [-60, 0, 60]],
                                                 "faces": [{"loop": l, "texture": "Ground/grass"} for l in [[0, 3, 2, 1], [4, 5, 6, 7], [0, 4, 7, 3], [1, 2, 6, 5], [0, 1, 5, 4], [3, 7, 6, 2]]]}]}

if __name__ == "__main__":
    out = sys.argv[1]
    tiles = []
    for n, (cy, dist, yaw, pitch) in CUTS.items():
        d = render.load(f"prefabs/buildings/{n}.map.json")
        d = {"entities": d["entities"] + [GROUND], "prefabInstances": d["prefabInstances"]}
        f = f"{out}/int_{n}.png"
        render.orbit(d, (0, 0.8, 0), dist, yaw, pitch, f, size=(620, 470), fov=48, cut_y=cy)
        tiles.append(Image.open(f))
    sheet = Image.new("RGB", (1240, 940))
    for i, t in enumerate(tiles):
        sheet.paste(t, ((i % 2) * 620, (i // 2) * 470))
    sheet.save(f"{out}/int_sheet.png")
