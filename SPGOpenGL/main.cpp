#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "objloader.hpp"
#include "ShaderUtils.h"
#include "CubGeometrie.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint noiseTexture;
GLuint waterNormalTexture;
GLuint heightmapMaskTexture;
#define PI glm::pi<float>()
const float LAND_MASK_THRESHOLD = 0.08f;
const float WATER_COAST_PADDING = 64.0f;

// --- Global variables ---
GLuint shader_programme;
glm::mat4 projectionMatrix, viewMatrix, modelMatrix;

// Terrain geometry
GLuint vaoObj, vboObj;
std::vector<glm::vec3> vertices;
std::vector<glm::vec3> normals;
std::vector<glm::vec2> uvs;

// Water geometry
GLuint vaoWater, vboWater;
std::vector<glm::vec3> waterVertices;
std::vector<glm::vec3> waterNormals;
std::vector<glm::vec2> waterUvs;
float waterLevel = 0.5f;

// Cube (trees)
GLuint vaoCub, vboCub;

// Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 150.0f, 300.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch = -20.0f;
float lastX = 450.0f, lastY = 350.0f;
bool firstMouse = true;

float axisRotAngle = 0.0f;
glm::vec3 lightPos(0, 20000, 0);
glm::vec3 viewPos(2, 3, 6);



void createFlatTerrain(float size, int subdiviziuni,
    std::vector<glm::vec3>& out_v, std::vector<glm::vec3>& out_n, std::vector<glm::vec2>& out_uv) {

    float pas = size / subdiviziuni;
    float offset = size / 2.0f;
    glm::vec3 normala(0.0f, 1.0f, 0.0f);

    for (int z = 0; z < subdiviziuni; z++) {
        for (int x = 0; x < subdiviziuni; x++) {
            glm::vec3 v1(x * pas - offset, 0.0f, z * pas - offset);
            glm::vec3 v2((x + 1) * pas - offset, 0.0f, z * pas - offset);
            glm::vec3 v3(x * pas - offset, 0.0f, (z + 1) * pas - offset);
            glm::vec3 v4((x + 1) * pas - offset, 0.0f, (z + 1) * pas - offset);

            glm::vec2 uv1((float)x / subdiviziuni, (float)z / subdiviziuni);
            glm::vec2 uv2((float)(x + 1) / subdiviziuni, (float)z / subdiviziuni);
            glm::vec2 uv3((float)x / subdiviziuni, (float)(z + 1) / subdiviziuni);
            glm::vec2 uv4((float)(x + 1) / subdiviziuni, (float)(z + 1) / subdiviziuni);

            out_v.push_back(v1); out_n.push_back(normala); out_uv.push_back(uv1);
            out_v.push_back(v3); out_n.push_back(normala); out_uv.push_back(uv3);
            out_v.push_back(v2); out_n.push_back(normala); out_uv.push_back(uv2);
            out_v.push_back(v2); out_n.push_back(normala); out_uv.push_back(uv2);
            out_v.push_back(v3); out_n.push_back(normala); out_uv.push_back(uv3);
            out_v.push_back(v4); out_n.push_back(normala); out_uv.push_back(uv4);
        }
    }
    std::cout << "Plan plat generat! Varfuri: " << out_v.size() << "\n";
}

void createWaterPlane(float waterSize, float terrainSize, int subdiviziuni, float level,
    std::vector<glm::vec3>& out_v, std::vector<glm::vec3>& out_n, std::vector<glm::vec2>& out_uv) {

    float pas = waterSize / subdiviziuni;
    float waterOffset = waterSize / 2.0f;
    float halfTerrain = terrainSize / 2.0f; // 1024 / 2 = 512
    glm::vec3 normala(0.0f, 1.0f, 0.0f);

    for (int z = 0; z < subdiviziuni; ++z) {
        for (int x = 0; x < subdiviziuni; ++x) {
            // Calculăm limitele (stânga, dreapta, jos, sus) pentru celula curentă
            float xMin = x * pas - waterOffset;
            float xMax = (x + 1) * pas - waterOffset;
            float zMin = z * pas - waterOffset;
            float zMax = (z + 1) * pas - waterOffset;

            // Omitere: dacă întreaga celulă se află în interiorul zonei de 1024x1024 a terenului
            if (xMin >= -halfTerrain && xMax <= halfTerrain &&
                zMin >= -halfTerrain && zMax <= halfTerrain) {
                continue; // Trecem peste, lăsând o "gaură" rectangulară pentru insulă
            }

            // Cele 4 vârfuri ale celulei de apă
            glm::vec3 v1(xMin, level, zMin);
            glm::vec3 v2(xMax, level, zMin);
            glm::vec3 v3(xMin, level, zMax);
            glm::vec3 v4(xMax, level, zMax);

            // Coordonatele UV pentru maparea texturii de apă normală
            glm::vec2 uv1((float)x / subdiviziuni, (float)z / subdiviziuni);
            glm::vec2 uv2((float)(x + 1) / subdiviziuni, (float)z / subdiviziuni);
            glm::vec2 uv3((float)x / subdiviziuni, (float)(z + 1) / subdiviziuni);
            glm::vec2 uv4((float)(x + 1) / subdiviziuni, (float)(z + 1) / subdiviziuni);

            // Triunghiul 1
            out_v.push_back(v1); out_n.push_back(normala); out_uv.push_back(uv1);
            out_v.push_back(v3); out_n.push_back(normala); out_uv.push_back(uv3);
            out_v.push_back(v2); out_n.push_back(normala); out_uv.push_back(uv2);

            // Triunghiul 2
            out_v.push_back(v2); out_n.push_back(normala); out_uv.push_back(uv2);
            out_v.push_back(v3); out_n.push_back(normala); out_uv.push_back(uv3);
            out_v.push_back(v4); out_n.push_back(normala); out_uv.push_back(uv4);
        }
    }

    std::cout << "Inelul de apa geometric a fost generat! Varfuri: " << out_v.size() << "\n";
}

void createHeightmapTerrain(const char* path, float size, int subdiviziuni, float heightScale,
    std::vector<glm::vec3>& out_v, std::vector<glm::vec3>& out_n, std::vector<glm::vec2>& out_uv) {

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 1);
    if (!data) {
        std::cout << "Eroare la incarcarea heightmap-ului: " << path << ". Folosesc teren plat.\n";
        createFlatTerrain(size, subdiviziuni, out_v, out_n, out_uv);
        return;
    }

    int gridSize = subdiviziuni + 1;
    float pas = size / subdiviziuni;
    float offset = size / 2.0f;
    std::vector<float> heights(gridSize * gridSize);
    std::vector<float> landMask(gridSize * gridSize);

    auto sampleRawHeight = [&](float u, float v) -> float {
        float imgX = u * (width - 1);
        float imgY = v * (height - 1);
        int x0 = (int)imgX;
        int y0 = (int)imgY;
        int x1 = x0 + 1;
        int y1 = y0 + 1;
        if (x1 >= width) x1 = width - 1;
        if (y1 >= height) y1 = height - 1;

        float tx = imgX - x0;
        float ty = imgY - y0;
        float h00 = data[y0 * width + x0] / 255.0f;
        float h10 = data[y0 * width + x1] / 255.0f;
        float h01 = data[y1 * width + x0] / 255.0f;
        float h11 = data[y1 * width + x1] / 255.0f;
        float h0 = h00 * (1.0f - tx) + h10 * tx;
        float h1 = h01 * (1.0f - tx) + h11 * tx;
        return h0 * (1.0f - ty) + h1 * ty;
        };

    auto sampleHeight = [&](float u, float v) -> float {
        float h = sampleRawHeight(u, v);
        h = h * h * (3.0f - 2.0f * h);
        return h * heightScale;
        };

    for (int z = 0; z < gridSize; ++z) {
        for (int x = 0; x < gridSize; ++x) {
            float u = (float)x / subdiviziuni;
            float v = (float)z / subdiviziuni;
            heights[z * gridSize + x] = sampleHeight(u, v);
            landMask[z * gridSize + x] = sampleRawHeight(u, v);
        }
    }

    stbi_image_free(data);

    auto heightAt = [&](int x, int z) -> float {
        if (x < 0) x = 0;
        if (z < 0) z = 0;
        if (x >= gridSize) x = gridSize - 1;
        if (z >= gridSize) z = gridSize - 1;
        return heights[z * gridSize + x];
        };

    auto maskAt = [&](int x, int z) -> float {
        if (x < 0) x = 0;
        if (z < 0) z = 0;
        if (x >= gridSize) x = gridSize - 1;
        if (z >= gridSize) z = gridSize - 1;
        return landMask[z * gridSize + x];
        };

    auto pointAt = [&](int x, int z) -> glm::vec3 {
        return glm::vec3(x * pas - offset, heightAt(x, z), z * pas - offset);
        };

    auto normalAt = [&](int x, int z) -> glm::vec3 {
        float hL = heightAt(x - 1, z);
        float hR = heightAt(x + 1, z);
        float hD = heightAt(x, z - 1);
        float hU = heightAt(x, z + 1);
        return glm::normalize(glm::vec3(hL - hR, 2.0f * pas, hD - hU));
        };

    for (int z = 0; z < subdiviziuni; ++z) {
        for (int x = 0; x < subdiviziuni; ++x) {
          
            glm::vec3 v1 = pointAt(x, z);
            glm::vec3 v2 = pointAt(x + 1, z);
            glm::vec3 v3 = pointAt(x, z + 1);
            glm::vec3 v4 = pointAt(x + 1, z + 1);

            glm::vec3 n1 = normalAt(x, z);
            glm::vec3 n2 = normalAt(x + 1, z);
            glm::vec3 n3 = normalAt(x, z + 1);
            glm::vec3 n4 = normalAt(x + 1, z + 1);

            glm::vec2 uv1((float)x / subdiviziuni, (float)z / subdiviziuni);
            glm::vec2 uv2((float)(x + 1) / subdiviziuni, (float)z / subdiviziuni);
            glm::vec2 uv3((float)x / subdiviziuni, (float)(z + 1) / subdiviziuni);
            glm::vec2 uv4((float)(x + 1) / subdiviziuni, (float)(z + 1) / subdiviziuni);

            out_v.push_back(v1); out_n.push_back(n1); out_uv.push_back(uv1);
            out_v.push_back(v3); out_n.push_back(n3); out_uv.push_back(uv3);
            out_v.push_back(v2); out_n.push_back(n2); out_uv.push_back(uv2);
            out_v.push_back(v2); out_n.push_back(n2); out_uv.push_back(uv2);
            out_v.push_back(v3); out_n.push_back(n3); out_uv.push_back(uv3);
            out_v.push_back(v4); out_n.push_back(n4); out_uv.push_back(uv4);
        }
    }

    std::cout << "Teren heightmap generat! Varfuri: " << out_v.size()
        << " | Heightmap: " << width << "x" << height << " | Subdiviziuni: " << subdiviziuni << "\n";
}
GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height, nrChannels;

    // MODIFICARE: Schimbăm ultimul parametru din 0 în 4 (STBI_rgb_alpha)
    // Acest lucru forțează stbi_load să returneze ÎNTOTDEAUNA un buffer RGBA (4 octeți/pixel)
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 4);

    if (data) {
        // Deoarece am forțat încărcarea în 4 canale, formatul va fi mereu GL_RGBA
        GLenum format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        std::cout << "Eroare la incarcarea texturii: " << path << std::endl;
    }

    stbi_image_free(data);
    return textureID;
}
void init() {
    glEnable(GL_DEPTH_TEST);
    glewInit();

    createHeightmapTerrain("heightmap.png", 1024.0f, 128, 60.0f, vertices, normals, uvs);
    // În loc de vechiul apel, folosește-l pe acesta:
    createWaterPlane(1400.0f, 1024.0f, 128, waterLevel, waterVertices, waterNormals, waterUvs);
    // Incarcam textura de noise
    noiseTexture = loadTexture("noise.jpeg"); //noise.png
    waterNormalTexture = loadTexture("Water.jpg");
    heightmapMaskTexture = loadTexture("heightmap.png");
    glBindTexture(GL_TEXTURE_2D, heightmapMaskTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Construim buffer-ul interclas�nd toate cele 3 atribute (pozitii, normale, uv-uri) 
    std::vector<float> vboData;
    for (size_t i = 0; i < vertices.size(); i++) { vboData.push_back(vertices[i].x); vboData.push_back(vertices[i].y); vboData.push_back(vertices[i].z); }
    for (size_t i = 0; i < normals.size(); i++) { vboData.push_back(normals[i].x); vboData.push_back(normals[i].y); vboData.push_back(normals[i].z); }
    for (size_t i = 0; i < uvs.size(); i++) { vboData.push_back(uvs[i].x); vboData.push_back(uvs[i].y); }

    glGenVertexArrays(1, &vaoObj); glGenBuffers(1, &vboObj);
    glBindVertexArray(vaoObj); glBindBuffer(GL_ARRAY_BUFFER, vboObj);
    glBufferData(GL_ARRAY_BUFFER, vboData.size() * sizeof(float), vboData.data(), GL_STATIC_DRAW);

    // Attribute 0: Pozitii
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    // Attribute 1: Normale
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(vertices.size() * 3 * sizeof(float)));
    // Attribute 2: UV-uri
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)((vertices.size() + normals.size()) * 3 * sizeof(float)));

    std::vector<float> waterVboData;
    for (size_t i = 0; i < waterVertices.size(); i++) { waterVboData.push_back(waterVertices[i].x); waterVboData.push_back(waterVertices[i].y); waterVboData.push_back(waterVertices[i].z); }
    for (size_t i = 0; i < waterNormals.size(); i++) { waterVboData.push_back(waterNormals[i].x); waterVboData.push_back(waterNormals[i].y); waterVboData.push_back(waterNormals[i].z); }
    for (size_t i = 0; i < waterUvs.size(); i++) { waterVboData.push_back(waterUvs[i].x); waterVboData.push_back(waterUvs[i].y); }

    glGenVertexArrays(1, &vaoWater); glGenBuffers(1, &vboWater);
    glBindVertexArray(vaoWater); glBindBuffer(GL_ARRAY_BUFFER, vboWater);
    glBufferData(GL_ARRAY_BUFFER, waterVboData.size() * sizeof(float), waterVboData.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(waterVertices.size() * 3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)((waterVertices.size() + waterNormals.size()) * 3 * sizeof(float)));

    std::string vstext = textFileRead("vertex.vert");
    std::string fstext = textFileRead("fragment.frag");
    const char* vs_c = vstext.c_str();
    const char* fs_c = fstext.c_str();
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_c, NULL); glCompileShader(vs);
    printShaderInfoLog(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_c, NULL); glCompileShader(fs);
    printShaderInfoLog(fs);
    shader_programme = glCreateProgram();
    glAttachShader(shader_programme, fs); glAttachShader(shader_programme, vs);
    glLinkProgram(shader_programme);
    printProgramInfoLog(shader_programme);

    glGenVertexArrays(1, &vaoCub); glGenBuffers(1, &vboCub);
    glBindVertexArray(vaoCub); glBindBuffer(GL_ARRAY_BUFFER, vboCub);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubVertices), cubVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glBindVertexArray(0);

}

void display() {
    glClearColor(0.5f, 0.8f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_programme);

    glUniform3fv(glGetUniformLocation(shader_programme, "lightPos"), 1, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(shader_programme, "viewPos"), 1, glm::value_ptr(cameraPos));
    glUniform1f(glGetUniformLocation(shader_programme, "time"), glutGet(GLUT_ELAPSED_TIME) / 1000.0f);
    glUniform1f(glGetUniformLocation(shader_programme, "waterLevel"), waterLevel);

    GLuint colorLoc = glGetUniformLocation(shader_programme, "objectColor");

    // Activeaza texturile pentru iarba
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glUniform1i(glGetUniformLocation(shader_programme, "noiseTexture"), 0);

    // 1. Plan plat verde (Iarba Volumetrica)
    glBindVertexArray(vaoObj);
    glUniform3f(colorLoc, 0.22f, 0.48f, 0.16f);
    modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0))
        * glm::rotate(axisRotAngle, glm::vec3(0, 1, 0));
    glUniformMatrix4fv(glGetUniformLocation(shader_programme, "modelViewProjectionMatrix"),
        1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader_programme, "modelMatrix"),
        1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader_programme, "normalMatrix"),
        1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(modelMatrix))));

    // Setari pt iarba
    glUniform1i(glGetUniformLocation(shader_programme, "isWater"), 0);
    glUniform1i(glGetUniformLocation(shader_programme, "isGrass"), 1);
    glDisable(GL_CULL_FACE);

    int numShells = 32;
    for (int i = 0; i < numShells; ++i) {
        float shellHeight = (float)i / (float)numShells;
        glUniform1f(glGetUniformLocation(shader_programme, "shellHeight"), shellHeight);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());
    }

    glEnable(GL_CULL_FACE);
    glUniform1i(glGetUniformLocation(shader_programme, "isGrass"), 0);
    glUniform1f(glGetUniformLocation(shader_programme, "shellHeight"), 0.0f);

    // 2. Apa
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, waterNormalTexture);
    glUniform1i(glGetUniformLocation(shader_programme, "waterNormalTexture"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, heightmapMaskTexture);
    glUniform1i(glGetUniformLocation(shader_programme, "heightmapMaskTexture"), 2);
    glUniform1f(glGetUniformLocation(shader_programme, "terrainSize"), 1024.0f);
    glUniform1f(glGetUniformLocation(shader_programme, "landMaskThreshold"), LAND_MASK_THRESHOLD);
    glUniform1f(glGetUniformLocation(shader_programme, "waterCoastPadding"), WATER_COAST_PADDING);
    glUniform1i(glGetUniformLocation(shader_programme, "isWater"), 1);

    glBindVertexArray(vaoWater);
    modelMatrix = glm::rotate(glm::mat4(1.0f), axisRotAngle, glm::vec3(0, 1, 0));
    glUniformMatrix4fv(glGetUniformLocation(shader_programme, "modelViewProjectionMatrix"),
        1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader_programme, "modelMatrix"),
        1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader_programme, "normalMatrix"),
        1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(modelMatrix))));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)waterVertices.size());
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glUniform1i(glGetUniformLocation(shader_programme, "isWater"), 0);

    // 3. Padure

    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    projectionMatrix = glm::perspective(PI / 4, (float)w / h, 0.1f, 2000.0f);
    viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

void mouseCallback(int xpos, int ypos) {
    if (firstMouse) { lastX = static_cast<float>(xpos); lastY = static_cast<float>(ypos); firstMouse = false; }
    float xoffset = (xpos - lastX) * 0.2f, yoffset = (lastY - ypos) * 0.2f;
    lastX = static_cast<float>(xpos); lastY = static_cast<float>(ypos);
    yaw += xoffset; pitch += yoffset;
    if (pitch > 89) pitch = 89; if (pitch < -89) pitch = -89;
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
    viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    float speed = 10.0f;
    switch (key) {
    case 'w': case 'W': cameraPos += speed * cameraFront; break;
    case 's': case 'S': cameraPos -= speed * cameraFront; break;
    case 'a': case 'A': cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed; break;
    case 'd': case 'D': cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed; break;
    case 'e': case 'E': cameraPos += speed * cameraUp; break;
    case 'q': case 'Q': cameraPos -= speed * cameraUp; break;
    case 'r': case 'R': axisRotAngle += 0.05f; break;
    case 'f': case 'F': axisRotAngle -= 0.05f; break;
    }
    if (cameraPos.y < 5.0f) cameraPos.y = 5.0f;
    viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(900, 700);
    glutCreateWindow("SPG - Plan Verde");
    init();
    glutIdleFunc(display);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(mouseCallback);
    glutMainLoop();
    return 0;
}