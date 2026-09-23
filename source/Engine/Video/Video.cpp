#include "Video.h"
#include <iostream>
#include <mutex>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <thread>
#include <algorithm>

#define PL_MPEG_IMPLEMENTATION
extern "C" {
#include "pl_mpeg.h"
}

extern "C" {
#include <includedLibraries/h264bsd/h264bsd_decoder.h>
}

// ---------------------------------------------------------------------------
// Shared helpers
// ---------------------------------------------------------------------------

// Copy an I420 image (with arbitrary source strides) into a FrameGrab, tightly packed.
static void StoreI420(Video::FrameGrab& g,
    const uint8_t* y, int ys,
    const uint8_t* cb, const uint8_t* cr, int cs,
    int w, int h)
{
#ifndef VIDEO_NO_THREADING
    std::lock_guard<std::mutex> lk(g.mtx);
#endif
    const int cw = (w + 1) / 2;
    const int ch = (h + 1) / 2;

    g.width = w;
    g.height = h;
    g.y_stride = w;
    g.c_stride = cw;
    g.y_plane.resize((size_t)w * h);
    g.cb_plane.resize((size_t)cw * ch);
    g.cr_plane.resize((size_t)cw * ch);

    for (int r = 0; r < h; ++r)
        std::memcpy(g.y_plane.data() + (size_t)r * w, y + (size_t)r * ys, w);
    for (int r = 0; r < ch; ++r) {
        std::memcpy(g.cb_plane.data() + (size_t)r * cw, cb + (size_t)r * cs, cw);
        std::memcpy(g.cr_plane.data() + (size_t)r * cw, cr + (size_t)r * cs, cw);
    }
    g.have_frame.store(true, std::memory_order_release);
}

bool Video::GrabToRgb(const FrameGrab& g, std::vector<uint8_t>& out_rgb, int& out_w, int& out_h) {
    int w = g.width, h = g.height;
    if (w <= 0 || h <= 0) return false;
    out_w = w; out_h = h;
    out_rgb.resize((size_t)w * h * 3);

    const uint8_t* yptr = g.y_plane.data();
    const uint8_t* cbptr = g.cb_plane.data();
    const uint8_t* crptr = g.cr_plane.data();
    int ys = g.y_stride, cs = g.c_stride;

    uint8_t* dst = out_rgb.data();
    for (int r = 0; r < h; ++r) {
        const uint8_t* yrow = yptr + (size_t)r * ys;
        const uint8_t* cbrow = cbptr + (size_t)(r / 2) * cs;
        const uint8_t* crrow = crptr + (size_t)(r / 2) * cs;
        for (int c = 0; c < w; ++c) {
            uint8_t R, G, B;
            yuv_to_rgb_pixel(yrow[c], cbrow[c / 2], crrow[c / 2], R, G, B);
            *dst++ = R; *dst++ = G; *dst++ = B;
        }
    }
    return true;
}

// pl_mpeg callback
static void plm_video_cb(plm_t* plm, plm_frame_t* frame, void* user) {
    if (!user || !frame) return;
    Video::FrameGrab* grab = (Video::FrameGrab*)user;

    int w = frame->width ? frame->width : plm_get_width(plm);
    int h = frame->height ? frame->height : plm_get_height(plm);

    // pl_mpeg planes are padded to macroblock multiples, so use their real strides
    StoreI420(*grab,
        frame->y.data, (int)frame->y.width,
        frame->cb.data, frame->cr.data, (int)frame->cb.width,
        w, h);
}

// ---------------------------------------------------------------------------
// Minimal MP4 (ISO BMFF) demuxer
// ---------------------------------------------------------------------------
namespace {

constexpr uint32_t FOURCC(char a, char b, char c, char d) {
    return ((uint32_t)(uint8_t)a << 24) | ((uint32_t)(uint8_t)b << 16) |
        ((uint32_t)(uint8_t)c << 8) | (uint32_t)(uint8_t)d;
}

inline uint32_t rd16(const uint8_t* p) { return ((uint32_t)p[0] << 8) | p[1]; }
inline uint32_t rd32(const uint8_t* p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}
inline uint64_t rd64(const uint8_t* p) { return ((uint64_t)rd32(p) << 32) | rd32(p + 4); }

struct Box {
    uint32_t type = 0;
    size_t payload = 0;   // first byte after the box header
    size_t end = 0;       // one past last byte of the box
    size_t size() const { return end - payload; }
};

// Reads the box at `pos` (must be < end) and advances pos past it.
bool NextBox(const uint8_t* d, size_t& pos, size_t end, Box& b) {
    if (pos + 8 > end) return false;
    uint64_t size = rd32(d + pos);
    b.type = rd32(d + pos + 4);
    size_t hdr = 8;
    if (size == 1) {
        if (pos + 16 > end) return false;
        size = rd64(d + pos + 8);
        hdr = 16;
    }
    else if (size == 0) {
        size = end - pos;
    }
    if (size < hdr) return false;
    if (pos + size > end) size = end - pos;   // be lenient with truncated files
    b.payload = pos + hdr;
    b.end = pos + (size_t)size;
    pos = b.end;
    return true;
}

// Parses avc1 sample entry -> avcC
bool ParseAvcC(const uint8_t* d, const Box& entry, Video::Mp4Info& m) {
    // VisualSampleEntry has 78 bytes of fixed fields before child boxes
    size_t p = entry.payload + 78;
    Box c;
    while (NextBox(d, p, entry.end, c)) {
        if (c.type != FOURCC('a', 'v', 'c', 'C')) continue;
        if (c.size() < 7) return false;
        const uint8_t* a = d + c.payload;
        const uint8_t* aend = d + c.end;
        m.nal_length_size = (a[4] & 3) + 1;
        static const uint8_t sc[4] = { 0, 0, 0, 1 };
        const uint8_t* q = a + 5;
        int nsps = q[0] & 0x1F; ++q;
        for (int i = 0; i < nsps; ++i) {
            if (q + 2 > aend) return false;
            size_t len = rd16(q); q += 2;
            if (q + len > aend) return false;
            m.headers_annexb.insert(m.headers_annexb.end(), sc, sc + 4);
            m.headers_annexb.insert(m.headers_annexb.end(), q, q + len);
            q += len;
        }
        if (q >= aend) return false;
        int npps = *q++;
        for (int i = 0; i < npps; ++i) {
            if (q + 2 > aend) return false;
            size_t len = rd16(q); q += 2;
            if (q + len > aend) return false;
            m.headers_annexb.insert(m.headers_annexb.end(), sc, sc + 4);
            m.headers_annexb.insert(m.headers_annexb.end(), q, q + len);
            q += len;
        }
        return !m.headers_annexb.empty();
    }
    return false;
}

// Parses an mp4a sample entry: channel count, sample rate, esds (object type + AudioSpecificConfig)
bool ParseMp4a(const uint8_t* d, const Box& entry, Video::Mp4Info& m) {
    if (entry.size() < 28) return false;
    const uint8_t* a = d + entry.payload;
    m.channels = (int)rd16(a + 16);
    m.sample_rate = (int)rd16(a + 24);   // integer part of the 16.16 fixed-point value

    size_t p = entry.payload + 28;
    Box c;
    while (NextBox(d, p, entry.end, c)) {
        if (c.type != FOURCC('e', 's', 'd', 's') || c.size() < 4) continue;
        const uint8_t* q = d + c.payload + 4;   // skip version/flags
        const uint8_t* qend = d + c.end;
        auto readLen = [&](const uint8_t*& r) -> size_t {
            size_t l = 0;
            for (int i = 0; i < 4 && r < qend; ++i) {
                uint8_t b = *r++;
                l = (l << 7) | (b & 0x7F);
                if (!(b & 0x80)) break;
            }
            return l;
        };
        while (q < qend) {
            uint8_t tag = *q++;
            size_t len = readLen(q);
            if (tag == 0x03) {            // ES_Descriptor: descend after its header
                q += 2;
                if (q >= qend) break;
                uint8_t fl = *q++;
                if (fl & 0x80) q += 2;
                if (fl & 0x40) { if (q < qend) q += 1 + *q; }
                if (fl & 0x20) q += 2;
            }
            else if (tag == 0x04) {       // DecoderConfigDescriptor: descend after its 13 fixed bytes
                if (q + 13 > qend) return false;
                m.object_type = q[0];
                q += 13;
            }
            else if (tag == 0x05) {       // DecoderSpecificInfo
                if (q + len > qend) return false;
                m.asc.assign(q, q + len);
                q += len;
            }
            else {
                q += len;
            }
        }
        return m.object_type != 0;
    }
    return false;
}

// Returns true if this trak is the wanted kind (H.264 video, or audio if `audio`) and fills `m`.
bool ParseTrak(const uint8_t* d, size_t begin, size_t end, Video::Mp4Info& m, bool audio) {
    bool isWanted = false, haveCodec = false;
    std::vector<std::pair<uint32_t, uint32_t>> stts;   // count, delta
    std::vector<std::pair<uint32_t, uint32_t>> stsc;   // first_chunk, samples_per_chunk
    std::vector<uint64_t> chunkOff;
    bool haveStss = false;

    size_t p = begin;
    Box b;
    while (NextBox(d, p, end, b)) {
        if (b.type != FOURCC('m', 'd', 'i', 'a')) continue;
        size_t p2 = b.payload;
        Box c;
        while (NextBox(d, p2, b.end, c)) {
            if (c.type == FOURCC('m', 'd', 'h', 'd')) {
                if (c.size() < 24) return false;
                m.timescale = rd32(d + c.payload + (d[c.payload] == 1 ? 20 : 12));
            }
            else if (c.type == FOURCC('h', 'd', 'l', 'r')) {
                if (c.size() < 12) return false;
                isWanted = rd32(d + c.payload + 8) == (audio ? FOURCC('s', 'o', 'u', 'n') : FOURCC('v', 'i', 'd', 'e'));
            }
            else if (c.type == FOURCC('m', 'i', 'n', 'f')) {
                if (!isWanted) return false;
                size_t p3 = c.payload;
                Box s;
                while (NextBox(d, p3, c.end, s)) {
                    if (s.type != FOURCC('s', 't', 'b', 'l')) continue;
                    size_t p4 = s.payload;
                    Box e;
                    while (NextBox(d, p4, s.end, e)) {
                        const uint8_t* q = d + e.payload;
                        if (e.type == FOURCC('s', 't', 's', 'd')) {
                            if (e.size() < 16) return false;
                            size_t pe = e.payload + 8;    // skip version/flags + entry_count
                            Box ent;
                            if (NextBox(d, pe, e.end, ent)) {
                                if (!audio && ent.type == FOURCC('a', 'v', 'c', '1'))
                                    haveCodec = ParseAvcC(d, ent, m);
                                else if (audio && ent.type == FOURCC('m', 'p', '4', 'a'))
                                    haveCodec = ParseMp4a(d, ent, m);
                                else if (audio && (ent.type == FOURCC('.', 'm', 'p', '3') ||
                                    ent.type == FOURCC('m', 'p', '3', ' '))) {
                                    // QuickTime-style MPEG audio entry: no esds, so fill in what we know
                                    if (ent.size() >= 28) {
                                        m.channels = (int)rd16(d + ent.payload + 16);
                                        m.sample_rate = (int)rd16(d + ent.payload + 24);
                                        m.object_type = 0x6B;
                                        haveCodec = true;
                                    }
                                }
                            }
                        }
                        else if (e.type == FOURCC('s', 't', 't', 's')) {
                            if (e.size() < 8) return false;
                            uint32_t n = rd32(q + 4);
                            if (e.size() < 8 + (uint64_t)n * 8) return false;
                            for (uint32_t i = 0; i < n; ++i)
                                stts.push_back({ rd32(q + 8 + i * 8), rd32(q + 12 + i * 8) });
                        }
                        else if (e.type == FOURCC('s', 't', 's', 'c')) {
                            if (e.size() < 8) return false;
                            uint32_t n = rd32(q + 4);
                            if (e.size() < 8 + (uint64_t)n * 12) return false;
                            for (uint32_t i = 0; i < n; ++i)
                                stsc.push_back({ rd32(q + 8 + i * 12), rd32(q + 12 + i * 12) });
                        }
                        else if (e.type == FOURCC('s', 't', 's', 'z')) {
                            if (e.size() < 12) return false;
                            uint32_t fixed = rd32(q + 4), n = rd32(q + 8);
                            if (fixed == 0 && e.size() < 12 + (uint64_t)n * 4) return false;
                            m.sample_size.resize(n);
                            for (uint32_t i = 0; i < n; ++i)
                                m.sample_size[i] = fixed ? fixed : rd32(q + 12 + i * 4);
                        }
                        else if (e.type == FOURCC('s', 't', 'c', 'o')) {
                            if (e.size() < 8) return false;
                            uint32_t n = rd32(q + 4);
                            if (e.size() < 8 + (uint64_t)n * 4) return false;
                            for (uint32_t i = 0; i < n; ++i) chunkOff.push_back(rd32(q + 8 + i * 4));
                        }
                        else if (e.type == FOURCC('c', 'o', '6', '4')) {
                            if (e.size() < 8) return false;
                            uint32_t n = rd32(q + 4);
                            if (e.size() < 8 + (uint64_t)n * 8) return false;
                            for (uint32_t i = 0; i < n; ++i) chunkOff.push_back(rd64(q + 8 + i * 8));
                        }
                        else if (e.type == FOURCC('s', 't', 's', 's')) {
                            if (e.size() < 8) return false;
                            uint32_t n = rd32(q + 4);
                            if (e.size() < 8 + (uint64_t)n * 4) return false;
                            haveStss = true;
                            for (uint32_t i = 0; i < n; ++i) {
                                uint32_t s1 = rd32(q + 8 + i * 4);
                                if (s1 > 0) m.sync_samples.push_back(s1 - 1);
                            }
                        }
                    }
                }
            }
        }
    }
    (void)haveStss;
    if (!isWanted || !haveCodec || m.timescale == 0 || m.sample_size.empty() ||
        stsc.empty() || chunkOff.empty() || stts.empty())
        return false;

    // Sample offsets from chunk table
    const size_t nSamples = m.sample_size.size();
    m.sample_offset.resize(nSamples);
    size_t s = 0, si = 0;
    for (size_t ci = 0; ci < chunkOff.size() && s < nSamples; ++ci) {
        while (si + 1 < stsc.size() && stsc[si + 1].first <= ci + 1) ++si;
        uint64_t off = chunkOff[ci];
        for (uint32_t k = 0; k < stsc[si].second && s < nSamples; ++k, ++s) {
            m.sample_offset[s] = off;
            off += m.sample_size[s];
        }
    }
    if (s < nSamples) return false;

    // Decode timestamps
    m.sample_time.resize(nSamples);
    uint64_t t = 0;
    s = 0;
    for (auto& e : stts)
        for (uint32_t k = 0; k < e.first && s < nSamples; ++k, ++s) { m.sample_time[s] = t; t += e.second; }
    for (; s < nSamples; ++s) m.sample_time[s] = t;   // pad if stts was short

    std::sort(m.sync_samples.begin(), m.sync_samples.end());
    return true;
}

} // namespace

static bool LooksLikeMp4(const uint8_t* b, size_t n);

bool Video::ParseMp4() const {
    const uint8_t* d = _data.data();
    size_t pos = 0, n = _data.size();
    Box b;
    while (NextBox(d, pos, n, b)) {
        if (b.type != FOURCC('m', 'o', 'o', 'v')) continue;
        size_t p = b.payload;
        Box t;
        while (NextBox(d, p, b.end, t)) {
            if (t.type != FOURCC('t', 'r', 'a', 'k')) continue;
            if (!_mp4) {
                auto info = std::make_unique<Mp4Info>();
                if (ParseTrak(d, t.payload, t.end, *info, false)) { _mp4 = std::move(info); continue; }
            }
            if (!_mp4_audio) {
                auto info = std::make_unique<Mp4Info>();
                if (ParseTrak(d, t.payload, t.end, *info, true)) _mp4_audio = std::move(info);
            }
        }
    }
    return _mp4 != nullptr;
}

// ---------------------------------------------------------------------------
// H.264 decoding via h264bsd
// ---------------------------------------------------------------------------
Video::Mp4Decoder::Mp4Decoder(const std::vector<uint8_t>& data, const Mp4Info& info)
    : _data(data), _m(info) {}

Video::Mp4Decoder::~Mp4Decoder() {
    if (_h264) {
        h264bsdShutdown((storage_t*)_h264);
        h264bsdFree((storage_t*)_h264);
    }
}

double Video::Mp4Decoder::Duration() const {
    size_t n = _m.sample_time.size();
    if (n == 0 || _m.timescale == 0) return 0.0;
    uint64_t last = _m.sample_time[n - 1];
    uint64_t delta = n > 1 ? last - _m.sample_time[n - 2] : 0;
    return (double)(last + delta) / _m.timescale;
}

bool Video::Mp4Decoder::ToRgb(std::vector<uint8_t>& out_rgb, int& out_w, int& out_h) const {
    if (!HasFrame()) return false;
    return GrabToRgb(_grab, out_rgb, out_w, out_h);
}

void Video::Mp4Decoder::Reset() {
    if (_h264) {
        h264bsdShutdown((storage_t*)_h264);
        h264bsdFree((storage_t*)_h264);
        _h264 = nullptr;
    }
    storage_t* dec = h264bsdAlloc();
    if (dec && h264bsdInit(dec, 1 /* noOutputReordering */) == 0) _h264 = dec;
    else {
        if (dec) h264bsdFree(dec);
        std::cerr << "Mp4Decoder: h264bsd init failed\n";
    }
    _cur = -1;
}

// Feeds one MP4 sample (length-prefixed NALs) to the decoder as Annex B.
// Updates _grab whenever a picture comes out. Returns false on I/O problems.
bool Video::Mp4Decoder::DecodeSample(size_t index, bool prepend_headers) {
    uint64_t off = _m.sample_offset[index];
    uint64_t size = _m.sample_size[index];
    if (off + size > _data.size()) return false;

    static const uint8_t sc[4] = { 0, 0, 0, 1 };
    std::vector<uint8_t> buf;
    buf.reserve((size_t)size + _m.headers_annexb.size() + 16);
    if (prepend_headers)
        buf.insert(buf.end(), _m.headers_annexb.begin(), _m.headers_annexb.end());

    const uint8_t* p = _data.data() + off;
    const uint8_t* end = p + size;
    const int L = _m.nal_length_size;
    while (p + L <= end) {
        uint32_t len = 0;
        for (int i = 0; i < L; ++i) len = (len << 8) | p[i];
        p += L;
        if (len == 0 || p + len > end) break;
        buf.insert(buf.end(), sc, sc + 4);
        buf.insert(buf.end(), p, p + len);
        p += len;
    }

    storage_t* dec = (storage_t*)_h264;
    size_t pos = 0;
    int stalls = 0;
    while (pos < buf.size()) {
        u32 read = 0;
        u32 res = h264bsdDecode(dec, buf.data() + pos, (u32)(buf.size() - pos), 0, &read);

        if (res == H264BSD_PIC_RDY) {
            u32 picId = 0, isIdr = 0, numErr = 0;
            while (u8* pic = h264bsdNextOutputPicture(dec, &picId, &isIdr, &numErr)) {
                u32 W = h264bsdPicWidth(dec) * 16;
                u32 H = h264bsdPicHeight(dec) * 16;
                u32 cropFlag = 0, cl = 0, cw = 0, ct = 0, chh = 0;
                h264bsdCroppingParams(dec, &cropFlag, &cl, &cw, &ct, &chh);
                if (!cropFlag) { cl = 0; ct = 0; cw = W; chh = H; }

                const uint8_t* Y = pic;
                const uint8_t* Cb = Y + (size_t)W * H;
                const uint8_t* Cr = Cb + (size_t)(W / 2) * (H / 2);
                StoreI420(_grab,
                    Y + (size_t)ct * W + cl, (int)W,
                    Cb + (size_t)(ct / 2) * (W / 2) + cl / 2,
                    Cr + (size_t)(ct / 2) * (W / 2) + cl / 2, (int)(W / 2),
                    (int)cw, (int)chh);
            }
        }
        // HDRS_RDY (new SPS/PPS activated) returns with read == 0: the decoder has set up its
        // buffers and the same NAL must be passed in again. Every other result must consume input.
        if (read == 0) {
            if (res == H264BSD_HDRS_RDY && ++stalls < 3) continue;
            break;
        }
        stalls = 0;
        pos += read;
    }
    return true;
}

bool Video::Mp4Decoder::DecodeToTime(double seconds, double timeout_seconds) {
    if (seconds < 0.0) seconds = 0.0;
    const size_t n = _m.sample_size.size();
    if (n == 0) return false;

    // Last sample whose timestamp is <= requested time
    const uint64_t t = (uint64_t)(seconds * _m.timescale);
    size_t target = (size_t)(std::upper_bound(_m.sample_time.begin(), _m.sample_time.end(), t)
        - _m.sample_time.begin());
    target = target ? target - 1 : 0;
    if (target >= n) target = n - 1;

    // Nearest sync sample at or before target
    size_t sync = target;   // no stss => every frame is intra
    if (!_m.sync_samples.empty()) {
        auto it = std::upper_bound(_m.sync_samples.begin(), _m.sync_samples.end(), (uint32_t)target);
        sync = (it == _m.sync_samples.begin()) ? 0 : *(it - 1);
    }

    // Continue from the current decoder state if we're moving forward inside the same GOP
    bool cont = _h264 && _cur >= 0 && (int64_t)target >= _cur && (int64_t)sync <= _cur;
    size_t start;
    if (cont) {
        start = (size_t)_cur + 1;
    }
    else {
        Reset();
        _grab.have_frame.store(false, std::memory_order_release);
        start = sync;
    }
    if (!_h264) return false;

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::duration<double>(timeout_seconds);
    for (size_t i = start; i <= target; ++i) {
        if (!DecodeSample(i, /*prepend_headers=*/ _cur < 0 && i == start)) return false;
        _cur = (int64_t)i;
        if (std::chrono::steady_clock::now() >= deadline) return false;
    }
    return HasFrame();
}

std::unique_ptr<Video::Mp4Decoder> Video::CreateMp4Decoder() const {
    if (!IsMp4()) return nullptr;
    return std::unique_ptr<Mp4Decoder>(new Mp4Decoder(_data, *_mp4));
}

// ---------------------------------------------------------------------------
// Implementation
// ---------------------------------------------------------------------------
static bool LooksLikeMp4(const uint8_t* b, size_t n) {
    return n >= 12 && std::memcmp(b + 4, "ftyp", 4) == 0;
}

void Video::ProbeMp4() const {
    if (_mp4_probed) return;
    _mp4_probed = true;
    if (!LooksLikeMp4(_data.data(), _data.size())) return;
    if (ParseMp4()) {
        std::cerr << "Video: MP4/H.264 detected (" << _mp4->sample_size.size() << " samples)\n";
        if (_mp4_audio)
            std::cerr << "Video: audio track, object type 0x" << std::hex << (int)_mp4_audio->object_type
            << std::dec << ", " << _mp4_audio->sample_rate << " Hz, " << _mp4_audio->channels << " ch\n";
    }
    else
        std::cerr << "Video: MP4 container found but no supported H.264 (avc1) track\n";
}

bool Video::IsMp4() const {
    ProbeMp4();
    return _mp4 != nullptr;
}

const Video::Mp4Info* Video::GetMp4Audio() const {
    ProbeMp4();
    return _mp4_audio.get();
}

Video* Video::FromMemory(const uint8_t* bytes, size_t length) {
    Video* v = new Video();
    v->_data.assign(bytes, bytes + length);
    if (LooksLikeMp4(bytes, length) && !v->IsMp4()) {
        delete v;
        return nullptr;
    }
    return v;
}

Video* Video::FromVector(const std::vector<uint8_t>& vec) {
    return FromMemory(vec.data(), vec.size());
}

Video::~Video() {
#ifndef VIDEO_NO_THREADING
    stop_worker();
#endif
}

bool Video::GetFrameAtTime(double seconds, std::vector<uint8_t>& out_rgb,
    int& out_w, int& out_h, double timeout_seconds) {
    out_rgb.clear();
    out_w = out_h = 0;
    if (_data.empty()) return false;
    if (seconds < 0.0) seconds = 0.0;

    if (IsMp4()) {
        if (!_mp4_dec) _mp4_dec = CreateMp4Decoder();
        return _mp4_dec && _mp4_dec->DecodeToTime(seconds, timeout_seconds) &&
            _mp4_dec->ToRgb(out_rgb, out_w, out_h);
    }

#ifndef VIDEO_NO_THREADING
    ensure_worker_running();

    {
        std::unique_lock<std::mutex> lk(_ctl_mutex);
        _target_time = seconds;
        _frame_requested = true;
        _ctl_cv.notify_one();
    }

    auto deadline = std::chrono::steady_clock::now() + std::chrono::duration<double>(timeout_seconds);
    std::unique_lock<std::mutex> lk(_frame_cv_mutex);
    while (!_latest_grab.have_frame.load(std::memory_order_acquire)) {
        if (std::chrono::steady_clock::now() >= deadline) return false;
        _frame_cv.wait_until(lk, deadline);
    }

    bool ok;
    {
        std::lock_guard<std::mutex> glk(_latest_grab.mtx);
        ok = GrabToRgb(_latest_grab, out_rgb, out_w, out_h);
    }
    _latest_grab.have_frame.store(false, std::memory_order_release);
    return ok;

#else
    // Single-threaded: create plm_t each call
    plm_t* plm = plm_create_with_memory((uint8_t*)_data.data(), _data.size(), 0);
    if (!plm) return false;

    FrameGrab grab;
    plm_set_video_decode_callback(plm, plm_video_cb, &grab);

    const double dt_step = 1.0 / 60.0;
    double acc_time = 0.0;
    const auto tstart = std::chrono::steady_clock::now();
    while (acc_time < seconds + 1e-6) {
        plm_decode(plm, dt_step);
        acc_time += dt_step;
        if (std::chrono::duration<double>(std::chrono::steady_clock::now() - tstart).count() > timeout_seconds) {
            plm_destroy(plm);
            return false;
        }
    }

    bool ok = grab.have_frame.load(std::memory_order_acquire) &&
        GrabToRgb(grab, out_rgb, out_w, out_h);
    plm_destroy(plm);
    return ok;
#endif
}

#ifndef VIDEO_NO_THREADING
void Video::ensure_worker_running() {
    if (_worker_running.load(std::memory_order_acquire)) return;
    std::lock_guard<std::mutex> lk(_ctl_mutex);
    if (_worker_running.load(std::memory_order_relaxed)) return;
    _worker_stop.store(false, std::memory_order_release);
    _worker = std::thread(&Video::worker_thread_func, this);
    _worker_running.store(true, std::memory_order_release);
}

void Video::stop_worker() {
    if (!_worker_running.load(std::memory_order_acquire)) return;
    {
        std::lock_guard<std::mutex> lk(_ctl_mutex);
        _worker_stop.store(true, std::memory_order_release);
        _ctl_cv.notify_one();
    }
    if (_worker.joinable()) _worker.join();
    _worker_running.store(false, std::memory_order_release);
}

void Video::worker_thread_func() {
    plm_t* plm = plm_create_with_memory((uint8_t*)_data.data(), _data.size(), 0);
    if (!plm) { std::cerr << "worker: plm_create_with_memory failed\n"; return; }
    plm_set_video_decode_callback(plm, plm_video_cb, &_latest_grab);

    double local_time = 0.0;
    const double dt_step = 1.0 / 60.0;

    while (true) {
        double req_time = 0.0;
        {
            std::unique_lock<std::mutex> lk(_ctl_mutex);
            _ctl_cv.wait(lk, [this] { return _frame_requested || _worker_stop.load(std::memory_order_acquire); });
            if (_worker_stop.load(std::memory_order_acquire)) break;
            req_time = _target_time;
            _frame_requested = false;
        }

        while (local_time < req_time && !_worker_stop.load(std::memory_order_acquire)) {
            plm_decode(plm, dt_step);
            local_time += dt_step;

            if (_latest_grab.have_frame.load(std::memory_order_acquire)) {
                std::lock_guard<std::mutex> lk(_frame_cv_mutex);
                _frame_cv.notify_all();
            }
        }

        std::lock_guard<std::mutex> lk(_frame_cv_mutex);
        _frame_cv.notify_all();
    }

    plm_destroy(plm);
}
#endif
