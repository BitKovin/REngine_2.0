"""A test map with every building once, to export and inspect (maps/village_study/bld_test.map.json)."""
from kit import P
import buildings

p = P()
p.box((-80, -1, -60), (80, 0, 60), "Ground/grass")
for i, name in enumerate(buildings.BUILDINGS):
    p.inst(f"prefabs/buildings/{name}.map.json", name, (-60 + i * 22, 0, 0), 0)
p.ent("sim_spawn", "e_spawn", (0, 0.1, 30), 0, targetname="start", spawnflags=1)
p.save("maps/village_study/bld_test.map.json")
print("ok")
