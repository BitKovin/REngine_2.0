#pragma once

#include "UiElement.h"
#include "UiRenderer.h"
#include "../Texture.hpp"
#include "../AssetRegistry.h"
#include "../Level.hpp"
#include "../Video/VideoInstance.h"
#include "../Time.hpp"

class UiVideo : public UiElement
{
public:
    string VideoPath = "GameData/cat.png";

    std::unique_ptr<VideoInstance> videoInstance = nullptr;

    UiVideo() {}
    ~UiVideo() { DestroyTexture(); }

    void Update() override
    {
        UiElement::Update();
        if (videoInstance)
            videoInstance->Update(Time::DeltaTimeNoTimeScale);
    }

    void Draw() override
    {
        if (videoInstance == nullptr)
        {
            Video* video = AssetRegistry::GetVideoFromFile(VideoPath);
            if (!video) { UiElement::Draw(); return; }
            videoInstance = make_unique<VideoInstance>(video);
            videoInstance->Start();
            videoInstance->Update(0);
        }

        const int w = videoInstance->GetWidth();
        const int h = videoInstance->GetHeight();

        if (w > 0 && h > 0)
        {
            // One persistent texture, (re)created only when the video size changes
            if (!bgfx::isValid(handle) || w != texW || h != texH)
            {
                DestroyTexture();
                handle = bgfx::createTexture2D((uint16_t)w, (uint16_t)h, false, 1,
                    bgfx::TextureFormat::RGBA8,
                    BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP,
                    nullptr);                       // no initial data => updatable texture
                texW = w; texH = h;
                texVersion = ~0ull;
            }

            // Upload only when a new picture was decoded. bgfx::copy() makes bgfx own a
            // private copy, so the video's buffer can be rewritten immediately afterwards.
            const uint64_t ver = videoInstance->GetFrameVersion();
            if (bgfx::isValid(handle) && ver != texVersion)
            {
                const std::vector<uint8_t>& rgba = videoInstance->GetCurrentFrameRGBA();
                if (rgba.size() == (size_t)w * h * 4)
                {
                    bgfx::updateTexture2D(handle, 0, 0, 0, 0, (uint16_t)w, (uint16_t)h,
                        bgfx::copy(rgba.data(), (uint32_t)rgba.size()));
                    texVersion = ver;
                }
            }
        }

        if (bgfx::isValid(handle))
            DrawSelfTextured(handle, GetFinalColor(), (float)texW, (float)texH);

        UiElement::Draw();
    }

private:
    void DestroyTexture()
    {
        if (bgfx::isValid(handle))
        {
            bgfx::destroy(handle);
            handle = BGFX_INVALID_HANDLE;
        }
    }

    bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
    int texW = 0, texH = 0;
    uint64_t texVersion = ~0ull;
};
