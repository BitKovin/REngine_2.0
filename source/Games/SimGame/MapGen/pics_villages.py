"""Pictures of the three villages: python pics_villages.py <out dir>"""
import sys
import render
from PIL import Image
from pics_interiors import GROUND

if __name__ == "__main__":
    out = sys.argv[1]
    for k in "abc":
        d = render.load(f"prefabs/village_{k}.map.json")
        d = {"entities": d["entities"] + [GROUND], "prefabInstances": d["prefabInstances"]}
        render.orbit(d, (0, 0, 0), 150, 0 if k != "b" else 180, 45, f"{out}/vil_{k}_obl.png", size=(900, 600), fov=40, markers=False)
        render.orbit(d, (0, 0, 0), 125, 0, 89.5, f"{out}/vil_{k}_top.png", size=(700, 700), fov=40, markers=True)
