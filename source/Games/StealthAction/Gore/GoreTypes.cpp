#include "GoreTypes.h"

namespace Gore
{
    uint32_t HashSeed(const std::string& victimId, uint32_t hitCounter)
    {
        uint32_t h = 2166136261u;   // FNV-1a over the id bytes
        for (unsigned char c : victimId)
        {
            h ^= c;
            h *= 16777619u;
        }

        // Mix the counter in (murmur3 finaliser).
        h ^= hitCounter * 0x9E3779B1u;
        h ^= h >> 16;
        h *= 0x85EBCA6Bu;
        h ^= h >> 13;
        h *= 0xC2B2AE35u;
        h ^= h >> 16;
        return h ? h : 1u;
    }
}
