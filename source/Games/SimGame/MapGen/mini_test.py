"""A tiny lighting test level: structural ground, a structural box, a detail box, a detail road strip on the ground."""
from kit import P
p = P()
p.box((-40, -1, -40), (40, 0, 40), "Ground/grass")
p.box((-3, 0, -3), (-1, 2, -1), "wall/brickWall1")                 # structural
p.box((1, 0, -3), (3, 2, -1), "wall/brickWall1", True)             # detail
p.box((-10, -0.3, 2), (10, 0.04, 6), "lq_terra/med_cobstn1_1", True)   # detail road on the ground
p.box((-10, -0.3, 8), (10, 0.04, 12), "lq_terra/med_cobstn1_1")        # structural road on the ground
p.ent("sim_spawn", "e_start", (0, 0.1, 15), 180, targetname="start", spawnflags=1)
p.ent("light_directional", "e_sun", (0, 60, 0), 0, brightness=1.4, color="1 0.96 0.88", style=0)
p.ents[-1]["angles"] = [55, 30, 0]
p.save("maps/village_study/mini_test.map.json", root_kv={"_sky_auto": "box", "_sky_texture": "env/skybox1_cube"})
