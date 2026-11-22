#include "WeaponIconBaker.h"

#ifndef DISTRIBUTION

#include <WeaponDefs.h>
#include "../../Entities/Player/Player.hpp"

#include <StaticMesh.h>
#include <SkeletalMesh.hpp>
#include <DrawCommands/StaticMeshDrawCommand.h>
#include <Time.hpp>
#include <Level.hpp>
#include <Logger.hpp>
#include <Screenshot.h>
#include <BgfxStateManager.h>
#include <Renderer/Abstractions/RenderTexture.h>
#include <Renderer/Abstractions/FrameBuffer.h>
#include <Renderer/Abstractions/ViewIdManager.h>
#include <Console/ConsoleRegister.h>
#include <Console/Console.h>

#include <includedLibraries/stb_image.h>
#include <includedLibraries/stb_image_write.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <deque>
#include <filesystem>
#include <fstream>
#include <set>

// Set by CMake for this file only: <game folder>/GameData (see the line next to UiSourceSync.cpp in CMakeLists.txt).
#ifndef ENGINE_GAME_SOURCE_DATA
#define ENGINE_GAME_SOURCE_DATA ""
#endif

namespace fs = std::filesystem;

namespace
{
	// Icon size: 8:3, drawn by the HUD at 150 x 52 with object-fit: contain.
	constexpr uint16_t kWidth = 512;
	constexpr uint16_t kHeight = 192;
	constexpr float kMargin = 0.09f;            // free space around the model, fraction of the icon (room for the thickening)
	// Max Payne 1 style: a flat white silhouette, thickened a little so thin barrels
	// and stocks read as solid shapes at HUD size (pixels of the 512 x 192 capture;
	// the HUD shows icons about 1/4 of that). Colour and outline come from the HUD's CSS.
	constexpr int kThicken = 4;
	constexpr float kLoadTimeout = 15.0f;      // seconds to wait for a model before giving up on it
	constexpr int kWarmupFrames = 12;            // frames drawn before the capture is requested
	constexpr float kCaptureTimeout = 5.0f;
		constexpr float kSettleSeconds = 0.5f;
		constexpr float kWarmupSeconds = 0.6f;      // real time too: finalize and texture streaming run on other threads
		constexpr float kTextureTimeout = 4.0f;     // seconds to wait for textures once the model is up

	// Fixed icon lighting (world space, the model sits at the origin).
	const glm::vec3 kAmbient = glm::vec3(0.45f);
	const glm::vec3 kDirect = glm::vec3(1.1f);
	const glm::vec3 kLightDirection = glm::normalize(glm::vec3(-0.3f, -1.0f, 0.5f));

	// The camera looks at the model's side along -X (from +X), Y up. A weapon
	// that comes out facing the wrong way is fixed with iconYaw in the table.
	const glm::vec3 kViewDirection = glm::vec3(-1, 0, 0);

	std::set<std::string> forcedClasses;
	bool forceAll = false;
	bool bakedThisRun = false;

	std::string RawPath(const std::string& className)
	{
		return "SaveData/weapon_icons_raw/" + className + ".png";
	}

	fs::path SourceDataDir()
	{
		return fs::path(ENGINE_GAME_SOURCE_DATA);
	}

	// "GameData/x/y.png" -> <game>/GameData/x/y.png, or empty when the source folder is unknown.
	fs::path SourcePathOf(const std::string& gameDataPath)
	{
		fs::path dataDir = SourceDataDir();
		if (dataDir.empty())
			return fs::path();
		return dataDir.parent_path() / gameDataPath;
	}

	bool NeedsBake(const WeaponDef& def)
	{
		if (forceAll || forcedClasses.count(def.className))
			return true;
		if (def.iconModel.empty())
			return false;

		std::error_code ec;
		fs::path icon = SourcePathOf(def.IconPath());
		fs::path model = SourcePathOf(def.iconModel);
		if (icon.empty())
		{
			icon = def.IconPath();
			model = def.iconModel;
		}

		if (!fs::exists(icon, ec))
			return true;
		if (!fs::exists(model, ec))
			return false;
		return fs::last_write_time(model, ec) > fs::last_write_time(icon, ec);
	}

	bool WriteBoth(const std::string& gameDataPath, const std::vector<uint8_t>& png)
	{
		bool ok = false;
		std::error_code ec;

		auto write = [&](const fs::path& path)
			{
				fs::create_directories(path.parent_path(), ec);
				std::ofstream out(path, std::ios::binary | std::ios::trunc);
				if (!out)
				{
					Logger::Error("WeaponIconBaker: cannot write %s", path.string().c_str());
					return false;
				}
				out.write(reinterpret_cast<const char*>(png.data()), (std::streamsize)png.size());
				return true;
			};

		fs::path source = SourcePathOf(gameDataPath);
		if (!source.empty())
			ok |= write(source);
		// No CMake define (e.g. the Visual Studio project, which runs the game from
		// the source folder): the running GameData is the one to write.

		ok |= write(fs::path(gameDataPath));
		return ok;
	}

	void AppendToVector(void* context, void* data, int size)
	{
		auto* out = static_cast<std::vector<uint8_t>*>(context);
		const uint8_t* bytes = static_cast<const uint8_t*>(data);
		out->insert(out->end(), bytes, bytes + size);
	}

	// Raw capture -> flat white silhouette, cropped and centred.
	bool FinishIcon(const std::string& rawPath, const std::string& iconPath)
	{
		int w = 0, h = 0, channels = 0;
		uint8_t* raw = stbi_load(rawPath.c_str(), &w, &h, &channels, 4);
		if (raw == nullptr)
		{
			Logger::Error("WeaponIconBaker: cannot read capture %s", rawPath.c_str());
			return false;
		}

		// 1. Coverage: the icon is the model's flat silhouette.
		std::vector<uint8_t> covered((size_t)w * h, 0);
		int minX = w, minY = h, maxX = -1, maxY = -1;

		for (int y = 0; y < h; ++y)
			for (int x = 0; x < w; ++x)
			{
				const uint8_t* p = raw + ((size_t)y * w + x) * 4;
				if (p[3] >= 8)
					covered[(size_t)y * w + x] = 1;
			}

		stbi_image_free(raw);

		// Drop specks: pieces far smaller than the weapon itself (stray detail
		// meshes, a muzzle flash quad) would show as dots and spoil the crop.
		{
			std::vector<int> label((size_t)w * h, -1);
			std::vector<int> sizes;
			std::vector<int> stack;
			for (int start = 0; start < w * h; ++start)
			{
				if (!covered[start] || label[start] >= 0)
					continue;
				const int id = (int)sizes.size();
				int size = 0;
				stack.push_back(start);
				label[start] = id;
				while (!stack.empty())
				{
					const int i = stack.back();
					stack.pop_back();
					++size;
					const int x = i % w, y = i / w;
					for (int dy = -1; dy <= 1; ++dy)
						for (int dx = -1; dx <= 1; ++dx)
						{
							const int nx = x + dx, ny = y + dy;
							if (nx < 0 || ny < 0 || nx >= w || ny >= h)
								continue;
							const int n = ny * w + nx;
							if (covered[n] && label[n] < 0)
							{
								label[n] = id;
								stack.push_back(n);
							}
						}
				}
				sizes.push_back(size);
			}

			const int largest = sizes.empty() ? 0 : *std::max_element(sizes.begin(), sizes.end());
			const int keep = std::max(24, largest / 50);
			for (int i = 0; i < w * h; ++i)
			{
				if (!covered[i])
					continue;
				if (sizes[label[i]] < keep)
				{
					covered[i] = 0;
					continue;
				}
				const int x = i % w, y = i / w;
				minX = std::min(minX, x); maxX = std::max(maxX, x);
				minY = std::min(minY, y); maxY = std::max(maxY, y);
			}
		}

		if (maxX < 0)
		{
			Logger::Error("WeaponIconBaker: %s is empty (the model drew nothing; check iconModel and iconYaw/Pitch/Roll)", rawPath.c_str());
			return false;
		}

		// 2. Thicken the silhouette a little.
		auto dilate = [&](const std::vector<uint8_t>& src, int radius)
			{
				std::vector<std::pair<int, int>> offsets;
				for (int dy = -radius; dy <= radius; ++dy)
					for (int dx = -radius; dx <= radius; ++dx)
						if (dx * dx + dy * dy <= radius * radius)
							offsets.push_back({ dx, dy });

				std::vector<uint8_t> dst(src.size(), 0);
				for (int y = 0; y < h; ++y)
					for (int x = 0; x < w; ++x)
					{
						if (!src[(size_t)y * w + x])
							continue;
						for (const auto& [dx, dy] : offsets)
						{
							const int nx = x + dx, ny = y + dy;
							if (nx >= 0 && ny >= 0 && nx < w && ny < h)
								dst[(size_t)ny * w + nx] = 1;
						}
					}
				return dst;
			};

		const std::vector<uint8_t> body = dilate(covered, kThicken);

		// 3. Crop to the silhouette plus a little room.
		const int pad = kThicken + 2;
		minX = std::max(0, minX - pad); minY = std::max(0, minY - pad);
		maxX = std::min(w - 1, maxX + pad); maxY = std::min(h - 1, maxY + pad);
		const int cw = maxX - minX + 1;
		const int ch = maxY - minY + 1;

		// 4. Centre the crop on an 8:3 canvas at its own scale (no resampling: the
		// camera already fitted the model to the target).
		const int outW = std::max(cw, (int)std::ceil(ch * (float)kWidth / kHeight));
		const int outH = std::max(ch, (int)std::ceil(outW * (float)kHeight / kWidth));
		const int offX = (outW - cw) / 2;
		const int offY = (outH - ch) / 2;

		std::vector<uint8_t> out((size_t)outW * outH * 4, 0);

		for (int y = 0; y < ch; ++y)
			for (int x = 0; x < cw; ++x)
			{
				const size_t i = (size_t)(minY + y) * w + (minX + x);
				uint8_t* o = &out[((size_t)(offY + y) * outW + (offX + x)) * 4];

				o[0] = o[1] = o[2] = 255;
				o[3] = body[i] ? 255 : 0;
			}

		std::vector<uint8_t> png;
		if (!stbi_write_png_to_func(AppendToVector, &png, outW, outH, 4, out.data(), outW * 4))
		{
			Logger::Error("WeaponIconBaker: cannot encode %s", iconPath.c_str());
			return false;
		}

		return WriteBoth(iconPath, png);
	}

	// Geometry uploaded and every base colour texture requested so far arrived.
	// Emissive maps are ignored: most models have none, and a missing one never
	// reaches the Visual tier (AllAssetsLoaded() would wait for it forever).
	bool ModelReady(const StaticMesh* mesh, bool texturesRequested = false)
	{
		if (mesh == nullptr || mesh->model == nullptr)
			return false;
		if (mesh->model->loadState.currentTier.load(std::memory_order_acquire) != AssetLoadTier::Visual)
			return false;
		for (const auto& sub : mesh->model->meshes)
		{
			if (texturesRequested && !sub.cachedBaseColor)
				return false;   // the first finalize has not asked for this texture yet
			if (sub.cachedBaseColor && sub.cachedBaseColor->loadState.currentTier.load(std::memory_order_acquire) != AssetLoadTier::Visual)
				return false;
		}
		return true;
	}

	// The model, lit with fixed icon lighting instead of the level's light grid.
	class IconMesh : public SkeletalMesh
	{
	public:
		IconMesh() : SkeletalMesh(nullptr) {}

		LightVolPointData GetLightVolData() override
		{
			return LightVolPointData{ kDirect, kAmbient, kLightDirection };
		}
	};
}

// ── The render pass ─────────────────────────────────────────────────────────
// An IDrawMesh on the baker entity: always "visible" so Level::FinalizeFrame
// puts it in the render list and Renderer::RenderLevel calls its PreDraw(),
// but it collects no draw commands of its own, so nothing shows in the world.
// The icon mesh lives outside the level; this pass finalizes it itself.
class WeaponIconPass : public IDrawMesh
{
public:

	IconMesh* mesh = nullptr;     // set by the baker while a weapon is in progress
	bool finalize = false;        // false until the model's embedded textures had time to register
	bool draw = false;            // main thread sets, PreDraw reads (same thread: RenderLevel runs on the main thread)
	glm::mat4 view = glm::mat4(1);
	glm::mat4 projection = glm::mat4(1);

	RenderTexture* color = nullptr;
	RenderTexture* depth = nullptr;
	Framebuffer* framebuffer = nullptr;

	WeaponIconPass()
	{
		IncludeInEntityBounds = false;
	}

	~WeaponIconPass() override
	{
		delete framebuffer;
		delete color;
		delete depth;
	}

	bool IsCameraVisible() override { return true; }
	bool IsShadowVisible() override { return false; }

	void PreFinalize() override
	{
		if (mesh != nullptr && finalize)
			mesh->PreFinalize();
	}

	void FinalizeFrameData() override
	{
		if (mesh != nullptr && finalize)
			mesh->FinalizeFrameData();
	}

	void PreDraw() override
	{
		if (!draw || mesh == nullptr)
			return;

		if (framebuffer == nullptr)
		{
			color = new RenderTexture(kWidth, kHeight, TextureFormat::RGBA8);
			depth = new RenderTexture(kWidth, kHeight, TextureFormat::Depth24Stencil8);
			framebuffer = new Framebuffer();
			framebuffer->attachColor(color, 0);
			framebuffer->attachDepth(depth);
		}

		// Same steps as the renderer's forward pass (Renderer::RenderCameraForward):
		// bind() takes a fresh current view that the draw commands submit to.
		framebuffer->bind(0, 0, kWidth, kHeight);
		const bgfx::ViewId viewId = ViewIdManager::GetCurrentId();
		bgfx::setViewMode(viewId, bgfx::ViewMode::Sequential);
		bgfx::setViewClear(viewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0x00000000, 1.0f, 0);
		bgfx::setViewName(viewId, "WeaponIconBake");

		const auto savedState = BgfxStateManager::GetState();

		BgfxStateManager::Reset();
		BgfxStateManager::SetWriteDepth(true);
		BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::LEqual);
		BgfxStateManager::SetCull(BgfxStateManager::Cull::CW);
		BgfxStateManager::SetMSAA(false);

		std::vector<IDrawCommand*> commands;
		mesh->CollectDrawCommands(commands);
		for (IDrawCommand* command : commands)
		{
			// Some weapon textures have an alpha channel full of zeros: draw everything opaque.
			if (auto* meshCommand = dynamic_cast<StaticMeshDrawCommand*>(command))
			{
				meshCommand->Material.SurfaceTypeOverride = SurfaceType::Opaque;

			}
			command->DrawForward(view, projection);
		}

		BgfxStateManager::SetState(savedState);
	}

	bgfx::TextureHandle ColorHandle() const
	{
		if (color == nullptr)
		{
			bgfx::TextureHandle invalid = BGFX_INVALID_HANDLE;
			return invalid;
		}
		return color->textureHandle();
	}
};

class WeaponIconBaker::Impl
{
public:

	enum class Step { Next, Loading, Drawing, Capturing };

	WeaponIconPass* pass = nullptr;
	std::deque<WeaponDef> queue;
	WeaponDef current;
	Step step = Step::Next;
	float timer = 0.0f;
	int frames = 0;
	float settle = 0.0f;
	int baked = 0;
	int failed = 0;

	void FitCamera()
	{
		IconMesh* mesh = pass->mesh;
		const BoundingBox box = mesh->GetBoundingBox();
		const glm::vec3 centre = (box.Min + box.Max) * 0.5f;
		const float radius = glm::length(box.Max - box.Min) * 0.5f + 0.01f;

		const glm::vec3 eye = centre - kViewDirection * (radius * 4.0f);
		pass->view = glm::lookAt(eye, centre, glm::vec3(0, 1, 0));

		// Extents of the box in view space.
		glm::vec2 lo(1e9f), hi(-1e9f);
		for (int i = 0; i < 8; ++i)
		{
			const glm::vec3 corner((i & 1) ? box.Max.x : box.Min.x, (i & 2) ? box.Max.y : box.Min.y, (i & 4) ? box.Max.z : box.Min.z);
			const glm::vec3 v = glm::vec3(pass->view * glm::vec4(corner, 1.0f));
			lo = glm::min(lo, glm::vec2(v));
			hi = glm::max(hi, glm::vec2(v));
		}

		glm::vec2 size = hi - lo;
		const glm::vec2 mid = (lo + hi) * 0.5f;
		size *= 1.0f + kMargin * 2.0f;

		// Grow one axis so the box keeps the icon's aspect.
		const float aspect = (float)kWidth / kHeight;
		if (size.x / size.y > aspect)
			size.y = size.x / aspect;
		else
			size.x = size.y * aspect;

		// glm::ortho with the default clip convention, like Camera's glm::perspective.
		pass->projection = glm::ortho(mid.x - size.x * 0.5f, mid.x + size.x * 0.5f, mid.y - size.y * 0.5f, mid.y + size.y * 0.5f, 0.01f, radius * 5.2f);   // the model lands at depth 0.15..0.92: inside the clip range of both -1..1 and 0..1 conventions
	}

	void Begin(const WeaponDef& def)
	{
		current = def;
		IconMesh* mesh = new IconMesh();
		mesh->Position = glm::vec3(0);
		// Table: yaw, pitch, roll. Engine rotations: x pitch, y yaw, z roll.
		mesh->Rotation = glm::vec3(def.iconRotation.y, def.iconRotation.x, def.iconRotation.z);
		mesh->CastShadows = false;
		mesh->TwoSided = true;
		mesh->blendMode = BgfxStateManager::Blend::None;   // textures of some weapons carry junk alpha
		mesh->Masked = false;
		mesh->TexturesLocation = def.iconModel + "/";   // textures are stored under "<model>.glb/"
		mesh->LoadFromFile(def.iconModel, AssetLoadTier::Visual);
		pass->mesh = mesh;
		pass->draw = false;
		pass->finalize = false;
		step = Step::Loading;
		timer = 0.0f;
		frames = 0;
		settle = 0.0f;
	}

	void EndCurrent(bool ok)
	{
		pass->draw = false;
		delete pass->mesh;
		pass->mesh = nullptr;
		(ok ? baked : failed)++;
		forcedClasses.erase(current.className);
		step = Step::Next;
	}

	// Returns false when the work is done.
	bool Tick(float dt)
	{
		timer += dt;

		switch (step)
		{
		case Step::Next:
			if (queue.empty())
				return false;
			Begin(queue.front());
			queue.pop_front();
			return true;

		case Step::Loading:
			if (ModelReady(pass->mesh))
			{
				// Embedded textures register on the main thread a little after the geometry:
				// the model stays out of the render list until they had time to (a texture
				// asked for too early is cached as "not found").
				settle += dt;
				if (settle < kSettleSeconds)
					return true;

				pass->mesh->PasteAnimationPose(AnimationPose());   // blank pose = rest pose
				pass->finalize = true;
				FitCamera();
				pass->draw = true;
				step = Step::Drawing;
				frames = 0;
				timer = 0.0f;
			}
			else if (timer > kLoadTimeout)
			{
				Logger::Error("WeaponIconBaker: %s did not load in %.0f s", current.iconModel.c_str(), kLoadTimeout);
				EndCurrent(false);
			}
			return true;

		case Step::Drawing:
			// A few frames so the draw commands are wired (PreFinalize), the
			// textures the first finalize requested have arrived, and the target
			// holds a finished image when the capture blits it.
			pass->mesh->PasteAnimationPose(AnimationPose());
			++frames;
			if (frames >= kWarmupFrames && timer >= kWarmupSeconds && (ModelReady(pass->mesh, true) || timer > kTextureTimeout))
			{
				std::error_code ec;
				fs::create_directories(fs::path(RawPath(current.className)).parent_path(), ec);
				fs::remove(RawPath(current.className), ec);
				Screenshot::RequestTarget(pass->ColorHandle(), kWidth, kHeight, RawPath(current.className));
				step = Step::Capturing;
				timer = 0.0f;
			}
			return true;

		case Step::Capturing:
		{
			std::error_code ec;
			if (fs::exists(RawPath(current.className), ec) && fs::file_size(RawPath(current.className), ec) > 0)
			{
				const bool ok = FinishIcon(RawPath(current.className), current.IconPath());
				if (ok)
				{
					Logger::Log("WeaponIconBaker: wrote %s", current.IconPath().c_str());
					if (Player::Instance != nullptr)
						Player::Instance->weaponGridRevision++;   // the HUD rebuilds the grid and picks the new icon up
				}
				EndCurrent(ok);
			}
			else if (timer > kCaptureTimeout)
			{
				Logger::Error("WeaponIconBaker: capture of %s never arrived", current.className.c_str());
				EndCurrent(false);
			}
			return true;
		}
		}
		return false;
	}
};

void WeaponIconBaker::StartIfNeeded()
{
	if (Level::Current == nullptr)
		return;
	if (bakedThisRun && !forceAll && forcedClasses.empty())
		return;

	bool any = false;
	for (const auto& [className, def] : WeaponDefs::All())
	{
		if (NeedsBake(def))
		{
			any = true;
			break;
		}
	}

	bakedThisRun = true;
	if (!any)
		return;

	WeaponIconBaker* baker = new WeaponIconBaker();
	Level::Current->AddEntity(baker);
	baker->Start();   // the level only starts the entities it loads with; one spawned later starts itself
}

void WeaponIconBaker::RequestRebake(const std::string& className)
{
	if (className.empty())
		forceAll = true;
	else
		forcedClasses.insert(className);

	bakedThisRun = false;
	StartIfNeeded();
}

WeaponIconBaker::WeaponIconBaker()
{
	SaveGame = false;
	ClassName = "weapon_icon_baker";
}

void WeaponIconBaker::Start()
{
	if (impl != nullptr)
		return;   // already started (StartIfNeeded starts it; a level load may call Start too)

	Entity::Start();

	impl = new Impl();
	impl->pass = new WeaponIconPass();
	Drawables.push_back(impl->pass);

	for (const auto& [className, def] : WeaponDefs::All())
	{
		if (NeedsBake(def))
			impl->queue.push_back(def);
	}
	forceAll = false;

	Logger::Log("WeaponIconBaker: %d icon(s) to render", (int)impl->queue.size());
}

void WeaponIconBaker::Update()
{
	Entity::Update();

	if (impl == nullptr)
		return;

	if (!impl->Tick(Time::DeltaTimeFNoTimeScale))
	{
		Logger::Log("WeaponIconBaker: done, %d written, %d failed", impl->baked, impl->failed);
		Destroy();
	}
}

void WeaponIconBaker::Destroy()
{
	if (impl != nullptr)
	{
		if (impl->pass != nullptr)
		{
			impl->pass->draw = false;
			delete impl->pass->mesh;
			impl->pass->mesh = nullptr;
		}
		delete impl;
		impl = nullptr;
	}

	Entity::Destroy();   // destroys Drawables (the pass)
}

CONSOLE_FUNC("weapon_icons.rebake", "weapon_icons.rebake [className] renders weapon grid icons again (all when no class is given)")
{
	const std::string className = Console::ArgString(args, 0, "");
	WeaponIconBaker::RequestRebake(className);
	Console::Get().AddLog("Weapon icons: rebake requested for %s", className.empty() ? "every weapon" : className.c_str());
}

#endif
