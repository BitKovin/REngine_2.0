// Editor only shaders (making textures to appear transparent)

textures/AAATRIGGER
{
	// Standard trigger texture
	qer_trans 0.5
}

textures/BEVEL
{
	// Like NULL, but changes how clipnodes are generated for a brush
	// same as HLCSG parm `-cliptype precise`
}

textures/black_HIDDEN
{
	// Invisible, but lightmapped. Used for fixing MDL lighting bugs
	qer_trans 0.75
}

textures/BOUNDINGBOX
{
	// Overrides entity's bounding box
	qer_trans 0.5
}

textures/CLIP
{
	// Clip all 3 hulls
	qer_trans 0.5
}

textures/CLIPBEVEL
{
	// Like BEVEL, but for clip brushes
	qer_trans 0.5
}

textures/CLIPBEVELBRUSH
{
	// Like CLIPBEVEL, but affects every single face
	qer_trans 0.5
}

textures/CLIPHULL1
{
	// Clip hull 1 (standing) only
	qer_trans 0.5
}

textures/CLIPHULL2
{
	// Clip hull 2 (large monsters) only
	qer_trans 0.5
}

textures/CLIPHULL3
{
	// Clip hull 3 (crouching) only
	qer_trans 0.5
}

textures/CONTENTEMPTY
{
	// Set brush contents to empty, disabling collision and letting light pass through
}

textures/CONTENTWATER
{
	// Set brush contents to empty, disabling collision and backface culling
	// also works as nodraw texture for water
}

textures/HINT
{
	// Hint cut visleaves
	qer_trans 0.5
}

textures/NOCLIP
{
	// Disable clipnode generation, leaving only hitscan collision
	// same as zhlt_noclip KV in entities
}

textures/NULL
{
	// Standard solid nodraw texture; blocks light if part of world
}

textures/ORIGIN
{
	// Set brush origin for rotating geo and light_origin overrides
	qer_trans 0.5
}

textures/SKIP
{
	// Nonsolid and doesn't seal brushes; use with HINT and black_HIDDEN only
	qer_trans 0.5
}

textures/sky
{
	// Render skybox
	// 
}

textures/SOLIDHINT
{
	// Like NULL, but affects face subdivision (use on complex terrain)
}

//textures/SPLITFACE
//{
//	// Like SKIP, but affects face subdivision of faces it touches
//	// Commented out, because it doesn't exist in zhlt.wad
//}