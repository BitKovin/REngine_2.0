"""Writes the three village prefabs and reports the layout checks."""
import villages

if __name__ == "__main__":
    for name, fn in villages.VILLAGES.items():
        v, gates = fn()
        v.p.save(f"prefabs/{name}.map.json")
        probs = v.check(gates)
        print(name, len(v.p.insts), "instances,", len(v.p.ents), "entities,", "problems:", probs or "none")
