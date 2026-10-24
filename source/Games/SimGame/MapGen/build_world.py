"""Writes every prefab (props, buildings, villages) and the world map, checks the villages."""
import build_props, build_buildings, build_villages, world

if __name__ == "__main__":
    import props, buildings, villages
    for name, fn in props.PROPS.items():
        fn().save(f"prefabs/props/{name}.map.json")
    for name, fn in buildings.BUILDINGS.items():
        fn().save(f"prefabs/buildings/{name}.map.json")
    for name, fn in villages.VILLAGES.items():
        v, gates = fn()
        v.p.save(f"prefabs/{name}.map.json")
        print(name, "problems:", v.check(gates) or "none")
    print("forest trees:", world.save())
