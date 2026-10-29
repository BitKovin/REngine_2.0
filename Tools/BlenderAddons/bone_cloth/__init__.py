"""Bone Cloth: sets up coats, jackets and skirts for the engine's bone cloth. See DESIGN.md."""
import bpy

from . import ops, overlay, props, ui


def register():
    props.register()
    for c in ops.CLASSES + ui.CLASSES:
        bpy.utils.register_class(c)
    if not bpy.app.background:
        overlay.register()


def unregister():
    if not bpy.app.background:
        overlay.unregister()
    for c in reversed(ops.CLASSES + ui.CLASSES):
        bpy.utils.unregister_class(c)
    props.unregister()
