#include "Renderer.h"

#include "../EngineMain.h"

Renderer::Renderer()
{
	ivec2 screenResolution = GetScreenResolution();

	colorBuffer = new RenderTexture(screenResolution.x, screenResolution.y, TextureFormat::RGB8, TextureType::Texture2DMultisample, GL_LINEAR, GL_LINEAR,
		GL_CLAMP_TO_EDGE, 1);

	depthBuffer = new RenderTexture(screenResolution.x, screenResolution.y, TextureFormat::Depth24, TextureType::Texture2DMultisample, GL_LINEAR, GL_LINEAR,
		GL_CLAMP_TO_EDGE, 1);

	colorResolveBuffer = new RenderTexture(screenResolution.x, screenResolution.y, TextureFormat::RGBA16F, TextureType::Texture2D);
	depthResolveBuffer = new RenderTexture(screenResolution.x, screenResolution.y, TextureFormat::Depth24, TextureType::Texture2D);

	forwardFBO.attachDepth(depthBuffer);
	forwardFBO.attachColor(colorBuffer, 0);

	forwardResolveFBO.attachDepth(depthResolveBuffer);
	forwardResolveFBO.attachColor(colorResolveBuffer);



	fullscreenShader = ShaderManager::GetShaderProgram("fullscreen_vertex","texture_pixel");

    // resize all our buffers
    colorBuffer->resize(screenResolution.x, screenResolution.y);
    depthBuffer->resize(screenResolution.x, screenResolution.y);
    colorBuffer->setSamples(2);
    colorBuffer->setSamples(2);

    colorResolveBuffer->resize(screenResolution.x, screenResolution.y);
    depthResolveBuffer->resize(screenResolution.x, screenResolution.y);

	InitFullscreenVAO();

}

Renderer::~Renderer()
{
	delete(colorBuffer);
	delete(depthBuffer);

	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);
}

void Renderer::RenderLevel(Level* level)
{

	vector<IDrawMesh*> renderList = level->VissibleRenderList;

	RenderCameraForward(renderList);

	//rendering to screen
	ivec2 screenResolution = GetScreenResolution();
	glViewport(0,0,screenResolution.x, screenResolution.y);
	RenderFullscreenQuad(colorResolveBuffer->id());

}

void Renderer::RenderCameraForward(vector<IDrawMesh*>& VissibleRenderList)
{

    ivec2 res = GetScreenResolution();

    // resize all our buffers
    colorBuffer->resize(res.x, res.y);
    depthBuffer->resize(res.x, res.y);
    colorBuffer->setSamples(2);
    colorBuffer->setSamples(2);

    colorResolveBuffer->resize(res.x, res.y);
    depthResolveBuffer->resize(res.x, res.y);

    // 1) bind the one multisample FBO with both attachments
    forwardFBO.bind();

    glViewport(0, 0, res.x, res.y);

    //
    // A) Depth‑only pass
    //
    // disable color writes, enable depth writes, clear depth
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    for (auto* mesh : VissibleRenderList) {
        if (mesh->Transparent) continue;
        const mat4& P = mesh->IsViewmodel
            ? Camera::finalizedProjectionViewmodel
            : Camera::finalizedProjection;
        mesh->DrawDepth(Camera::finalizedView, P);
    }

    //
    // B) Opaque + transparent color passes
    //
    // re‑enable color writes, disable depth writes, clear color    
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_FALSE);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw opaque where depth ==
    glDepthFunc(GL_EQUAL);
    for (auto* mesh : VissibleRenderList) {
        if (mesh->Transparent) continue;
        const mat4& P = mesh->IsViewmodel
            ? Camera::finalizedProjectionViewmodel
            : Camera::finalizedProjection;
        mesh->DrawForward(Camera::finalizedView, P);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // draw transparent with normal depth test
    glDepthFunc(GL_LESS);
    for (auto* mesh : VissibleRenderList) {
        if (!mesh->Transparent) continue;
        const mat4& P = mesh->IsViewmodel
            ? Camera::finalizedProjectionViewmodel
            : Camera::finalizedProjection;
        mesh->DrawForward(Camera::finalizedView, P);
    }


    forwardFBO.unbind();

    // 2) resolve to single‐sample FBO
    forwardFBO.resolve(forwardResolveFBO,
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,  
        GL_LINEAR);

}

void Renderer::RenderFullscreenQuad(GLuint textureID)
{

	glDisable(GL_DEPTH_TEST);
	fullscreenShader->UseProgram();

	// Bind texture to unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	fullscreenShader->SetTexture("screenTexture", textureID);

	// Draw quad
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);

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

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindVertexArray(0);
}
