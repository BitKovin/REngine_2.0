#pragma once

#include "../Level.hpp"
#include "../glm.h"

#include "Abstractions/FrameBuffer.h"
#include "Abstractions/RenderTexture.h"

#include "../ShaderManager.h"
#include "../Shader.hpp"
#include "RHI/RenderInterface.h"
#include "RHI/Bgfx/bgfx_wrapper.h"

class Renderer
{
public:
	Renderer();
	~Renderer();

	void RenderLevel(Level* level);

	void RenderCameraForward(vector<IDrawMesh*>& VissibleRenderList);

	void RenderDirectionalLightShadows(vector<IDrawMesh*>& ShadowRenderList, Framebuffer& fbo, int numCascades);

	void RenderFullscreenQuad(uint32_t textureID);

	static void SetSurfaceShaderUniforms(ShaderProgram* shader);

	int MultiSampleCount = 1;

private:

	inline ivec2 GetScreenResolution() const;

	void InitFullscreenVAO();

	void InitFrameBuffers();
	void InitResolveFrameBuffers();

	uint32_t quadVBO;

	RenderTexture* colorBuffer = nullptr;
	RenderTexture* depthBuffer = nullptr;

	RenderTexture* colorResolveBuffer = nullptr;
	RenderTexture* depthResolveBuffer = nullptr;

	Framebuffer* forwardFBO = nullptr;
	Framebuffer* forwardResolveFBO = nullptr;

	RenderTexture* DirectionalShadowMap = nullptr;
	Framebuffer* DirectionalShadowMapFBO = nullptr;

	RenderTexture* DetailDirectionalShadowMap = nullptr;
	Framebuffer* DetailDirectionalShadowMapFBO = nullptr;

	ShaderProgram* fullscreenShader = nullptr;

};