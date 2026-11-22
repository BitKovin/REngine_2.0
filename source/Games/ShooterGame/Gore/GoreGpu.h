#pragma once

#include <AssetRegistry.h>
#include <bgfx/bgfx.h>

namespace Gore
{
    // bgfx may only be called from the main thread. The last reference to a piece of generated geometry (a cap, a cavity) can be
    // dropped anywhere: by the game thread (a cut ends, a mesh is cleared), by the build worker, or by a pool thread in PreFinalize.
    // Its buffers are therefore not destroyed by the destructor itself but handed to the main thread, which destroys them at the
    // start of the next frame (AssetRegistry::ProcessPendingUploads). Nothing can still be drawing them by then: the owner held the
    // geometry for as long as a draw command pointed at it.
    inline void DestroyBuffersOnMainThread(bgfx::VertexBufferHandle vbh, bgfx::IndexBufferHandle ibh)
    {
        if (!bgfx::isValid(vbh) && !bgfx::isValid(ibh)) return;
        AssetRegistry::EnqueuePendingUpload([vbh, ibh]()
        {
            if (bgfx::isValid(vbh)) bgfx::destroy(vbh);
            if (bgfx::isValid(ibh)) bgfx::destroy(ibh);
        });
    }
}
