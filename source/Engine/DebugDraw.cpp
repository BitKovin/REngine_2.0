#include "DebugDraw.hpp"

// Definitions for static members, usually in DebugDraw.cpp:
std::mutex DebugDraw::mainLock;
std::vector<std::unique_ptr<DebugDrawCommand>> DebugDraw::commands;
std::vector<DebugDrawCommand*> DebugDraw::finalizedCommands;

void DebugDraw::IndexedMesh(vector<vec3> vertices, vector<uint32_t> indices, float duration, float thickness)
{
	uint32_t oldIndex = indices[0];

	for (auto index : indices)
	{

		Line(vertices[oldIndex], vertices[index], duration, thickness);

		oldIndex = index;
	}

}
