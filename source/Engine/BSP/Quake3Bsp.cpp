
#include "Quake3Bsp.h"

#include <algorithm> // std::sort
#include <cstring>   // GCC7 fix
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "../AssetRegisty.h"
#include "../ShaderManager.h"
#include "../Camera.h"


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

CQuake3BSP::~CQuake3BSP()
{
    delete[] pTextures;
    delete[] pLightmaps;
}

bool CQuake3BSP::LoadBSP(const char* filename) {

    if (!filename) {
        printf("ERROR:: You must specify BSP file as parameter");
        return 0;
    }

    FILE* fp = NULL;
    fopen_s(&fp, filename, "rb");
    if (fp == NULL) {
        printf("ERROR:: cannot open BSP file: %s\n", filename);
        return 0;
    }

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
    // We create a local pointer of tBSPTextures because we don't need
    // that information once we create texture maps from it.
    m_numOfTextures = lumps[kTextures].length / sizeof(tBSPTexture);

    // Seek to the position in the file that stores the vertex information
    fseek(fp, lumps[kVertices].offset, SEEK_SET);

    // Since Quake has the Z-axis pointing up, we want to convert the data so
    // that Y-axis is pointing up (like normal!) :)
    // Go through all of the vertices that need to be read
    for (int i = 0; i < m_numOfVerts; i++) {
        fread(&m_pVerts[i], 1, sizeof(tBSPVertex), fp);
        // Swap Y/Z and negate Z for vertex position
        float temp = m_pVerts[i].vPosition.y;
        m_pVerts[i].vPosition.y = m_pVerts[i].vPosition.z;
        m_pVerts[i].vPosition.z = -temp;

        // Also swap normal coordinates
        temp = m_pVerts[i].vNormal.y;
        m_pVerts[i].vNormal.y = m_pVerts[i].vNormal.z;
        m_pVerts[i].vNormal.z = -temp;
    }

    // Add plane coordinate conversion after loading planes:
    for (auto& plane : planes) {
        // Swap Y/Z and negate Z for plane normal
        float temp = plane.normal.y;
        plane.normal.y = plane.normal.z;
        plane.normal.z = -temp;

        // Recalculate plane distance with new normal
        glm::vec3 oldNormal(plane.normal.x, plane.normal.z, -plane.normal.y);
        plane.dist = glm::dot(oldNormal, glm::vec3(0, 0, 0)) - plane.dist;
    }

    // Convert model bounds after loading models:
    for (auto& model : models) {
        // Swap Y/Z and negate Z for mins/maxs
        float temp = model.mins[1];
        model.mins[1] = model.mins[2];
        model.mins[2] = -temp;

        temp = model.maxs[1];
        model.maxs[1] = model.maxs[2];
        model.maxs[2] = -temp;
    }

    pTextures = new tBSPTexture[m_numOfTextures];

    fseek(fp, lumps[kIndices].offset, SEEK_SET);                // Seek the index information
    fread(m_pIndices, m_numOfIndices, sizeof(int), fp);         // index information
    fseek(fp, lumps[kFaces].offset, SEEK_SET);                  // Seek the face information
    fread(m_pFaces, m_numOfFaces, sizeof(tBSPFace), fp);        // face information
    fseek(fp, lumps[kTextures].offset, SEEK_SET);               // Seek the texture information
    fread(pTextures, m_numOfTextures, sizeof(tBSPTexture), fp); // texture information

    // Create a texture from the image
    for (int i = 0; i < m_numOfTextures; i++) {
        // Find the extension if any and append it to the file name
        strcpy_s(tname[i], pTextures[i].strName);
        strcat_s(tname[i], ".jpg");
        printf("loading: %s \n", tname[i]);
    }

    m_numOfLightmaps = lumps[kLightmaps].length / sizeof(tBSPLightmap);
    tBSPLightmap* pLightmaps = new tBSPLightmap[m_numOfLightmaps];
    // Seek to the position in the file that stores the lightmap information
    fseek(fp, lumps[kLightmaps].offset, SEEK_SET);

    // Go through all of the lightmaps and read them in
    for (int i = 0; i < m_numOfLightmaps; i++) {
        // Read in the RGB data for each lightmap
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

    for (int v = 0; v < pFace->numOfVerts; v++) {
        Rbuffers.v_faceVBOs[index].push_back(m_pVerts[pFace->startVertIndex + v].vPosition.x);
        Rbuffers.v_faceVBOs[index].push_back(m_pVerts[pFace->startVertIndex + v].vPosition.y);
        Rbuffers.v_faceVBOs[index].push_back(m_pVerts[pFace->startVertIndex + v].vPosition.z);

        Rbuffers.v_faceVBOs[index].push_back(m_pVerts[pFace->startVertIndex + v].vTextureCoord.x);
        Rbuffers.v_faceVBOs[index].push_back(m_pVerts[pFace->startVertIndex + v].vTextureCoord.y);

        Rbuffers.v_faceVBOs[index].push_back(m_pVerts[pFace->startVertIndex + v].vLightmapCoord.x);
        Rbuffers.v_faceVBOs[index].push_back(m_pVerts[pFace->startVertIndex + v].vLightmapCoord.y);
    }
}

void CQuake3BSP::CreateIndices(int index) {
    tBSPFace* pFace = &m_pFaces[index];
    // printf("idx: [%d]\n", index);
    for (int j = 0; j < pFace->numOfIndices; j++)
        Rbuffers.v_faceIDXs[index].push_back(m_pIndices[j + pFace->startIndex]);
}

glm::vec3 CQuake3BSP::GetLightvolColor(const glm::vec3& position)
{
    // Get world bounds from base model (convert float[3] to glm::vec3)
    glm::vec3 modelMins(models[0].mins[0], models[0].mins[1], models[0].mins[2]);
    glm::vec3 modelMaxs(models[0].maxs[0], models[0].maxs[1], models[0].maxs[2]);

    // Calculate grid dimensions according to Q3 BSP specs
    glm::ivec3 lightVolGridDims(
        static_cast<int>(std::floor(modelMaxs.x / 64.0f) - std::ceil(modelMins.x / 64.0f) + 1,
            static_cast<int>(std::floor(modelMaxs.y / 64.0f) - std::ceil(modelMins.y / 64.0f) + 1,
                static_cast<int>(std::floor(modelMaxs.z / 128.0f) - std::ceil(modelMins.z / 128.0f) + 1
                    ))));

    // Calculate grid indices from world position
    int nx = static_cast<int>((position.x - modelMins.x) / 64.0f);
    int ny = static_cast<int>((position.y - modelMins.y) / 64.0f);
    int nz = static_cast<int>((position.z - modelMins.z) / 128.0f);

    // Clamp to grid dimensions using scalar clamp
    nx = glm::clamp(nx, 0, lightVolGridDims.x - 1);
    ny = glm::clamp(ny, 0, lightVolGridDims.y - 1);
    nz = glm::clamp(nz, 0, lightVolGridDims.z - 1);

    // Get the light volume at this grid cell
    const tBSPLightvol & vol = lightVols[nz * (lightVolGridDims.x * lightVolGridDims.y) +
        ny * lightVolGridDims.x + nx];

    // Convert byte values (0-255) to float (0.0-1.0)
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

    return ambient + directional;
}

int CQuake3BSP::FindCameraCluster(const glm::vec3& cameraPos) {
    printf("Camera position: %.2f, %.2f, %.2f\n",
        cameraPos.x, cameraPos.y, cameraPos.z);

    int nodeIndex = 0;
    int depth = 0;

    while (nodeIndex >= 0 && depth < 100) { // Prevent infinite loops
        const tBSPNode& node = nodes[nodeIndex];
        const tBSPPlane& plane = planes[node.plane];

        printf("Node %d - Plane normal: %.2f, %.2f, %.2f Dist: %.2f\n",
            nodeIndex, plane.normal.x, plane.normal.y, plane.normal.z, plane.dist);

        float distance = glm::dot(cameraPos, plane.normal) - plane.dist;
        int childIndex = distance >= 0 ? 0 : 1;
        int nextChild = node.children[childIndex];

        printf("Distance: %.2f, Child: %d, Next: %d\n",
            distance, childIndex, nextChild);

        if (nextChild < 0) {
            int leafIndex = -(nextChild + 1);
            printf("Found leaf %d, cluster %d\n",
                leafIndex, leafs[leafIndex].cluster);
            return leafs[leafIndex].cluster;
        }

        nodeIndex = nextChild;
        depth++;
    }

    printf("Failed to find cluster after %d iterations\n", depth);
    return -1;
}

void CQuake3BSP::DrawForward(mat4x4 view, mat4x4 projection)
{
    RenderBSP(Camera::finalizedPosition * 16.0f);
}

void CQuake3BSP::RenderBSP(const glm::vec3& cameraPos)
{


    // 1. Find camera's cluster via BSP tree traversal
    int cameraCluster = FindCameraCluster(cameraPos);

    int drawnFaces = 0;

    // 2. Iterate through all leaves
    for (const tBSPLeaf& leaf : leafs) {
        if (leaf.cluster < 0) 
            continue; // Skip invalid clusters

        // 3. Check visibility using visdata
        if (!IsClusterVisible(cameraCluster, leaf.cluster)) 
            continue;

        // 4. Render visible leaf's faces
        for (int i = 0; i < leaf.n_leaffaces; i++) {
            int faceIndex = leafFaces[leaf.leafface + i];
            RenderSingleFace(faceIndex);
            drawnFaces++;
        }
    }

    printf("drawn %i faces\n", cameraCluster);

}

void CQuake3BSP::BSPDebug(int index) {
    printf("\n");
    printf("Face:----> %d\n", index);

    for (unsigned int x = 0; x < Rbuffers.v_faceVBOs[index].size(); x++)
        printf(">- %f\n", Rbuffers.v_faceVBOs[index][x]);

    for (unsigned int x = 0; x < Rbuffers.v_faceIDXs[index].size(); x++)
        printf("%d-\n", Rbuffers.v_faceIDXs[index][x]);

    //printf("VBuffer size=%lu(bytes)\n", sizeof(GLfloat) * Rbuffers.v_faceVBOs[index].size());
    //printf("IBuffer size=%lu(bytes)\n", sizeof(GLuint) * Rbuffers.v_faceIDXs[index].size());
    printf("EndFace.\n");
}

void CQuake3BSP::CreateRenderBuffers(int index)
{
    // 1) generate & bind a VAO for this face, store it
    glGenVertexArrays(1, &FB_array.FB_Idx[index].VAO);
    glBindVertexArray(FB_array.FB_Idx[index].VAO);

    // 2) upload your vertex data into a VBO
    glGenBuffers(1, &FB_array.FB_Idx[index].VBO);
    glBindBuffer(GL_ARRAY_BUFFER, FB_array.FB_Idx[index].VBO);
    glBufferData(GL_ARRAY_BUFFER,
        sizeof(GLfloat) * Rbuffers.v_faceVBOs[index].size(),
        Rbuffers.v_faceVBOs[index].data(),
        GL_STATIC_DRAW);

    // 3) upload your index data into an EBO
    glGenBuffers(1, &FB_array.FB_Idx[index].EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, FB_array.FB_Idx[index].EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        sizeof(int) * Rbuffers.v_faceIDXs[index].size(),
        Rbuffers.v_faceIDXs[index].data(),
        GL_STATIC_DRAW);

    // 4) set up your attribute pointers once and for all
    //    each vertex: [ x y z | u v | lm_u lm_v ] = 3+2+2 = 7 floats
    glEnableVertexAttribArray(0); // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        7 * sizeof(GLfloat), (void*)(0));

    glEnableVertexAttribArray(1); // texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        7 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(2); // lightmap coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
        7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));

    // 5) unbind VAO to lock in state
    glBindVertexArray(0);

    // optional: unbind VBO/EBO to keep clean
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CQuake3BSP::RenderSingleFace(int index)
{
    ShaderProgram* shader = ShaderManager::GetShaderProgram("bsp", "bsp");
    shader->UseProgram();

    // bind the face's VAO (which has its VBO/EBO & attribs)
    glBindVertexArray(FB_array.FB_Idx[index].VAO);

    // bind your textures as before
    tBSPFace* pFace = &m_pFaces[index];
    Texture* faceTexture = AssetRegistry::GetTextureFromFile("GameData/Textures/brushes/cat.png");
    GLuint lightmapId = (pFace->lightmapID >= 0)
        ? m_lightmap_gen_IDs[pFace->lightmapID]
        : missing_LM_id;

    shader->SetTexture("s_bspTexture", faceTexture);
    shader->SetTexture("s_bspLightmap", lightmapId);
    shader->SetUniform("view", Camera::finalizedView);
    shader->SetUniform("projection", Camera::finalizedProjection);
    shader->SetUniform("model", glm::scale(vec3(1.0f / 16.0f)));

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);


    glDepthMask(GL_TRUE);


    // draw using the EBO already bound in the VAO; offset = 0
    glDrawElements(GL_TRIANGLES,
        pFace->numOfIndices,
        GL_UNSIGNED_INT,
        0);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // unbind for cleanliness
    glBindVertexArray(0);
}

void CQuake3BSP::GenerateTexture() 
{
   

} // end

void CQuake3BSP::GenerateLightmap() {
    // GLfloat aniso = 8.0f;

    std::ofstream logfile;
    logfile.open("log.txt");
    logfile << "LOG::\n\n";

    // generate missing lightmap
    GLfloat white_lightmap[] =
        {1.0f, 1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f, 1.0f};

    glGenTextures(1, &missing_LM_id);
    glBindTexture(GL_TEXTURE_2D, missing_LM_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGB, GL_FLOAT, &white_lightmap);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    logfile << "lm START ID: " << missing_LM_id << "\n";
    logfile << "--------------\n";


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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, &Rbuffers.G_lightMaps[i].imageBits);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    logfile << "m_numOfLightmaps " << m_numOfLightmaps;

    logfile.close();
}

void CQuake3BSP::renderFaces() {
    for (auto& f : Rbuffers.v_faceVBOs)
        RenderSingleFace(f.first);
}
