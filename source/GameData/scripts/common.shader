textures/common/nolightmap
{
	surfaceparm nolightmap
}

textures/common/trigger
{
    qer_editorimage textures/common/trigger.png

    // make this surface invisible to the standard light build
    surfaceparm nodraw
    surfaceparm nolightmap
    q3map_nolightmap

    // disable the ambient-occlusion (-dirty) pass
    q3map_noDirty

    // still allow the trigger to be solid/functional if needed
    surfaceparm nonsolid
}