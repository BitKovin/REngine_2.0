#include "StaticMeshDrawCommand.h"

#include "../Renderer/Renderer.h"
#include "../LightSystem/LightManager.h"
#include "../ShaderManager.h"
#include "../Shader.hpp"
#include "../skinned_model.hpp"

#include <BgfxStateManager.h>
#include <Renderer/Abstractions/ViewIdManager.h>

#include <algorithm>

void StaticMeshDrawCommand::ApplyAdditionalShaderParams(Shader* shader)
{
	for (auto& param : CustomShaderParams)
	{
		shader->SetUniform(param.first, param.second);
	}
}

void StaticMeshDrawCommand::DrawForward(const mat4& view, const mat4& projection)
{
	if (!bgfx::isValid(VertexBuffer) || !bgfx::isValid(IndexBuffer)) return;

	auto startState = BgfxStateManager::GetState();

	const bool isTransparent = GetSurfaceType() == SurfaceType::Transparent;

	// Transparent submeshes never write depth, regardless of DepthWrite.
	BgfxStateManager::SetWriteDepth(DepthWrite && !isTransparent);

	BgfxStateManager::SetBlend(BlendMode);
	BgfxStateManager::SetCull(ResolveCull());

	Shader* shader = GetForwardShader();
	shader->UseProgram();

	shader->SetUniform("masked", Material.EffectiveSurfaceType() == SurfaceType::Masked);

	Renderer::SetSurfaceShaderUniforms(shader, Brightness);

	shader->SetUniform("modelColor", Material.Color);

	shader->SetUniform("light_color", LightAmbientColor);
	shader->SetUniform("direct_light_color", LightDirectColor);
	shader->SetUniform("direct_light_dir", LightDirection);

	shader->SetUniform("view", view);
	shader->SetUniform("projection", projection);
	shader->SetUniform("viewmodelScaleFactor", Material.ViewmodelScaleFactor);

	shader->SetUniform("world", WorldMatrix);

	shader->SetUniform("isViewmodel", IsViewmodel);

	shader->SetUniform("customId", Material.CustomId);
	shader->SetUniform("flipedNormals", false);

	ApplyAdditionalShaderParams(shader);

	LightManager::ApplyPointLightToShader(shader, WorldBounds.Min, WorldBounds.Max);

	Material.BindColorTexture(shader);
	Material.BindEmissiveTexture(shader);

	bgfx::setVertexBuffer(0, VertexBuffer);
	SubmitIndexed(shader, IndexBuffer, IndexRanges, ViewIdManager::GetCurrentId());

	BgfxStateManager::SetState(startState);
}

void StaticMeshDrawCommand::DrawDepth(const mat4& view, const mat4& projection)
{
	if (!DepthPrePass) return;
	if (!bgfx::isValid(VertexBuffer) || !bgfx::isValid(IndexBuffer)) return;

	auto startState = BgfxStateManager::GetState();

	BgfxStateManager::SetCull(ResolveCull());

	Shader* shader = GetDepthShader();
	shader->UseProgram();

	shader->SetUniform("view", view);
	shader->SetUniform("projection", projection);
	shader->SetUniform("viewmodelScaleFactor", Material.ViewmodelScaleFactor);
	shader->SetUniform("world", WorldMatrix);
	shader->SetUniform("isViewmodel", IsViewmodel);

	ApplyAdditionalShaderParams(shader);

	// fs_empty doesn't sample a texture - only bind one when alpha-testing.
	if (Material.NeedsAlphaDiscard())
		Material.BindColorTexture(shader);

	bgfx::setVertexBuffer(0, VertexBuffer);
	SubmitIndexed(shader, IndexBuffer, IndexRanges, ViewIdManager::GetCurrentId());

	BgfxStateManager::SetState(startState);
}

void StaticMeshDrawCommand::DrawCustomId(const mat4& view, const mat4& projection)
{
	if (Material.CustomId == 0) return;
	if (!bgfx::isValid(VertexBuffer) || !bgfx::isValid(IndexBuffer)) return;

	auto startState = BgfxStateManager::GetState();

	BgfxStateManager::SetCull(ResolveCull());

	Shader* shader = GetCustomIdShader();
	shader->UseProgram();

	shader->SetUniform("view", view);
	shader->SetUniform("projection", projection);
	shader->SetUniform("viewmodelScaleFactor", Material.ViewmodelScaleFactor);
	shader->SetUniform("world", WorldMatrix);
	shader->SetUniform("customId", Material.CustomId);
	shader->SetUniform("isViewmodel", IsViewmodel);

	ApplyAdditionalShaderParams(shader);

	Material.BindColorTexture(shader);

	bgfx::setVertexBuffer(0, VertexBuffer);
	SubmitIndexed(shader, IndexBuffer, IndexRanges, ViewIdManager::GetCurrentId());

	BgfxStateManager::SetState(startState);
}

void StaticMeshDrawCommand::DrawShadow(const mat4& view, const mat4& projection)
{
	if (!bgfx::isValid(VertexBuffer) || !bgfx::isValid(IndexBuffer)) return;

	auto startState = BgfxStateManager::GetState();

	BgfxStateManager::SetCull(ResolveCull());

	Shader* shader = GetDepthShader();
	shader->UseProgram();

	shader->SetUniform("view", view);
	shader->SetUniform("projection", projection);
	shader->SetUniform("world", WorldMatrix);
	shader->SetUniform("isViewmodel", false);

	ApplyAdditionalShaderParams(shader);

	if (Material.NeedsAlphaDiscard())
		Material.BindColorTexture(shader);

	bgfx::setVertexBuffer(0, VertexBuffer);
	SubmitIndexed(shader, IndexBuffer, IndexRanges, ViewIdManager::GetCurrentId());

	BgfxStateManager::SetState(startState);
}

// Per submesh - Renderer::DrawDetailShadows loops these directly.
void StaticMeshDrawCommand::DrawShadowVolumeStencil(const mat4& view, const mat4& projection)
{
	const bool hasCap  = bgfx::isValid(ShadowCapVbh)  && ShadowCapIndexCount  > 0;
	const bool hasEdge = bgfx::isValid(ShadowEdgeVbh) && ShadowEdgeIndexCount > 0;
	if (!hasCap && !hasEdge) return;

	const vec3 fakeLightPos = normalize(ShadowLightDirection) * 1e6f;
	const bgfx::ViewId viewId = ViewIdManager::GetCurrentId();

	Shader* capShader  = nullptr;
	Shader* edgeShader = nullptr;
	GetShadowVolumeShaders(capShader, edgeShader);

	auto setCommonUniforms = [&](Shader* sh)
		{
			sh->UseProgram();
			sh->SetUniform("world", WorldMatrix);
			sh->SetUniform("view", view);
			sh->SetUniform("projection", projection);
			sh->SetUniform("u_shadowLightPos", vec4(fakeLightPos, 0.0f));
			ApplyAdditionalShaderParams(sh);
		};

	BgfxStateManager::SetCull(BgfxStateManager::Cull::None);
	BgfxStateManager::SetWriteRGB(false);
	BgfxStateManager::SetWriteAlpha(false);
	BgfxStateManager::SetWriteDepth(false);
	BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Less);
	BgfxStateManager::SetMSAA(false);
	BgfxStateManager::Apply();

	const uint32_t frontStencil =
		BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_FUNC_REF(0) |
		BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_S_KEEP |
		BGFX_STENCIL_OP_FAIL_Z_DECR | BGFX_STENCIL_OP_PASS_Z_KEEP;

	const uint32_t backStencil =
		BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_FUNC_REF(0) |
		BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_S_KEEP |
		BGFX_STENCIL_OP_FAIL_Z_INCR | BGFX_STENCIL_OP_PASS_Z_KEEP;

	if (hasCap)
	{
		setCommonUniforms(capShader);
		bgfx::setStencil(frontStencil, backStencil);
		bgfx::setVertexBuffer(0, ShadowCapVbh);
		SubmitIndexed(capShader, ShadowCapIbh, ShadowCapRanges, viewId);
	}

	if (hasEdge)
	{
		setCommonUniforms(edgeShader);
		bgfx::setStencil(frontStencil, backStencil);
		bgfx::setVertexBuffer(0, ShadowEdgeVbh);
		SubmitIndexed(edgeShader, ShadowEdgeIbh, ShadowEdgeRanges, viewId);
	}
}

void StaticMeshDrawCommand::SubmitIndexed(Shader* shader, bgfx::IndexBufferHandle ib,
	const std::vector<DrawIndexRange>& ranges, bgfx::ViewId viewId)
{
	if (ranges.empty())
	{
		bgfx::setIndexBuffer(ib);
		BgfxStateManager::Apply();
		shader->Submit(viewId);
		return;
	}

	// Find the last non-empty range: it must carry BGFX_DISCARD_ALL.
	int last = -1;
	for (int i = 0; i < (int)ranges.size(); ++i)
		if (ranges[i].count > 0) last = i;

	if (last < 0)
	{
		bgfx::discard(BGFX_DISCARD_ALL);   // nothing to draw: drop the vertex buffer/state already set
		return;
	}

	BgfxStateManager::Apply();
	for (int i = 0; i <= last; ++i)
	{
		const DrawIndexRange& r = ranges[i];
		if (r.count == 0) continue;
		if (RangeParamUniform) shader->SetUniform(RangeParamUniform, r.param);
		bgfx::setIndexBuffer(ib, r.first, r.count);
		shader->Submit(viewId, i == last ? BGFX_DISCARD_ALL : BGFX_DISCARD_INDEX_BUFFER);
	}
}

void SkeletalMeshDrawCommand::ApplyAdditionalShaderParams(Shader* shader)
{
	StaticMeshDrawCommand::ApplyAdditionalShaderParams(shader);

	if (!BoneMatrices || BoneMatrices->empty())
		return;

	// The palette goes up as two arrays (bgfx stores a uniform array's size in 8 bits) - see SKINNED_BONES_PER_UNIFORM.
	const mat4* palette = BoneMatrices->data();
	const size_t lowCount = std::min(BoneMatrices->size(), static_cast<size_t>(SKINNED_BONES_PER_UNIFORM));
	shader->SetUniform("finalBonesMatrices0", palette, lowCount);
	if (BoneMatrices->size() > lowCount)
		shader->SetUniform("finalBonesMatrices1", palette + lowCount, BoneMatrices->size() - lowCount);

	// Shaders that still declare the single 128-slot array (games not moved to the split palette yet).
	shader->SetUniform("finalBonesMatrices", palette, lowCount);
}
