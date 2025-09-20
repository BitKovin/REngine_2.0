#include "Renderer.h"

#include "../EngineMain.h"

#include "../LightSystem/LightManager.h"
#include "../DebugDraw.hpp"

#include "../FogManager.h"
#include "RHI/Bgfx/bgfx_wrapper.h"

Renderer::Renderer()
{
	ivec2 screenResolution = GetScreenResolution();

    InitFrameBuffers();
    InitResolveFrameBuffers();

	fullscreenShader = ShaderManager::GetShaderProgram("fullscreen_vertex","postprocessing");



    if (LightManager::DirectionalShadowsEnabled)
    {
        DirectionalShadowMap = new RenderTexture(LightManager::ShadowMapResolution, LightManager::ShadowMapResolution, TextureFormat::Depth32F, TextureType::Texture2D);
        DirectionalShadowMapFBO = new Framebuffer();
        DirectionalShadowMapFBO->attachDepth(DirectionalShadowMap);

        DetailDirectionalShadowMap = new RenderTexture(LightManager::ShadowMapResolution, LightManager::ShadowMapResolution, TextureFormat::Depth32F, TextureType::Texture2D);
        DetailDirectionalShadowMapFBO = new Framebuffer();
        DetailDirectionalShadowMapFBO->attachDepth(DetailDirectionalShadowMap);
    }


	InitFullscreenVAO();

}

Renderer::~Renderer()
{
	delete(colorBuffer);
	delete(depthBuffer);

	// For bgfx, cleanup is handled automatically
	// No need to manually delete VAOs and VBOs
}

void Renderer::RenderLevel(Level* level)
{
    if (LightManager::DirectionalShadowsEnabled)
    {
        RenderDirectionalLightShadows(level->ShadowRenderList, *DirectionalShadowMapFBO, 4);
        RenderDirectionalLightShadows(level->DetailShadowRenderList, *DirectionalShadowMapFBO, 3);
    }
	RenderCameraForward(level->VissibleRenderList);

	//rendering to screen
	ivec2 screenResolution = GetScreenResolution();
	RenderInterface::SetViewport(0, 0, screenResolution.x, screenResolution.y);
	RenderFullscreenQuad(colorResolveBuffer->id());

}

void Renderer::RenderCameraForward(vector<IDrawMesh*>& VissibleRenderList)
{
    

    ivec2 res = GetScreenResolution();

    // For bgfx, multisample is handled automatically
    // No need to manually enable/disable it

#ifndef GL_ES_PROFILE

    if (MultiSampleCount) 
    {

        if (colorBuffer->type() == TextureType::Texture2D)
        {
            InitFrameBuffers();
        }

        colorBuffer->setSamples(MultiSampleCount);
        depthBuffer->setSamples(MultiSampleCount);
    }
    else
    {

        if (colorBuffer->type() == TextureType::Texture2DMultisample)
        {
            InitFrameBuffers();
        }

        colorBuffer->setSamples(1);
        depthBuffer->setSamples(1);
    }

#endif

    // resize all our buffers
    colorBuffer->resize(res.x, res.y);
    depthBuffer->resize(res.x, res.y);

    colorResolveBuffer->resize(res.x, res.y);
    depthResolveBuffer->resize(res.x, res.y);



    // 1) bind the one multisample FBO with both attachments
    forwardFBO->bind();

    RenderInterface::SetViewport(0, 0, res.x, res.y);

    // For bgfx, polygon offset is handled in the state
    // RenderInterface::SetState(BGFX_STATE_POLYGON_OFFSET | BGFX_STATE_WRITE_Z);

    //
    // A) Depth‑only pass
    //
    // For bgfx, we use SetViewClear to set up the clear state
    RenderInterface::SetViewClear(0, RenderInterface::DEPTH_BUFFER_BIT, 0x000000ff, 1.0f, 0);
    RenderInterface::Touch(0);


    for (auto* mesh : VissibleRenderList) 
    {
        if (mesh->Transparent)
        {
            //continue;
        }
        const mat4& P = mesh->IsViewmodel
            ? Camera::finalizedProjectionViewmodel
            : Camera::finalizedProjection;
        mesh->DrawDepth(Camera::finalizedView, P);

    }
    // For bgfx, we don't need to call UseProgram(0)

    forwardFBO->unbind();
    // For bgfx, polygon offset is handled in the state

    forwardFBO->resolve(*forwardResolveFBO,
        RenderInterface::COLOR_BUFFER_BIT | RenderInterface::DEPTH_BUFFER_BIT,
        RenderInterface::LINEAR);

    forwardFBO->bind();

    //
    // B) Opaque + transparent color passes
    //
    // For bgfx, we use SetViewClear to set up the clear state
    RenderInterface::SetViewClear(0, RenderInterface::COLOR_BUFFER_BIT, 0x000000ff, 1.0f, 0);
    RenderInterface::Touch(0);

    // For bgfx, depth function is handled in the state
    for (auto* mesh : VissibleRenderList) {
        if (mesh->Transparent) continue;
        const mat4& P = mesh->IsViewmodel
            ? Camera::finalizedProjectionViewmodel
            : Camera::finalizedProjection;
        mesh->DrawForward(Camera::finalizedView, P);
    }
    // For bgfx, we don't need to call UseProgram(0)
    // For bgfx, blend state is handled in the state
    // RenderInterface::SetState(BGFX_STATE_BLEND_ALPHA);

    // For bgfx, depth function is handled in the state

    Level::Current->BspData.RenderTransparentFaces();

    for (auto* mesh : VissibleRenderList) {
        if (!mesh->Transparent) continue;
        const mat4& P = mesh->IsViewmodel
            ? Camera::finalizedProjectionViewmodel
            : Camera::finalizedProjection;
        mesh->DrawForward(Camera::finalizedView, P);
    }

    DebugDraw::Draw();
    // For bgfx, we don't need to call UseProgram(0)

    // For bgfx, color mask and clear are handled in SetViewClear
    // This is a simplified approach - in practice, you'd want to handle this more carefully

    forwardFBO->unbind();

    // 2) resolve to single‐sample FBO
    forwardFBO->resolve(*forwardResolveFBO,
        RenderInterface::COLOR_BUFFER_BIT | RenderInterface::DEPTH_BUFFER_BIT,  
        RenderInterface::NEAREST);

}

void Renderer::RenderDirectionalLightShadows(vector<IDrawMesh*>& ShadowRenderList, Framebuffer& fbo, int numCascades)
{
    // For bgfx, we don't need to call UseProgram(0)
    fbo.bind();

    int halfRes = LightManager::ShadowMapResolution / 2;

    // For bgfx, we use SetViewClear to set up the clear state
    RenderInterface::SetViewClear(0, RenderInterface::DEPTH_BUFFER_BIT, 0x000000ff, 1.0f, 0);
    RenderInterface::Touch(0);

    // For bgfx, depth test, color mask, and cull face are handled in the state

    RenderInterface::SetViewport(0, 0, halfRes, halfRes);
    for (auto* mesh : ShadowRenderList) {
        mesh->DrawShadow(LightManager::lightView1, LightManager::lightProjection1);
    }

    RenderInterface::SetViewport(halfRes, 0, halfRes, halfRes);
    for (auto* mesh : ShadowRenderList) {
        mesh->DrawShadow(LightManager::lightView2, LightManager::lightProjection2);
    }

    RenderInterface::SetViewport(0, halfRes, halfRes, halfRes);
    for (auto* mesh : ShadowRenderList) {
        mesh->DrawShadow(LightManager::lightView3, LightManager::lightProjection3);
    }

    RenderInterface::SetViewport(halfRes, halfRes, halfRes, halfRes);
    for (auto* mesh : ShadowRenderList) {
        mesh->DrawShadow(LightManager::lightView4, LightManager::lightProjection4);
    }
    // For bgfx, we don't need to call UseProgram(0)
}

void Renderer::RenderFullscreenQuad(uint32_t textureID)
{
    // For bgfx, dither and depth test are handled in the state
    // RenderInterface::SetState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);

	fullscreenShader->UseProgram();

    ivec2 screenResolution = GetScreenResolution();

	
	fullscreenShader->SetTexture("screenTexture", textureID);
    fullscreenShader->SetUniform("screenResolution", screenResolution);
	// Draw quad with bgfx
	RenderInterface::SetVertexBuffer(0, quadVBO, 0, 4);
	RenderInterface::Submit(0, fullscreenShader->program, 0, 0);


}

void Renderer::SetSurfaceShaderUniforms(ShaderProgram* shader)
{
    if (shader == nullptr) return;

    shader->SetUniform("lightDirection", LightManager::LightDirection);

    shader->SetUniform("brightness", 1.0f);


    if (LightManager::DirectionalShadowsEnabled)
    {
        shader->SetUniform("lightMatrix1", LightManager::lightProjection1 * LightManager::lightView1);
        shader->SetUniform("lightMatrix2", LightManager::lightProjection2 * LightManager::lightView2);
        shader->SetUniform("lightMatrix3", LightManager::lightProjection3 * LightManager::lightView3);
        shader->SetUniform("lightMatrix4", LightManager::lightProjection4 * LightManager::lightView4);


        shader->SetTexture("shadowMap", EngineMain::MainInstance->MainRenderer->DirectionalShadowMap->id());
        shader->SetTexture("shadowMapDetail", EngineMain::MainInstance->MainRenderer->DetailDirectionalShadowMap->id());
        shader->SetTexture("shadowMapRaw", EngineMain::MainInstance->MainRenderer->DirectionalShadowMap->id());
        shader->SetTexture("shadowMapDetailRaw", EngineMain::MainInstance->MainRenderer->DetailDirectionalShadowMap->id());
        shader->SetUniform("shadowDistance1", LightManager::LightDistance1);
        shader->SetUniform("shadowDistance2", LightManager::LightDistance2);
        shader->SetUniform("shadowDistance3", LightManager::LightDistance3);
        shader->SetUniform("shadowDistance4", LightManager::LightDistance4);

        shader->SetUniform("shadowRadius1", LightManager::LightRadius1);
        shader->SetUniform("shadowRadius2", LightManager::LightRadius2);
        shader->SetUniform("shadowRadius3", LightManager::LightRadius3);
        shader->SetUniform("shadowRadius4", LightManager::LightRadius4);
    }
    else
    {
        //shader->SetTexture("shadowMap", nullptr);
       // shader->SetTexture("shadowMapDetail", nullptr);
        //shader->SetTexture("shadowMapRaw", nullptr);
        //shader->SetTexture("shadowMapDetailRaw", nullptr);
    }

    shader->SetTexture("depthTexture", EngineMain::MainInstance->MainRenderer->depthResolveBuffer->id());

    shader->SetUniform("fog_start", FogManager::StartDistance);
    shader->SetUniform("fog_end", FogManager::EndDistance);
    shader->SetUniform("fog_opacity", FogManager::Opacity);
    shader->SetUniform("fog_color", FogManager::Color);


    shader->SetUniform("cameraPosition", Camera::finalizedPosition);
    shader->SetUniform("shadowMapSize", LightManager::ShadowMapResolution);

    shader->SetUniform("shaddowOffsetScale", LightManager::ShaddowOffsetScale);

}

inline ivec2 Renderer::GetScreenResolution() const
{
	return ivec2(EngineMain::MainInstance->ScreenSize.x, EngineMain::MainInstance->ScreenSize.y);
}

void Renderer::InitFullscreenVAO()
{
	float quadVertices[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
	};

	// Create vertex layout for bgfx
	bgfx::VertexLayout layout;
	layout.begin()
		.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		.end();

	// Create vertex buffer with bgfx
	void* mem = RenderInterface::Alloc(sizeof(quadVertices));
	memcpy(mem, quadVertices, sizeof(quadVertices));
	quadVBO = RenderInterface::CreateVertexBuffer(mem, sizeof(quadVertices), &layout, 0);
	RenderInterface::Free(mem);
}

void Renderer::InitFrameBuffers()
{

    TextureFormat colorTextureFormat = TextureFormat::RGB8;

    ivec2 screenResolution = GetScreenResolution();

    if (colorBuffer != nullptr)
    {
        delete(colorBuffer);
    }

    if (depthBuffer != nullptr)
    {
        delete(depthBuffer);
    }

    if (forwardFBO != nullptr)
    {
        delete(forwardFBO);
    }

    TextureType textureType = (MultiSampleCount > 0) ? TextureType::Texture2DMultisample : TextureType::Texture2D;

    colorBuffer = new RenderTexture(screenResolution.x, screenResolution.y, colorTextureFormat, textureType, false, GL_LINEAR, GL_LINEAR,
        GL_CLAMP_TO_EDGE, 1);

    depthBuffer = new RenderTexture(screenResolution.x, screenResolution.y, TextureFormat::Depth24, textureType, false, GL_LINEAR, GL_LINEAR,
        GL_CLAMP_TO_EDGE, 1);

    forwardFBO = new Framebuffer();
    forwardFBO->attachDepth(depthBuffer);
    forwardFBO->attachColor(colorBuffer, 0);

    // resize all our buffers
    colorBuffer->resize(screenResolution.x, screenResolution.y);
    depthBuffer->resize(screenResolution.x, screenResolution.y);
    colorBuffer->setSamples(MultiSampleCount);
    depthBuffer->setSamples(MultiSampleCount);

}

void Renderer::InitResolveFrameBuffers()
{

    ivec2 screenResolution = GetScreenResolution();
    TextureFormat colorTextureFormat = TextureFormat::RGBA16F;

    colorTextureFormat = TextureFormat::RGB8;

    colorResolveBuffer = new RenderTexture(screenResolution.x, screenResolution.y, colorTextureFormat, TextureType::Texture2D);
    depthResolveBuffer = new RenderTexture(screenResolution.x, screenResolution.y, TextureFormat::Depth24, TextureType::Texture2D);

    forwardResolveFBO = new Framebuffer();

    forwardResolveFBO->attachDepth(depthResolveBuffer);
    forwardResolveFBO->attachColor(colorResolveBuffer);

    colorResolveBuffer->resize(screenResolution.x, screenResolution.y);
    depthResolveBuffer->resize(screenResolution.x, screenResolution.y);

}
