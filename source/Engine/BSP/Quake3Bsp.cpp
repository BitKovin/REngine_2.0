
#include "Quake3Bsp.h"

#include <algorithm> // std::sort
#include <cstring>   // GCC7 fix
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "../DebugDraw.hpp"

#include "../AssetRegistry.h"

#include "../Camera.h"

#include "../Level.hpp"

#include "../MapParser.h"

#include "../LevelObjectFactory.h"
#include "../Level.hpp"

#if __EMSCRIPTEN__

#define strcpy_s strcpy
#define strcat_s strcat

#endif

CQuake3BSP::CQuake3BSP() {
    m_numOfVerts = 0;
    m_numOfFaces = 0;
    m_numOfIndices = 0;
    m_numOfTextures = 0;
    m_numOfLightmaps = 0;
    numVisibleFaces = 0;
    skipindices = 0;

    m_pVerts = NULL;
    m_pFaces = NULL;
    m_pIndices = NULL;
}

CQuake3BSP::~CQuake3BSP() {
    delete[] m_pVerts;
    delete[] m_pFaces;
    delete[] m_pIndices;
    delete[] pTextures;
    delete[] pLightmaps;
    if (m_lightmap_gen_IDs) {
        glDeleteTextures(m_numOfLightmaps, m_lightmap_gen_IDs);
        delete[] m_lightmap_gen_IDs;
    }

    glDeleteTextures(1, &missing_LM_id);
}

bool CQuake3BSP::LoadBSP(const char* filename) {
    if (!filename) {
        printf("ERROR:: You must specify BSP file as parameter");
        return 0;
    }

    FILE* fp = NULL;
    
#if __EMSCRIPTEN__

    fp = fopen(filename, "rb");

#else

    fopen_s(&fp, filename, "rb");

#endif

    if (fp == NULL) {
        printf("ERROR:: cannot open BSP file: %s\n", filename);
        return 0;
    }

    filePath = filename;

    tBSPHeader header = { 0 };
    tBSPLump lumps[kMaxLumps] = { 0 };

    fread(&header, 1, sizeof(tBSPHeader), fp);
    fread(&lumps, kMaxLumps, sizeof(tBSPLump), fp);

    // Read Entities
    if (lumps[kEntities].length > 0) {
        fseek(fp, lumps[kEntities].offset, SEEK_SET);
        std::vector<char> entData(lumps[kEntities].length + 1);
        fread(entData.data(), 1, lumps[kEntities].length, fp);
        entData.back() = '\0';
        entities.assign(entData.data());
    }

    // Read Planes
    planes.resize(lumps[kPlanes].length / sizeof(tBSPPlane));
    fseek(fp, lumps[kPlanes].offset, SEEK_SET);
    fread(planes.data(), sizeof(tBSPPlane), planes.size(), fp);

    for (auto& plane : planes) {
        // Quake3 uses Z-up, convert to Y-up
        float oldY = plane.normal.y;
        plane.normal.y = plane.normal.z;
        plane.normal.z = -oldY;
        // Do not recalculate plane distance; keep original
    }

    // Read Nodes
    nodes.resize(lumps[kNodes].length / sizeof(tBSPNode));
    fseek(fp, lumps[kNodes].offset, SEEK_SET);
    fread(nodes.data(), sizeof(tBSPNode), nodes.size(), fp);

    // Read Leafs
    leafs.resize(lumps[kLeafs].length / sizeof(tBSPLeaf));
    fseek(fp, lumps[kLeafs].offset, SEEK_SET);
    fread(leafs.data(), sizeof(tBSPLeaf), leafs.size(), fp);

    // Read LeafFaces
    leafFaces.resize(lumps[kLeafFaces].length / sizeof(int));
    fseek(fp, lumps[kLeafFaces].offset, SEEK_SET);
    fread(leafFaces.data(), sizeof(int), leafFaces.size(), fp);

    // Read LeafBrushes
    leafBrushes.resize(lumps[kLeafBrushes].length / sizeof(int));
    fseek(fp, lumps[kLeafBrushes].offset, SEEK_SET);
    fread(leafBrushes.data(), sizeof(int), leafBrushes.size(), fp);

    // Read Models
    models.resize(lumps[kModels].length / sizeof(tBSPModel));
    fseek(fp, lumps[kModels].offset, SEEK_SET);
    fread(models.data(), sizeof(tBSPModel), models.size(), fp);

    // Store original bounds before transformation
    if (!models.empty()) {
        originalMins = glm::vec3(models[0].mins[0], models[0].mins[1], models[0].mins[2]);
        originalMaxs = glm::vec3(models[0].maxs[0], models[0].maxs[1], models[0].maxs[2]);
    }

    for (auto& model : models) {
        // Convert mins
        float temp = model.mins[1];
        model.mins[1] = model.mins[2];
        model.mins[2] = -temp;

        // Convert maxs
        temp = model.maxs[1];
        model.maxs[1] = model.maxs[2];
        model.maxs[2] = -temp;
    }

    for (auto& leaf : leafs)
    {
        float temp = leaf.mins[1];
        leaf.mins[1] = leaf.mins[2];
        leaf.mins[2] = -temp;

        // Convert maxs
        temp = leaf.maxs[1];
        leaf.maxs[1] = leaf.maxs[2];
        leaf.maxs[2] = -temp;
    }

    // Read Brushes
    brushes.resize(lumps[kBrushes].length / sizeof(tBSPBrush));
    fseek(fp, lumps[kBrushes].offset, SEEK_SET);
    fread(brushes.data(), sizeof(tBSPBrush), brushes.size(), fp);

    // Read BrushSides
    brushSides.resize(lumps[kBrushSides].length / sizeof(tBSPBrushSide));
    fseek(fp, lumps[kBrushSides].offset, SEEK_SET);
    fread(brushSides.data(), sizeof(tBSPBrushSide), brushSides.size(), fp);

    // Read Meshverts (kIndices corresponds to Meshverts)
    meshVerts.resize(lumps[kIndices].length / sizeof(tBSPMeshVert));
    fseek(fp, lumps[kIndices].offset, SEEK_SET);
    fread(meshVerts.data(), sizeof(tBSPMeshVert), meshVerts.size(), fp);

    // Read Effects (kShaders in the code's enum)
    effects.resize(lumps[kShaders].length / sizeof(tBSPEffect));
    fseek(fp, lumps[kShaders].offset, SEEK_SET);
    fread(effects.data(), sizeof(tBSPEffect), effects.size(), fp);

    // Read Lightvols
    lightVols.resize(lumps[kLightVolumes].length / sizeof(tBSPLightvol));
    fseek(fp, lumps[kLightVolumes].offset, SEEK_SET);
    fread(lightVols.data(), sizeof(tBSPLightvol), lightVols.size(), fp);

    // Read VisData
    if (lumps[kVisData].length > 0) {
        fseek(fp, lumps[kVisData].offset, SEEK_SET);
        fread(&visData.n_vecs, sizeof(int), 1, fp);
        fread(&visData.sz_vecs, sizeof(int), 1, fp);
        visData.vecs.resize(visData.n_vecs * visData.sz_vecs);
        fread(visData.vecs.data(), 1, visData.vecs.size(), fp);
    }

    // Allocate the vertex memory
    m_numOfVerts = lumps[kVertices].length / sizeof(tBSPVertex);
    m_pVerts = new tBSPVertex[m_numOfVerts];

    // Allocate the face memory
    m_numOfFaces = lumps[kFaces].length / sizeof(tBSPFace);
    m_pFaces = new tBSPFace[m_numOfFaces];

    // Allocate the index memory
    m_numOfIndices = lumps[kIndices].length / sizeof(int);
    m_pIndices = new int[m_numOfIndices];

    // Allocate memory to read in the texture information.
    m_numOfTextures = lumps[kTextures].length / sizeof(tBSPTexture);

    // Seek to the position in the file that stores the vertex information
    fseek(fp, lumps[kVertices].offset, SEEK_SET);

    // Convert vertices to Y-up
    for (int i = 0; i < m_numOfVerts; i++) {
        fread(&m_pVerts[i], 1, sizeof(tBSPVertex), fp);
        float temp = m_pVerts[i].vPosition.y;
        m_pVerts[i].vPosition.y = m_pVerts[i].vPosition.z;
        m_pVerts[i].vPosition.z = -temp;

        temp = m_pVerts[i].vNormal.y;
        m_pVerts[i].vNormal.y = m_pVerts[i].vNormal.z;
        m_pVerts[i].vNormal.z = -temp;
    }

    pTextures = new tBSPTexture[m_numOfTextures];

    fseek(fp, lumps[kIndices].offset, SEEK_SET);
    fread(m_pIndices, m_numOfIndices, sizeof(int), fp);
    fseek(fp, lumps[kFaces].offset, SEEK_SET);
    fread(m_pFaces, m_numOfFaces, sizeof(tBSPFace), fp);
    fseek(fp, lumps[kTextures].offset, SEEK_SET);
    fread(pTextures, m_numOfTextures, sizeof(tBSPTexture), fp);

    // Create textures
    for (int i = 0; i < m_numOfTextures; i++) {
        strcpy_s(tname[i], pTextures[i].strName);
        strcat_s(tname[i], ".jpg");
        //printf("loading: %s \n", tname[i]);
    }

    m_numOfLightmaps = lumps[kLightmaps].length / sizeof(tBSPLightmap);
    pLightmaps = new tBSPLightmap[m_numOfLightmaps];
    fseek(fp, lumps[kLightmaps].offset, SEEK_SET);

    for (int i = 0; i < m_numOfLightmaps; i++) {
        fread(&pLightmaps[i], 1, sizeof(tBSPLightmap), fp);
        Rbuffers.G_lightMaps.push_back(pLightmaps[i]);
    }

    fclose(fp);
    return (fp);
}

// int skipindices = 0;
void CQuake3BSP::BuildVBO() {
    for (int index = 0; index < m_numOfFaces; index++) {
        tBSPFace* pFace = &m_pFaces[index];

        if (pFace->type == FACE_POLYGON)
            skipindices += pFace->numOfIndices;

        CreateVBO(index);
        CreateIndices(index);
        CreateRenderBuffers(index);
        // BSPDebug(index);
        numVisibleFaces++;
    }
}

void CQuake3BSP::CreateVBO(int index) {
    tBSPFace* pFace = &m_pFaces[index];
    auto& vertices = Rbuffers.v_faceVBOs[index]; // Now a vector<VertexData>

    for (int v = 0; v < pFace->numOfVerts; v++) {
        const tBSPVertex& bspVert = m_pVerts[pFace->startVertIndex + v];
        VertexData vd;

        vd.Position = bspVert.vPosition;
        vd.Normal = bspVert.vNormal;
        vd.TextureCoordinate = bspVert.vTextureCoord;
        vd.ShadowMapCoords = bspVert.vLightmapCoord; // Using ShadowMapCoords for lightmap

        // Convert color from byte[4] to vec4
        vd.Color = glm::vec4(
            (float)bspVert.color[0] / 255.0f,
            (float)bspVert.color[1] / 255.0f,
            (float)bspVert.color[2] / 255.0f,
            (float)bspVert.color[3] / 255.0f
        );

        vertices.push_back(vd);
    }
}

void CQuake3BSP::CreateIndices(int index) {
    tBSPFace* pFace = &m_pFaces[index];
    int start = pFace->startIndex;
    int count = pFace->numOfIndices;
    auto& out = Rbuffers.v_faceIDXs[index];

    // reserve once to avoid repeated reallocations
    out.reserve(count);

    // process each triangle (3 indices) and swap the last two
    for (int j = 0; j < count; j += 3) {
        unsigned short i0 = m_pIndices[start + j + 0];
        unsigned short i1 = m_pIndices[start + j + 1];
        unsigned short i2 = m_pIndices[start + j + 2];
        // Original (CCW): i0, i1, i2
        // Flipped (CW):    i0, i2, i1
        out.push_back(i0);
        out.push_back(i2);
        out.push_back(i1);
    }
}

glm::vec3 computeLightDirection(const unsigned char vol_dir[2]) {
    // Step 1: Scale angles to degrees
    float yaw_deg = static_cast<float>(vol_dir[1]) * 360.0f / 255.0f;
    float pitch_deg = 270.0f - static_cast<float>(vol_dir[0]) * 360.0f / 255.0f;

    // Step 2: Convert angles to radians
    float yaw_rad = glm::radians(yaw_deg);
    float pitch_rad = glm::radians(pitch_deg);

    // Step 3: Create rotation matrices
    // Yaw around Z-axis
    glm::mat4 yaw_matrix = glm::rotate(glm::mat4(1.0f), yaw_rad, glm::vec3(0.0f, 0.0f, 1.0f));
    // Pitch around Y-axis
    glm::mat4 pitch_matrix = glm::rotate(glm::mat4(1.0f), pitch_rad, glm::vec3(0.0f, 1.0f, 0.0f));

    // Step 4: Combine rotations (pitch after yaw)
    glm::mat4 rotation = pitch_matrix * yaw_matrix;

    // Step 5: Apply rotation to initial vector (1, 0, 0)
    glm::vec3 view_vector = glm::vec3(rotation * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));

    // Step 6: Invert to get light direction in Quake coordinates
    glm::vec3 light_dir_quake = -view_vector;

    // Step 7: Transform to engine coordinates (assuming Y-up: X, Z, -Y)
    glm::vec3 light_dir_engine = glm::vec3(light_dir_quake.x, light_dir_quake.z, light_dir_quake.y);

    return light_dir_engine;
}

LightVolPointData CQuake3BSP::GetLightvolColor(const glm::vec3& position)
{
    // Transform position from Y-up (engine) to Z-up (Quake 3)
    glm::vec3 pos_quake(position.x, -position.z, position.y);

    // Use original bounds in Z-up
    glm::vec3 modelMins = originalMins;
    glm::vec3 modelMaxs = originalMaxs;

    // Calculate grid dimensions according to Quake 3 specs (Z-up)
    glm::ivec3 lightVolGridDims(
        static_cast<int>(std::floor(modelMaxs.x / 64.0f) - std::ceil(modelMins.x / 64.0f) + 1),
        static_cast<int>(std::floor(modelMaxs.y / 64.0f) - std::ceil(modelMins.y / 64.0f) + 1),
        static_cast<int>(std::floor(modelMaxs.z / 128.0f) - std::ceil(modelMins.z / 128.0f) + 1)
    );

    // Calculate grid indices and fractional offsets
    float fx = (pos_quake.x - modelMins.x) / 64.0f;
    float fy = (pos_quake.y - modelMins.y) / 64.0f;
    float fz = (pos_quake.z - modelMins.z) / 128.0f;

    // Get integer indices for the lower corner
    int nx = static_cast<int>(std::floor(fx));
    int ny = static_cast<int>(std::floor(fy));
    int nz = static_cast<int>(std::floor(fz));

    // Get fractional components for interpolation
    fx = fx - nx; // Range [0,1]
    fy = fy - ny;
    fz = fz - nz;

    // Clamp indices to prevent out-of-bounds access
    int nx0 = glm::clamp(nx, 0, lightVolGridDims.x - 1);
    int ny0 = glm::clamp(ny, 0, lightVolGridDims.y - 1);
    int nz0 = glm::clamp(nz, 0, lightVolGridDims.z - 1);
    int nx1 = glm::clamp(nx + 1, 0, lightVolGridDims.x - 1);
    int ny1 = glm::clamp(ny + 1, 0, lightVolGridDims.y - 1);
    int nz1 = glm::clamp(nz + 1, 0, lightVolGridDims.z - 1);

    // Fetch light volumes at the eight corners
    auto getLightVolData = [&](int x, int y, int z) -> std::tuple<glm::vec3, glm::vec3, glm::vec3> {
        int index = z * (lightVolGridDims.x * lightVolGridDims.y) + y * lightVolGridDims.x + x;
        const tBSPLightvol& vol = lightVols[index];
        glm::vec3 ambient(
            static_cast<float>(vol.ambient[0]) / 255.0f,
            static_cast<float>(vol.ambient[1]) / 255.0f,
            static_cast<float>(vol.ambient[2]) / 255.0f
        );
        glm::vec3 directional(
            static_cast<float>(vol.directional[0]) / 255.0f,
            static_cast<float>(vol.directional[1]) / 255.0f,
            static_cast<float>(vol.directional[2]) / 255.0f
        );
        glm::vec3 dir_engine = computeLightDirection(vol.dir);
        return { ambient, directional, dir_engine };
        };

    auto [amb000, dir000, vec000] = getLightVolData(nx0, ny0, nz0);
    auto [amb100, dir100, vec100] = getLightVolData(nx1, ny0, nz0);
    auto [amb010, dir010, vec010] = getLightVolData(nx0, ny1, nz0);
    auto [amb110, dir110, vec110] = getLightVolData(nx1, ny1, nz0);
    auto [amb001, dir001, vec001] = getLightVolData(nx0, ny0, nz1);
    auto [amb101, dir101, vec101] = getLightVolData(nx1, ny0, nz1);
    auto [amb011, dir011, vec011] = getLightVolData(nx0, ny1, nz1);
    auto [amb111, dir111, vec111] = getLightVolData(nx1, ny1, nz1);

    // Trilinear interpolation
    auto lerp = [](float a, float b, float t) { return a + t * (b - a); };
    auto lerpVec3 = [](const glm::vec3& a, const glm::vec3& b, float t) {
        return a + t * (b - a);
        };

    // Interpolate along X for each plane
    glm::vec3 amb_x00 = lerpVec3(amb000, amb100, fx);
    glm::vec3 amb_x10 = lerpVec3(amb010, amb110, fx);
    glm::vec3 amb_x01 = lerpVec3(amb001, amb101, fx);
    glm::vec3 amb_x11 = lerpVec3(amb011, amb111, fx);

    glm::vec3 dir_x00 = lerpVec3(dir000, dir100, fx);
    glm::vec3 dir_x10 = lerpVec3(dir010, dir110, fx);
    glm::vec3 dir_x01 = lerpVec3(dir001, dir101, fx);
    glm::vec3 dir_x11 = lerpVec3(dir011, dir111, fx);

    glm::vec3 vec_x00 = lerpVec3(vec000, vec100, fx);
    glm::vec3 vec_x10 = lerpVec3(vec010, vec110, fx);
    glm::vec3 vec_x01 = lerpVec3(vec001, vec101, fx);
    glm::vec3 vec_x11 = lerpVec3(vec011, vec111, fx);

    // Interpolate along Y
    glm::vec3 amb_y0 = lerpVec3(amb_x00, amb_x10, fy);
    glm::vec3 amb_y1 = lerpVec3(amb_x01, amb_x11, fy);

    glm::vec3 dir_y0 = lerpVec3(dir_x00, dir_x10, fy);
    glm::vec3 dir_y1 = lerpVec3(dir_x01, dir_x11, fy);

    glm::vec3 vec_y0 = lerpVec3(vec_x00, vec_x10, fy);
    glm::vec3 vec_y1 = lerpVec3(vec_x01, vec_x11, fy);

    // Interpolate along Z
    glm::vec3 ambient = lerpVec3(amb_y0, amb_y1, fz);
    glm::vec3 directional = lerpVec3(dir_y0, dir_y1, fz);
    glm::vec3 dir_engine = lerpVec3(vec_y0, vec_y1, fz);

    // Normalize direction to ensure valid vector
    if (glm::length(dir_engine) > 0.0f) {
        dir_engine = glm::normalize(dir_engine);
    }
    else {
        dir_engine = glm::vec3(0.0f, 1.0f, 0.0f); // Default direction (up)
    }

    return LightVolPointData{ directional, ambient, dir_engine };
}

int CQuake3BSP::FindClusterAtPosition(const glm::vec3& cameraPos) 
{

    int nodeIndex = 0;
    int depth = 0;
    while (nodeIndex >= 0 && depth < 100) { // Add depth limit to prevent infinite loops
		const tBSPNode& node = nodes[nodeIndex];

		const vec3 min = vec3(node.mins[0], node.mins[1], node.mins[2]) / MAP_SCALE;
		const vec3 max = vec3(node.maxs[0], node.maxs[1], node.maxs[2]) / MAP_SCALE;

        const tBSPPlane& plane = planes[node.plane];
        float distance = glm::dot(cameraPos, plane.normal) - plane.dist;

        int childIndex = distance >= 0 ? 0 : 1;
        int nextChild = node.children[childIndex];
        if (nextChild < 0) {
            int leafIndex = -nextChild - 1;

            return leafs[leafIndex].cluster;
        }
        nodeIndex = nextChild;
        depth++;
    }
    printf("Failed to find cluster\n");
    return -1;
}

bool CQuake3BSP::IsClusterVisible(int sourceCluster, int testCluster)
{

    if (sourceCluster < 0) return false;

    if (sourceCluster < 0 || testCluster < 0) return true;

    int byteIndex = (sourceCluster * visData.sz_vecs) + (testCluster / 8);
    int bitIndex = testCluster % 8;

    // Convert std::byte to unsigned integer for bitwise operations
    unsigned char byteValue = (visData.vecs[byteIndex]);
    return (byteValue & (1 << bitIndex)) != 0;
}

void CQuake3BSP::DrawForward(mat4x4 view, mat4x4 projection)
{
    
    bool first = true;
    for (auto& model : models)
    {

        vec3 min = vec3(model.mins[0], model.mins[1], model.mins[2]) / MAP_SCALE;
        vec3 max = vec3(model.maxs[0], model.maxs[1], model.maxs[2]) / MAP_SCALE;

        if (Camera::frustum.IsBoxVisible(min, max))
        {
            RenderBSP(Camera::finalizedPosition * MAP_SCALE, model, first, first);
        }


        first = false;
    }

}

void CQuake3BSP::RenderBSP(const glm::vec3& cameraPos, tBSPModel& model, bool useClusterVis, bool lightmap)
{

    auto light = GetLightvolColor(Camera::finalizedPosition * MAP_SCALE);
    //printf("light : %f, %f, %f \n", light.ambientColor.x, light.ambientColor.y, light.ambientColor.z);

    DebugDraw::Line(Camera::finalizedPosition + Camera::Forward(), Camera::finalizedPosition + Camera::Forward() + light.direction, 0.01f);


    // 1. Find camera's cluster via BSP tree traversal
    int cameraCluster = FindClusterAtPosition(cameraPos);

    int drawnFaces = 0;

    ShaderProgram* shader = ShaderManager::GetShaderProgram("bsp", "bsp");
    shader->UseProgram();


	LightVolPointData lightData = { vec3(0),vec3(1) ,vec3(0) };

    if (lightmap == false)
    {

        vec3 min = vec3(model.mins[0], model.mins[1], model.mins[2]);
        vec3 max = vec3(model.maxs[0], model.maxs[1], model.maxs[2]);

		lightData = GetLightvolColor((min + max) / 2.0f);

        //lightData.ambientColor = vec3(1);

        DebugDraw::Line((min + max) / 2.0f / MAP_SCALE + vec3(1, 0, 0), (min + max) / 2.0f / MAP_SCALE + vec3(1, 0, 0) + lightData.direction, 0.01f);

    }

	if (useClusterVis)
	{

		// 2. Iterate through all leaves
		for (const tBSPLeaf& leaf : leafs) {
			if (leaf.cluster < 0)
				continue; // Skip invalid clusters

			// 3. Check visibility using visdata
			if (!IsClusterVisible(cameraCluster, leaf.cluster))
				continue;

			// 4. Render visible leaf's faces
			for (int i = 0; i < leaf.n_leaffaces; i++)
			{

				int faceIndex = leafFaces[leaf.leafface + i];

				if (model.face <= faceIndex && faceIndex < model.face + model.n_faces)
				{
					RenderSingleFace(faceIndex, shader, lightmap, lightData);

					drawnFaces++;
				}
			}
		}

    }
    else
    {
		for (int i = model.face; i < model.face + model.n_faces; i++)
		{

			RenderSingleFace(i, shader, lightmap, lightData);

			drawnFaces++;

        }
    }



    //printf("drawn %i faces\n", drawnFaces);

}

vector<BSPModelRef> CQuake3BSP::GetAllModelRefs()
{
    vector<BSPModelRef> refs;
    for (size_t i = 0; i < models.size(); ++i)
    {
        BSPModelRef ref(this, static_cast<int>(i), models[i]);
        refs.push_back(ref);             // Add the reference to the vector
    }
    return refs;
}

void AddPhysicsBodyForEntityAndModel(Entity* entity, BSPModelRef& model)
{
    auto vertices = model.GetVertices();

    vector<vec3> vertexPositions;

    for (auto& vertex : vertices)
    {
        vertexPositions.push_back(vertex.Position / MAP_SCALE);
    }

    RefConst<Shape> shape;

    vec3 bodyPos = vec3(0);

    if (model.model.face > 0 && model.model.n_faces == 0)
    {

        vec3 min = vec3(
            model.model.mins[0], 
            model.model.mins[1],
            model.model.mins[2]) / MAP_SCALE;

        vec3 max = vec3(
            model.model.maxs[0], 
            model.model.maxs[1], 
            model.model.maxs[2]) / MAP_SCALE;

        bodyPos = (min + max) / 2.0f;

        shape = Physics::CreateBoxShape(abs(max-min));


    }
    else if (entity->ConvexCollision)
    {
        shape = Physics::CreateConvexHullFromPoints(vertexPositions);
    }
    else
    {

        auto indices = model.GetIndices();

        MeshUtils::PositionVerticesIndices mesh;

        mesh.vertices = vertexPositions;
        mesh.indices = indices;
		mesh = MeshUtils::RemoveDegenerates(mesh, 0.01f, 0.00f);

        shape = Physics::CreateMeshShape(mesh.vertices, mesh.indices);
    }

    Body* body = Physics::CreateBodyFromShape(entity, vec3(0), shape,10,true,BodyType::World, BodyType::GroupCollisionTest);

    Physics::SetBodyPosition(body, bodyPos);

    entity->LeadBody = body;

    model.StaticNavigation = entity->Static;

}

void CQuake3BSP::LoadToLevel()
{
    auto parsedEntities = MapParser::ParseBSPEntities(entities);
    parsedEntities[0].Properties["classname"] = "worldspawn";

    auto models = GetAllModelRefs();

    for (auto& entityData : parsedEntities)
    {

        Entity* ent = LevelObjectFactory::instance().create(entityData.Classname);

        if (ent == nullptr)
            ent = new Entity();

        ent->FromData(entityData);

        string modelStr = entityData.GetPropertyString("model");

        if (modelStr.size() > 1)
        {
            if (modelStr[0] == '*')
            {

                int modelId = stoi(modelStr.substr(1, modelStr.size() - 1));

                BSPModelRef modelRef = models[modelId];

                AddPhysicsBodyForEntityAndModel(ent, modelRef);

                modelRef.CalculateAveragePosition();

                ent->Drawables.push_back(new BSPModelRef(modelRef));

            }
		}
		else if (ent->ClassName == "worldspawn") //worldspawn always has 0 model
		{
			BSPModelRef modelRef = models[0];

            AddPhysicsBodyForEntityAndModel(ent, modelRef);

            modelRef.CalculateAveragePosition();

            ent->Drawables.push_back(new BSPModelRef(modelRef));

		}

        Level::Current->AddEntity(ent);

	}

}

void CQuake3BSP::BSPDebug(int index) {
    printf("\n");
    printf("Face:----> %d\n", index);

    //for (unsigned int x = 0; x < Rbuffers.v_faceVBOs[index].size(); x++)
        //printf(">- %f\n", Rbuffers.v_faceVBOs[index][x]);

    //for (unsigned int x = 0; x < Rbuffers.v_faceIDXs[index].size(); x++)
        //printf("%d-\n", Rbuffers.v_faceIDXs[index][x]);

    //printf("VBuffer size=%lu(bytes)\n", sizeof(GLfloat) * Rbuffers.v_faceVBOs[index].size());
    //printf("IBuffer size=%lu(bytes)\n", sizeof(GLuint) * Rbuffers.v_faceIDXs[index].size());
    printf("EndFace.\n");
}

void CQuake3BSP::CreateRenderBuffers(int index) {
    auto& vertices = Rbuffers.v_faceVBOs[index];
    auto& indices = Rbuffers.v_faceIDXs[index];

    // Create buffers
    FB_array.FB_Idx[index].VBO = std::make_unique<VertexBuffer>(
        vertices, VertexData::Declaration()
    );
    FB_array.FB_Idx[index].EBO = std::make_unique<IndexBuffer>(indices);

    // Create VAO linking them
    FB_array.FB_Idx[index].VAO = std::make_unique<VertexArrayObject>(
        *FB_array.FB_Idx[index].VBO,
        *FB_array.FB_Idx[index].EBO
    );
}

string GetLightMapFilePathFromId(int id, const string& filePath)
{
    // Find the position of the last '/' and the last '.'
    size_t lastSlash = filePath.find_last_of("/\\");
    size_t lastDot = filePath.find_last_of('.');

    // Extract mapname from the path
    string mapName = filePath.substr(lastSlash + 1, lastDot - lastSlash - 1);

    // Construct the folder path
    string folder = filePath.substr(0, lastSlash + 1) + mapName + "/";

    // Construct the filename
    std::ostringstream oss;
    oss << "lm_" << std::setw(4) << std::setfill('0') << id;

    return folder + oss.str() + ".tga";
}
void CQuake3BSP::RenderSingleFace(int index, ShaderProgram* shader, bool lightmap, LightVolPointData lightData)
{

    // bind the face's VAO (which has its VBO/EBO & attribs)
    auto& buffers = FB_array.FB_Idx[index];

    buffers.VAO->Bind();

    // bind your textures as before
    tBSPFace* pFace = &m_pFaces[index];

	string texturePath = "GameData/" + string(pTextures[pFace->textureID].strName) + ".png";

    Texture* faceTexture = AssetRegistry::GetTextureFromFile(texturePath);
    GLuint lightmapId = (pFace->lightmapID >= 0)
        ? m_lightmap_gen_IDs[pFace->lightmapID]
        : missing_LM_id;

    if (m_numOfLightmaps == 0)
    {

        string lightMapPath = GetLightMapFilePathFromId(pFace->lightmapID, filePath);

        lightmapId = AssetRegistry::GetTextureFromFile(lightMapPath)->getID();
    }

    if (lightmap == false)
    {
        lightmapId = missing_LM_id;
    }

    shader->SetUniform("light_color", lightData.ambientColor);
    shader->SetUniform("direct_light_color", lightData.directColor);
    shader->SetUniform("direct_light_dir", lightData.direction);

    shader->SetTexture("s_bspTexture", faceTexture);
    shader->SetTexture("s_bspLightmap", lightmapId);
    shader->SetUniform("view", Camera::finalizedView);
    shader->SetUniform("projection", Camera::finalizedProjection);
    shader->SetUniform("model", glm::scale(vec3(1.0f / MAP_SCALE)));



    glDepthMask(GL_TRUE);


    // draw using the EBO already bound in the VAO; offset = 0
    glDrawElements(GL_TRIANGLES,
        pFace->numOfIndices,
        GL_UNSIGNED_INT,
        0);


    VertexArrayObject::Unbind();
}

void CQuake3BSP::GenerateTexture() 
{
   

} // end

void CQuake3BSP::GenerateLightmap() {
    // GLfloat aniso = 8.0f;


    float b = 1.0f;

    // generate missing lightmap
    GLubyte white_lightmap[] = {
        255,255,255,255,   // pixel #1 (RGBA)
        255,255,255,255,   // pixel #2
        255,255,255,255,   // pixel #3
        255,255,255,255    // pixel #4
    };

    glGenTextures(1, &missing_LM_id);
    glBindTexture(GL_TEXTURE_2D, missing_LM_id);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,             // internalFormat
        2, 2,
        0,
        GL_RGBA,             // format
        GL_UNSIGNED_BYTE,    // type
        white_lightmap       // data pointer
    );
    //glGenerateMipmap(GL_TEXTURE_2D);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);




    // generate lightmaps
    m_lightmap_gen_IDs = new GLuint[m_numOfLightmaps];
    glGenTextures(Rbuffers.G_lightMaps.size(), m_lightmap_gen_IDs);

    for (GLuint i = 0; i < Rbuffers.G_lightMaps.size(); i++) {
        glBindTexture(GL_TEXTURE_2D, m_lightmap_gen_IDs[i]);
        // glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &aniso);
        // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, &Rbuffers.G_lightMaps[i].imageBits);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

}

void CQuake3BSP::renderFaces() {
    for (auto& f : Rbuffers.v_faceVBOs)
        RenderSingleFace(f.first, nullptr, true, LightVolPointData());
}

mat4 BSPModelRef::GetWorldMatrix()
{
	return translate(Position) * MathHelper::GetRotationMatrix(Rotation) * scale(Scale);
}


void BSPModelRef::CalculateAveragePosition()
{
    auto vertices = GetVertices();

    avgPosition = vec3(0);

    for (auto& vertex : vertices)
    {
        avgPosition += vertex.Position / MAP_SCALE;
    }

    avgPosition /= (float)vertices.size();
}

vector<MeshUtils::PositionVerticesIndices> BSPModelRef::GetNavObstacleMeshes()
{
    vector<MeshUtils::PositionVerticesIndices> result;

    mat3 world = GetWorldMatrix();



    MeshUtils::PositionVerticesIndices meshData;

    meshData.indices = GetIndices();

    auto vertices = GetVertices();

    for (auto& vertex : vertices)
    {
		meshData.vertices.push_back(vertex.Position / MAP_SCALE);
    }

    meshData = MeshUtils::RemoveDegenerates(meshData, 0.01f, 0.00f);

    //DebugDraw::IndexedMesh(meshData.vertices, meshData.indices, 100);

    result.push_back(meshData);

    return result;
}

std::vector<VertexData> BSPModelRef::GetVertices() {
    std::vector<VertexData> result;
    // Iterate over all faces of the model
    for (int i = model.face; i < model.face + model.n_faces; i++) {
        // Get the vertex array for face i
        auto& faceVertices = bsp->Rbuffers.v_faceVBOs[i];
        // Append all vertices from this face to the result
        result.insert(result.end(), faceVertices.begin(), faceVertices.end());
    }
    return result;
}

std::vector<uint32_t> BSPModelRef::GetIndices() {
    std::vector<uint32_t> result;
    uint32_t vertexOffset = 0; // Tracks the number of vertices before the current face
    // Iterate over all faces of the model
    for (int i = model.face; i < model.face + model.n_faces; i++) {
        // Get the index array for face i
        auto& faceIndices = bsp->Rbuffers.v_faceIDXs[i];
        // Add each index, adjusted by the current offset
        for (auto idx : faceIndices) {
            result.push_back(static_cast<uint32_t>(idx) + vertexOffset);
        }
        // Update the offset by the number of vertices in this face
        vertexOffset += static_cast<uint32_t>(bsp->Rbuffers.v_faceVBOs[i].size());
    }
    return result;
}

void BSPModelRef::DrawForward(mat4x4 view, mat4x4 projection)
{
    bsp->RenderBSP(Camera::finalizedPosition, model, useBspVisibility, Static);
}
