textures/skies/skybox
{
    surfaceparm noimpact
    surfaceparm nolightmap
    surfaceparm sky
    q3map_lightimage env/space1_up.png
    q3map_sun        1 1 1 100 -58 58
    q3map_surfacelight 400

    skyparms env/space1 - -
    {
        map env/space1_up.png
        blendfunc GL_ONE GL_ONE
        tcMod scroll 0.05 0.06
        tcMod scale 3 2
    }
}