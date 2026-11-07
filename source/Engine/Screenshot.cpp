#include "Screenshot.h"

#include "Console/ConsoleRegister.h"
#include "FileSystem/FileSystem.h"
#include "Logger.hpp"

#include <includedLibraries/stb_image_write.h>

#include <algorithm>
#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <mutex>

namespace
{
	std::mutex requestMutex;
	std::string pendingPath;
	int pendingDownscale = 1;
	int activeDownscale = 1;     // the scale of the request bgfx is working on (one at a time)
	std::string pendingTargetPath;
	bgfx::TextureHandle pendingSource = BGFX_INVALID_HANDLE;
	uint16_t pendingWidth = 0;
	uint16_t pendingHeight = 0;

	// A target capture in flight: blit queued, read queued, waiting for the data to land.
	bgfx::TextureHandle readback = BGFX_INVALID_HANDLE;
	std::string readbackPath;
	std::vector<uint8_t> readbackPixels;
	uint16_t readbackWidth = 0;
	uint16_t readbackHeight = 0;
	uint32_t readbackReadyFrame = 0;
	const bgfx::ViewId kBlitView = 254;
	bool launchShotTaken = false;

	// Frame sharing (console: frame_share <downscale|off>). Every presented frame is copied, optionally shrunk, into the file
	// SaveData/ui_frame.bin so another process can show the game live by memory-mapping it. Layout: 32 byte header
	// { 'UIFR', width, height, stride, sequence } then BGRA rows, top row first. The sequence is odd while a frame is being written
	// and even when it is complete, so a reader copies the pixels and keeps the copy only if the sequence did not change meanwhile.
	constexpr const char* kShareMarker = "<frame_share>";
	constexpr const char* kSharePath = "SaveData/ui_frame.bin";
	constexpr uint32_t kShareHeader = 32;
	std::atomic<int> shareDownscale{ 0 };
	std::atomic<bool> shareInFlight{ false };
	std::FILE* shareFile = nullptr;
	uint32_t shareWidth = 0;
	uint32_t shareHeight = 0;
	uint32_t shareSequence = 0;

	void ShareClose()
	{
		if (shareFile)
			std::fclose(shareFile);
		shareFile = nullptr;
		shareWidth = shareHeight = 0;
	}

	void ShareSequence(uint32_t value)
	{
		shareSequence = value;
		std::fseek(shareFile, 16, SEEK_SET);
		std::fwrite(&value, 4, 1, shareFile);
		std::fflush(shareFile);
	}

	void ShareWrite(const uint8_t* source, uint32_t width, uint32_t height, uint32_t pitch, bool bgra, bool yflip)
	{
		const uint32_t d = static_cast<uint32_t>(std::max(1, shareDownscale.load()));
		const uint32_t w = std::max(1u, width / d);
		const uint32_t h = std::max(1u, height / d);
		if (!shareFile || w != shareWidth || h != shareHeight)
		{
			ShareClose();
			std::error_code ec;
			std::filesystem::create_directories("SaveData", ec);
			shareFile = std::fopen(kSharePath, "wb+");
			if (!shareFile)
			{
				Logger::Error("frame_share: cannot create %s (is another tool still reading an older size?)", kSharePath);
				shareDownscale = 0;
				return;
			}
			shareWidth = w;
			shareHeight = h;
			shareSequence = 0;
			const uint32_t header[8] = { 0x52465549u, w, h, w * 4, 0, 0, 0, 0 };   // 'UIFR'
			std::fwrite(header, sizeof(header), 1, shareFile);
			std::vector<uint8_t> blank(static_cast<size_t>(w) * h * 4, 0);
			std::fwrite(blank.data(), blank.size(), 1, shareFile);
			std::fflush(shareFile);
		}
		std::vector<uint8_t> out(static_cast<size_t>(w) * h * 4);
		for (uint32_t y = 0; y < h; ++y)
			for (uint32_t x = 0; x < w; ++x)
			{
				uint32_t sum[3] = { 0, 0, 0 };
				for (uint32_t dy = 0; dy < d; ++dy)
				{
					const uint32_t sy = y * d + dy;
					const uint8_t* row = source + static_cast<size_t>(yflip ? height - 1 - sy : sy) * pitch;
					for (uint32_t dx = 0; dx < d; ++dx)
					{
						const uint8_t* px = row + (x * d + dx) * 4;
						sum[0] += px[0]; sum[1] += px[1]; sum[2] += px[2];
					}
				}
				uint8_t* o = out.data() + (static_cast<size_t>(y) * w + x) * 4;
				const uint32_t n = d * d;
				const uint8_t c0 = static_cast<uint8_t>(sum[0] / n), c1 = static_cast<uint8_t>(sum[1] / n), c2 = static_cast<uint8_t>(sum[2] / n);
				o[0] = bgra ? c0 : c2;   // blue
				o[1] = c1;               // green
				o[2] = bgra ? c2 : c0;   // red
				o[3] = 255;
			}
		ShareSequence(shareSequence + 1u);                                      // odd: writing
		std::fseek(shareFile, kShareHeader, SEEK_SET);
		std::fwrite(out.data(), out.size(), 1, shareFile);
		ShareSequence(shareSequence + 1u);                                      // even: complete
	}

	std::string PathFor(const std::string& name)
	{
		std::string path = name.empty() ? "screenshot" : name;
		if (path.find('/') == std::string::npos && path.find('\\') == std::string::npos)
			path = "SaveData/screenshots/" + path;
		const size_t dot = path.find_last_of('.');
		const size_t slash = path.find_last_of("/\\");
		if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
			path += ".png";
		return path;
	}

	void AppendToVector(void* context, void* data, int size)
	{
		auto* bytes = static_cast<std::vector<uint8_t>*>(context);
		const uint8_t* begin = static_cast<const uint8_t*>(data);
		bytes->insert(bytes->end(), begin, begin + size);
	}

	class Callback final : public bgfx::CallbackI
	{
	public:
		~Callback() override = default;

		void fatal(const char* filePath, uint16_t line, bgfx::Fatal::Enum code, const char* message) override
		{
			Logger::Error("bgfx fatal %d at %s:%u: %s", static_cast<int>(code), filePath, static_cast<unsigned>(line), message);
			if (code != bgfx::Fatal::DebugCheck)
				std::abort();
		}

		void traceVargs(const char* filePath, uint16_t line, const char* format, va_list args) override
		{
			char text[2048];
			std::vsnprintf(text, sizeof(text), format, args);
			std::fprintf(stderr, "%s (%u): %s", filePath, static_cast<unsigned>(line), text);
		}

		void profilerBegin(const char*, uint32_t, const char*, uint16_t) override {}
		void profilerBeginLiteral(const char*, uint32_t, const char*, uint16_t) override {}
		void profilerEnd() override {}

		uint32_t cacheReadSize(uint64_t) override { return 0; }
		bool cacheRead(uint64_t, void*, uint32_t) override { return false; }
		void cacheWrite(uint64_t, const void*, uint32_t) override {}

		void screenShot(const char* filePath, uint32_t width, uint32_t height, uint32_t pitch, bgfx::TextureFormat::Enum format, const void* data, uint32_t size, bool yflip) override
		{
			if (filePath && std::string(filePath) == kShareMarker)
			{
				shareInFlight = false;
				if (shareDownscale > 0 && (format == bgfx::TextureFormat::BGRA8 || format == bgfx::TextureFormat::RGBA8))
					ShareWrite(static_cast<const uint8_t*>(data), width, height, pitch, format == bgfx::TextureFormat::BGRA8, yflip);
				return;
			}
			if (format != bgfx::TextureFormat::BGRA8 && format != bgfx::TextureFormat::RGBA8)
			{
				Logger::Error("screenshot: the back buffer format %d cannot be saved", static_cast<int>(format));
				return;
			}
			// bgfx hands over BGRA (or RGBA) rows of `pitch` bytes; PNG wants tightly packed RGBA, top row first.
			std::vector<uint8_t> pixels(static_cast<size_t>(width) * height * 4);
			const uint8_t* source = static_cast<const uint8_t*>(data);
			for (uint32_t y = 0; y < height; ++y)
			{
				const uint8_t* row = source + static_cast<size_t>(yflip ? height - 1 - y : y) * pitch;
				uint8_t* out = pixels.data() + static_cast<size_t>(y) * width * 4;
				for (uint32_t x = 0; x < width; ++x)
				{
					const bool bgra = format == bgfx::TextureFormat::BGRA8;
					out[x * 4 + 0] = row[x * 4 + (bgra ? 2 : 0)];
					out[x * 4 + 1] = row[x * 4 + 1];
					out[x * 4 + 2] = row[x * 4 + (bgra ? 0 : 2)];
					out[x * 4 + 3] = 255;
				}
			}
			std::vector<uint8_t> png;
			const std::string name = filePath;
			if (name.size() > 4 && name.compare(name.size() - 4, 4, ".bmp") == 0)
			{
				// Fast path for live previews: box-filter down by an integer factor, drop alpha, no compression.
				const uint32_t d = static_cast<uint32_t>(std::max(1, activeDownscale));
				const uint32_t w = std::max(1u, width / d);
				const uint32_t h = std::max(1u, height / d);
				std::vector<uint8_t> rgb(static_cast<size_t>(w) * h * 3);
				for (uint32_t y = 0; y < h; ++y)
					for (uint32_t x = 0; x < w; ++x)
					{
						uint32_t sum[3] = { 0, 0, 0 };
						for (uint32_t dy = 0; dy < d; ++dy)
							for (uint32_t dx = 0; dx < d; ++dx)
							{
								const uint8_t* px = pixels.data() + (static_cast<size_t>(y * d + dy) * width + (x * d + dx)) * 4;
								sum[0] += px[0]; sum[1] += px[1]; sum[2] += px[2];
							}
						uint8_t* out = rgb.data() + (static_cast<size_t>(y) * w + x) * 3;
						for (int c = 0; c < 3; ++c)
							out[c] = static_cast<uint8_t>(sum[c] / (d * d));
					}
				if (!stbi_write_bmp_to_func(AppendToVector, &png, static_cast<int>(w), static_cast<int>(h), 3, rgb.data()))
				{
					Logger::Error("screenshot: could not encode %s", filePath);
					return;
				}
				if (!FileSystemEngine::WriteFileBinary(filePath, png))
					Logger::Error("screenshot: could not write %s", filePath);
				(void)size;
				return;
			}
			if (!stbi_write_png_to_func(AppendToVector, &png, static_cast<int>(width), static_cast<int>(height), 4, pixels.data(), static_cast<int>(width * 4)))
			{
				Logger::Error("screenshot: could not encode %s", filePath);
				return;
			}
			if (FileSystemEngine::WriteFileBinary(filePath, png))
				Logger::Log("screenshot saved: %s (%ux%u)", filePath, width, height);
			else
				Logger::Error("screenshot: could not write %s", filePath);
			(void)size;
		}

		void captureBegin(uint32_t, uint32_t, uint32_t, bgfx::TextureFormat::Enum, bool) override {}
		void captureEnd() override {}
		void captureFrame(const void*, uint32_t) override {}
	};

	Callback callback;
}

namespace Screenshot
{
	bgfx::CallbackI* Callback() { return &callback; }

	void Request(const std::string& name, int downscale)
	{
		std::lock_guard<std::mutex> lock(requestMutex);
		pendingPath = PathFor(name);
		pendingDownscale = std::max(1, downscale);
	}

	void RequestTarget(bgfx::TextureHandle source, uint16_t width, uint16_t height, const std::string& name)
	{
		std::lock_guard<std::mutex> lock(requestMutex);
		pendingTargetPath = PathFor(name);
		pendingSource = source;
		pendingWidth = width;
		pendingHeight = height;
	}

	void AfterPresent(uint32_t frameNumber)
	{
		if (!bgfx::isValid(readback) || frameNumber < readbackReadyFrame)
			return;
		if (bgfx::getCaps()->originBottomLeft)
		{
			// OpenGL reads texture rows bottom-up; the PNG (and every other backend) is top row first.
			const size_t rowBytes = static_cast<size_t>(readbackWidth) * 4;
			for (uint32_t y = 0; y < readbackHeight / 2u; ++y)
				std::swap_ranges(readbackPixels.begin() + y * rowBytes, readbackPixels.begin() + (y + 1) * rowBytes, readbackPixels.begin() + (readbackHeight - 1 - y) * rowBytes);
		}
		std::vector<uint8_t> png;
		if (!stbi_write_png_to_func(AppendToVector, &png, readbackWidth, readbackHeight, 4, readbackPixels.data(), readbackWidth * 4))
			Logger::Error("screenshot: could not encode %s", readbackPath.c_str());
		else if (FileSystemEngine::WriteFileBinary(readbackPath, png))
			Logger::Log("screenshot saved: %s (%ux%u)", readbackPath.c_str(), static_cast<unsigned>(readbackWidth), static_cast<unsigned>(readbackHeight));
		else
			Logger::Error("screenshot: could not write %s", readbackPath.c_str());
		bgfx::destroy(readback);
		readback = BGFX_INVALID_HANDLE;
		readbackPixels.clear();
	}

	void BeforePresent(double gameSeconds, const std::map<std::string, std::vector<std::string>>& arguments)
	{
		if (!launchShotTaken)
		{
			const auto found = arguments.find("screenshot");
			if (found != arguments.end() && !found->second.empty() && gameSeconds >= std::atof(found->second[0].c_str()))
			{
				launchShotTaken = true;
				const auto file = arguments.find("screenshot_file");
				Request(file != arguments.end() && !file->second.empty() ? file->second[0] : "screenshot");
			}
		}

		if (shareDownscale > 0)
		{
			if (!shareInFlight.exchange(true))
				bgfx::requestScreenShot(BGFX_INVALID_HANDLE, kShareMarker);
		}
		else if (shareFile)
			ShareClose();

		std::string path;
		{
			std::lock_guard<std::mutex> lock(requestMutex);
			path.swap(pendingPath);
		}
		if (!path.empty())
		{
			activeDownscale = pendingDownscale;
			bgfx::requestScreenShot(BGFX_INVALID_HANDLE, path.c_str());
		}

		std::string targetPath;
		bgfx::TextureHandle source = BGFX_INVALID_HANDLE;
		uint16_t width = 0;
		uint16_t height = 0;
		{
			std::lock_guard<std::mutex> lock(requestMutex);
			if (bgfx::isValid(readback))
				return;   // one capture at a time; the request stays pending
			targetPath.swap(pendingTargetPath);
			source = pendingSource;
			width = pendingWidth;
			height = pendingHeight;
			pendingSource = BGFX_INVALID_HANDLE;
		}
		if (targetPath.empty() || !bgfx::isValid(source))
			return;
		// bgfx cannot screenshot an off-screen framebuffer, so blit its colour texture into a CPU-readable one and read that back.
		readback = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK);
		readbackPath = targetPath;
		readbackWidth = width;
		readbackHeight = height;
		readbackPixels.assign(static_cast<size_t>(width) * height * 4, 0);
		bgfx::TextureRegion dstRegion;
		dstRegion.init(readback, 0, 0, width, height);
		bgfx::TextureRegion srcRegion;
		srcRegion.init(source, 0, 0, width, height);
		bgfx::blit(kBlitView, dstRegion, srcRegion);
		readbackReadyFrame = bgfx::read(dstRegion, readbackPixels.data());
	}
}

CONSOLE_FUNC("screenshot", "Save the next frame as a PNG: screenshot [name] [downscale] (SaveData/screenshots/name.png; name.bmp is a fast uncompressed copy)")
{
	Screenshot::Request(args.empty() ? "screenshot" : args[0], args.size() > 1 ? std::max(1, std::atoi(args[1].c_str())) : 1);
}

CONSOLE_FUNC("frame_share", "frame_share <downscale|off> - publishes every presented frame in SaveData/ui_frame.bin for a tool that maps the file (UI Studio's live preview)")
{
	if (args.empty() || args[0] == "off" || args[0] == "0")
	{
		shareDownscale = 0;
		Logger::Log("frame_share: off");
		return;
	}
	shareDownscale = std::max(1, std::atoi(args[0].c_str()));
	Logger::Log("frame_share: on, downscale %d", shareDownscale.load());
}
