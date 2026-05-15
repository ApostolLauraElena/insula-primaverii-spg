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
#define PI glm::pi<float>()

// --- Global variables ---
GLuint shader_programme;
glm::mat4 projectionMatrix, viewMatrix, modelMatrix;

// Terrain geometry
GLuint vaoObj, vboObj;
std::vector<glm::vec3> vertices;
std::vector<glm::vec3> normals;
std::vector<glm::vec2> uvs;

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


// Genereaza un plan plat verde
void createFlatTerrain(float size, int subdiviziuni,
    std::vector<glm::vec3>& out_v, std::vector<glm::vec3>& out_n) {

    float pas = size / subdiviziuni;
    float offset = size / 2.0f;
    glm::vec3 normala(0.0f, 1.0f, 0.0f);

    for (int z = 0; z < subdiviziuni; z++) {
        for (int x = 0; x < subdiviziuni; x++) {
            glm::vec3 v1(x * pas - offset, 0.0f, z * pas - offset);
            glm::vec3 v2((x + 1) * pas - offset, 0.0f, z * pas - offset);
            glm::vec3 v3(x * pas - offset, 0.0f, (z + 1) * pas - offset);
            glm::vec3 v4((x + 1) * pas - offset, 0.0f, (z + 1) * pas - offset);

            // Triunghi 1
            out_v.push_back(v1); out_n.push_back(normala);
            out_v.push_back(v3); out_n.push_back(normala);
            out_v.push_back(v2); out_n.push_back(normala);
            // Triunghi 2
            out_v.push_back(v2); out_n.push_back(normala);
            out_v.push_back(v3); out_n.push_back(normala);
            out_v.push_back(v4); out_n.push_back(normala);
        }
    }
    std::cout << "Plan plat generat! Varfuri: " << out_v.size() << "\n";
}


void createFlatTerrain(float size, int subdiviziuni,
    std::vector<glm::vec3>& out_v, std::vector<glm::vec3>& out_n, std::vector<glm::vec2>& out_uv) { // NOU: out_uv

    float pas = size / subdiviziuni;
    float offset = size / 2.0f;
    glm::vec3 normala(0.0f, 1.0f, 0.0f);

    for (int z = 0; z < subdiviziuni; z++) {
        for (int x = 0; x < subdiviziuni; x++) {
            glm::vec3 v1(x * pas - offset, 0.0f, z * pas - offset);
            glm::vec3 v2((x + 1) * pas - offset, 0.0f, z * pas - offset);
            glm::vec3 v3(x * pas - offset, 0.0f, (z + 1) * pas - offset);
            glm::vec3 v4((x + 1) * pas - offset, 0.0f, (z + 1) * pas - offset);

            // Gener�m coordonatele UV (�ntre 0.0 ?i 1.0)
            glm::vec2 uv1((float)x / subdiviziuni, (float)z / subdiviziuni);
            glm::vec2 uv2((float)(x + 1) / subdiviziuni, (float)z / subdiviziuni);
            glm::vec2 uv3((float)x / subdiviziuni, (float)(z + 1) / subdiviziuni);
            glm::vec2 uv4((float)(x + 1) / subdiviziuni, (float)(z + 1) / subdiviziuni);

            // Triunghi 1
            out_v.push_back(v1); out_n.push_back(normala); out_uv.push_back(uv1);
            out_v.push_back(v3); out_n.push_back(normala); out_uv.push_back(uv3);
            out_v.push_back(v2); out_n.push_back(normala); out_uv.push_back(uv2);
            // Triunghi 2
            out_v.push_back(v2); out_n.push_back(normala); out_uv.push_back(uv2);
            out_v.push_back(v3); out_n.push_back(normala); out_uv.push_back(uv3);
            out_v.push_back(v4); out_n.push_back(normala); out_uv.push_back(uv4);
        }
    }
    std::cout << "Plan plat generat! Varfuri: " << out_v.size() << "\n";
}
GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        std::cout << "Eroare la incarcarea texturii de iarba: " << path << std::endl;
    }
    stbi_image_free(data);
    return textureID;
}
void init() {
    glEnable(GL_DEPTH_TEST);
    glewInit();

    createFlatTerrain(1024.0f, 100, vertices, normals, uvs);

    // Incarcam textura de noise
    noiseTexture = loadTexture("noise.png"); //noise.png

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

    GLuint colorLoc = glGetUniformLocation(shader_programme, "objectColor");

    // Activeaza texturile pentru iarba
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glUniform1i(glGetUniformLocation(shader_programme, "noiseTexture"), 0);

    // 1. Plan plat verde (Iarba Volumetrica)
    glBindVertexArray(vaoObj);
    glUniform3f(colorLoc, 0.3f, 0.7f, 0.3f);
    modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0))
        * glm::rotate(axisRotAngle, glm::vec3(0, 1, 0));
    glUniformMatrix4fv(glGetUniformLocation(shader_programme, "modelViewProjectionMatrix"),
        1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader_programme, "modelMatrix"),
        1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader_programme, "normalMatrix"),
        1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(modelMatrix))));

    // Setari pt iarba
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

    // 2. Padure

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