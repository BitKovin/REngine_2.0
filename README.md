This repository contains code of REngine 2. It's evolution of my previous RetroEngine that was written in C#.

Core improvements of this version:

1. Jolt Physics as main physics engine (Instead of outdated Bullet Physics)
2. Full Quake 3 BSP map format support (including lightmaps, light volumes, precomputed visibility and additional rendering optimizations)
3. Behavior Tree editor
4. Emscripten compilation support with experimental multi-threading
5. Improved animation blending and playback
6. Native OpenGL ES 3.0 and OpenGL 4+ renderer (to support both desktop and browser with possibility of mobile using Angle)
7. Mobile control
