textures/skies/skybox1_cube
{
    surfaceparm noimpact
    surfaceparm nolightmap
    surfaceparm sky
    q3map_lightimage env/skybox1_cube_up.png
    q3map_sun        0.82 0.92 1 100 -45 80
    q3map_surfacelight 100

    skyparms env/skybox1_cube - -
    {
        map env/skybox1_cube_up.png
        blendfunc GL_ONE GL_ONE
        tcMod scroll 0.05 0.06
        tcMod scale 3 2
    }
}