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

	void RenderFullscreenQuad(GLuint textureID);

private:

	inline ivec2 GetScreenResolution() const;

	void InitFullscreenVAO();

	unsigned int quadVAO;
	unsigned int quadVBO;

	RenderTexture* colorBuffer;
	RenderTexture* depthBuffer;

	RenderTexture* colorResolveBuffer;
	RenderTexture* depthResolveBuffer;

	Framebuffer forwardFBO;
	Framebuffer forwardResolveFBO;

	ShaderProgram* fullscreenShader;

};