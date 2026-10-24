"""Writes every building prefab to GameData/prefabs/buildings and a sheet map for looking at them."""
import buildings

if __name__ == "__main__":
    for name, fn in buildings.BUILDINGS.items():
        fn().save(f"prefabs/buildings/{name}.map.json")
    print(len(buildings.BUILDINGS), "buildings")
