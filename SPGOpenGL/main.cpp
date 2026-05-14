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

// Forest
struct Cires { glm::vec3 pozitie; unsigned int seed; };
std::vector<Cires> padure;

void planteazaPadurea(int numarCopaci, float distantaMinima) {
    srand(42);
    int incercari = 0;
    while ((int)padure.size() < numarCopaci && incercari < 10000) {
        incercari++;
        int randomIndex = ((rand() << 15) | rand()) % (int)vertices.size();
        glm::vec3 poz = vertices[randomIndex];
        // Pe un plan plat nu mai filtram dupa inaltime
        bool preaAproape = false;
        for (const auto& c : padure)
            if (glm::distance(glm::vec2(poz.x, poz.z), glm::vec2(c.pozitie.x, c.pozitie.z)) < distantaMinima)
            {
                preaAproape = true; break;
            }
        if (!preaAproape) padure.push_back({ poz, (unsigned int)rand() });
    }
    std::cout << "Am plantat " << padure.size() << " ciresi japonezi pe insula!\n";
}

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

void deseneazaCreanga(int adancime, glm::mat4 matriceCurenta, float lungime) {
    GLuint modelLoc = glGetUniformLocation(shader_programme, "modelViewProjectionMatrix");
    GLuint normalLoc = glGetUniformLocation(shader_programme, "normalMatrix");
    GLuint colorLoc = glGetUniformLocation(shader_programme, "objectColor");
    if (adancime == 0) {
        glUniform3f(colorLoc, 1.0f, 0.6f, 0.8f);
        float m = 12.0f + (rand() % 50) / 10.0f;
        glm::mat4 mat = glm::scale(matriceCurenta, glm::vec3(m, m * 0.8f, m));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * mat));
        glUniformMatrix4fv(normalLoc, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(mat))));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        return;
    }
    glUniform3f(colorLoc, 0.35f, 0.2f, 0.1f);
    glm::mat4 mat = glm::translate(glm::scale(matriceCurenta, glm::vec3(0.6f * adancime, lungime, 0.6f * adancime)), glm::vec3(0, 0.5f, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * mat));
    glUniformMatrix4fv(normalLoc, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(mat))));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    int rNoi = (rand() % 2) + 2;
    for (int i = 0; i < rNoi; i++) {
        glm::mat4 m2 = glm::translate(matriceCurenta, glm::vec3(0, lungime, 0));
        m2 = glm::rotate(m2, ((rand() % 100) / 100.0f - 0.5f) * 1.5f, glm::vec3(1, 0, 0));
        m2 = glm::rotate(m2, ((rand() % 100) / 100.0f - 0.5f) * 1.5f, glm::vec3(0, 0, 1));
        deseneazaCreanga(adancime - 1, m2, lungime * 0.8f);
    }
}


void init() {
    glEnable(GL_DEPTH_TEST);
    glewInit();

    // Plan plat: 500 unitati, 50 subdiviziuni
    createFlatTerrain(1024.0f, 100, vertices, normals);

    std::vector<glm::vec3> vn = vertices;
    vn.insert(vn.end(), normals.begin(), normals.end());
    glGenVertexArrays(1, &vaoObj); glGenBuffers(1, &vboObj);
    glBindVertexArray(vaoObj); glBindBuffer(GL_ARRAY_BUFFER, vboObj);
    glBufferData(GL_ARRAY_BUFFER, vn.size() * sizeof(glm::vec3), &vn[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(vertices.size() * sizeof(glm::vec3)));

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

    planteazaPadurea(50, 20.0f);
}

void display() {
    glClearColor(0.5f, 0.8f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_programme);

    glUniform3fv(glGetUniformLocation(shader_programme, "lightPos"), 1, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(shader_programme, "viewPos"), 1, glm::value_ptr(viewPos));

    GLuint colorLoc = glGetUniformLocation(shader_programme, "objectColor");

    // 1. Plan plat verde
    glBindVertexArray(vaoObj);
    glUniform3f(colorLoc, 0.3f, 0.7f, 0.3f);
    modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0))
        * glm::rotate(axisRotAngle, glm::vec3(0, 1, 0));
    glUniformMatrix4fv(glGetUniformLocation(shader_programme, "modelViewProjectionMatrix"),
        1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader_programme, "normalMatrix"),
        1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(modelMatrix))));
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());

    // 2. Padure
    glBindVertexArray(vaoCub);
    for (const auto& cires : padure) {
        srand(cires.seed);
        glm::mat4 matBaza = glm::rotate(glm::mat4(1.0f), axisRotAngle, glm::vec3(0, 1, 0));
        matBaza = glm::translate(matBaza, glm::vec3(cires.pozitie.x, cires.pozitie.y, cires.pozitie.z));
        deseneazaCreanga(4, matBaza, 20.0f);
    }

    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    projectionMatrix = glm::perspective(PI / 4, (float)w / h, 0.1f, 2000.0f);
    viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

void mouseCallback(int xpos, int ypos) {
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = (xpos - lastX) * 0.2f, yoffset = (lastY - ypos) * 0.2f;
    lastX = xpos; lastY = ypos;
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
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(mouseCallback);
    glutMainLoop();
    return 0;
}