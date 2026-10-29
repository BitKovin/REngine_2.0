"""Which steps are done, stale or untouched (DESIGN.md section 6.1)."""
import json
import uuid

from . import mesh_prep, props

ORDER = ("character", "mesh", "bones", "weights", "rest", "poses", "colliders", "motion")
TITLES = {
    "character": "Character", "mesh": "Mesh", "bones": "Shape & bones", "weights": "Weights", "rest": "Rest shape",
    "poses": "Wind poses", "colliders": "Collisions", "motion": "Motion",
}
UPSTREAM = {"bones": "character", "weights": "bones", "rest": "weights", "poses": "rest", "colliders": "rest"}


def _objects(setup):
    return [r.obj.name for r in setup.cloth_objects if r.obj]


def _own(setup, key):
    if key == "character":
        return props.settings_hash(setup.character) + json.dumps(_objects(setup))
    if key == "bones":
        return props.settings_hash(setup.guides) + props.settings_hash(setup.character)
    if key == "weights":
        return props.settings_hash(setup.weights) + json.dumps(_objects(setup))
    if key == "rest":
        r = setup.rest
        return json.dumps([round(r.flare, 5), round(r.open, 5), round(r.bend, 5), r.clear_legs, round(r.clearance, 5),
                           round(setup.motion.radius, 5)])
    if key == "poses":
        out = []
        for layer in (setup.poses.open, setup.poses.volume):
            out.append([layer.enabled, layer.clip_name, round(layer.amount, 5), round(layer.direction, 5),
                        round(layer.bend, 5), round(layer.upper_share, 5), round(layer.backward, 5), round(layer.max_turn, 4), round(layer.slide, 4)])
        cols = [[c.name, round(c.angle, 4), c.enabled, c.open_mult, c.volume_mult] for c in setup.guides.columns]
        return json.dumps([out, cols, [layer.mode for layer in (setup.poses.open, setup.poses.volume)]])
    if key == "colliders":
        return json.dumps([[c.bone, c.to, c.enabled, c.auto_radius, round(c.radius, 5), c.auto_end,
                            round(c.end_radius, 5)] for c in setup.colliders])
    return ""


def current_input(setup, key):
    up = UPSTREAM.get(key)
    base = _own(setup, key)
    if up:
        base += "|" + props.stage(setup, up).output_hash
    return base


def mark_done(setup, key, note=""):
    st = props.stage(setup, key)
    st.input_hash = current_input(setup, key)
    st.output_hash = uuid.uuid4().hex[:10]
    st.note = note


def status(setup, arm, key):
    """'none', 'ok', 'stale' or 'warn'."""
    st = props.stage(setup, key)
    if key == "mesh":
        items = mesh_prep.check(setup, arm)
        if not setup.cloth_objects:
            return "none"
        return "warn" if any(i[1] in ("warn", "error") for i in items) else "ok"
    if key == "motion":
        return "ok"
    if not st.input_hash:
        return "none"
    up = UPSTREAM.get(key)
    if up and not props.stage(setup, up).input_hash:
        return "stale"
    return "ok" if st.input_hash == current_input(setup, key) else "stale"


def stale_list(setup, arm):
    return [k for k in ORDER if status(setup, arm, k) == "stale"]
