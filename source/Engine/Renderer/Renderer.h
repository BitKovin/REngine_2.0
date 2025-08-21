#pragma once

#include "../Level.hpp"
#include "../glm.h"

#include "Abstractions/FrameBuffer.h"
#include "Abstractions/RenderTexture.h"

#include "../ShaderManager.h"
#include "../Shader.hpp"

class Renderer
{
public:
	Renderer();
	~Renderer();

	void RenderLevel(Level* level);

	void RenderCameraForward(vector<IDrawMesh*>& VissibleRenderList);

	void RenderDirectionalLightShadows(vector<IDrawMesh*>& ShadowRenderList, Framebuffer& fbo, int numCascades);

	void RenderFullscreenQuad(GLuint textureID);

	static void SetSurfaceShaderUniforms(ShaderProgram* shader);

	int MultiSampleCount = 1;

private:

	inline ivec2 GetScreenResolution() const;

	void InitFullscreenVAO();

	unsigned int quadVAO;
	unsigned int quadVBO;

	RenderTexture* colorBuffer;
	RenderTexture* depthBuffer;

	RenderTexture* depthBufferCopy;
	RenderTexture* depthBufferCopyResolve;

	RenderTexture* colorResolveBuffer;
	RenderTexture* depthResolveBuffer;

	Framebuffer forwardFBO;
	Framebuffer forwardResolveFBO;

	RenderTexture* DirectionalShadowMap;
	Framebuffer DirectionalShadowMapFBO;

	RenderTexture* DetailDirectionalShadowMap;
	Framebuffer DetailDirectionalShadowMapFBO;

	ShaderProgram* fullscreenShader;

};