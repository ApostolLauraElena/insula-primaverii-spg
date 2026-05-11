#include <iostream>
#include <vector>
#include <stack>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Fisierele noastre externe
#include "objloader.hpp"       // Pastram pentru cele 10 obiecte ascunse pe viitor
#include "ShaderUtils.h"       // Functiile utilitare pentru shadere
#include "CubGeometrie.h"      // Geometria procedurala a copacilor
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define PI glm::pi<float>()

// --- Variabile Globale ---
GLuint shader_programme;
glm::mat4 projectionMatrix, viewMatrix, modelMatrix;

// Geometria Insulei
GLuint vaoObj, vboObj;
std::vector<glm::vec3> vertices;
std::vector<glm::vec3> normals;
std::vector<glm::vec2> uvs; // Pastram pentru texturare viitoare

// Geometria procedurala
GLuint vaoCub, vboCub;

// Camera & Lumini
// --- Variabile Camera FPS ---
glm::vec3 cameraPos = glm::vec3(0.0f, 150.0f, 300.0f); // Pozitia ta în lume
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);    // Direcția în care te uiți
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);    // Direcția "în sus"

float yaw = -90.0f; // Rotirea stânga-dreapta
float pitch = -20.0f; // Rotirea sus-jos (privim puțin în jos inițial)
float lastX = 450.0f; // Mijlocul ferestrei (900/2)
float lastY = 350.0f; // Mijlocul ferestrei (700/2)
bool firstMouse = true;

float axisRotAngle = 0.0f;
glm::vec3 lightPos(0, 20000, 0);
glm::vec3 viewPos(2, 3, 6);

// --- Logica Pădurii ---
struct Cires {
	glm::vec3 pozitie;
	unsigned int seed;
};
std::vector<Cires> padure;

void planteazaPadurea(int numarCopaci, float distantaMinima) {
	srand(42);
	int incercari = 0;
	while (padure.size() < numarCopaci && incercari < 100000) {
		incercari++;
		int randomIndex = ((rand() << 15) | rand()) % vertices.size();
		glm::vec3 pozitieCandidat = vertices[randomIndex];

		if (pozitieCandidat.y < 0.5f) continue;

		bool ePreaAproape = false;
		for (const auto& copac : padure) {
			if (glm::distance(glm::vec2(pozitieCandidat.x, pozitieCandidat.z), glm::vec2(copac.pozitie.x, copac.pozitie.z)) < distantaMinima) {
				ePreaAproape = true; break;
			}
		}
		if (!ePreaAproape) padure.push_back({ pozitieCandidat, (unsigned int)rand() });
	}
	std::cout << "Am plantat " << padure.size() << " ciresi japonezi pe insula!\n";
}

bool loadTerrainFromHeightmap(const char* filepath, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec3>& out_normals) {
	int width, height, channels;
	unsigned char* data = stbi_load(filepath, &width, &height, &channels, 1);
	if (!data) { std::cout << "Eroare: Nu s-a putut incarca " << filepath << std::endl; return false; }

	float yScale = 0.2f, xzScale = 2.0f;
	float offsetX = (width * xzScale) / 2.0f, offsetZ = (height * xzScale) / 2.0f;

	auto getHeight = [&](int x, int z) -> float {
		if (x < 0) x = 0; if (x >= width) x = width - 1;
		if (z < 0) z = 0; if (z >= height) z = height - 1;
		return (float)data[z * width + x] * yScale;
		};

	for (int z = 0; z < height - 1; z++) {
		for (int x = 0; x < width - 1; x++) {
			glm::vec3 v1(x * xzScale - offsetX, getHeight(x, z), z * xzScale - offsetZ);
			glm::vec3 v2((x + 1) * xzScale - offsetX, getHeight(x + 1, z), z * xzScale - offsetZ);
			glm::vec3 v3(x * xzScale - offsetX, getHeight(x, z + 1), (z + 1) * xzScale - offsetZ);
			glm::vec3 v4((x + 1) * xzScale - offsetX, getHeight(x + 1, z + 1), (z + 1) * xzScale - offsetZ);

			glm::vec3 n1 = glm::normalize(glm::cross(v3 - v1, v2 - v1));
			glm::vec3 n2 = glm::normalize(glm::cross(v4 - v2, v3 - v2));

			out_vertices.push_back(v1); out_normals.push_back(n1);
			out_vertices.push_back(v3); out_normals.push_back(n1);
			out_vertices.push_back(v2); out_normals.push_back(n1);
			out_vertices.push_back(v2); out_normals.push_back(n2);
			out_vertices.push_back(v3); out_normals.push_back(n2);
			out_vertices.push_back(v4); out_normals.push_back(n2);
		}
	}
	stbi_image_free(data);
	std::cout << "Insula generata! Varfuri: " << out_vertices.size() << std::endl;
	return true;
}

void deseneazaCreanga(int adancime, glm::mat4 matriceCurenta, float lungime) {
	GLuint modelLoc = glGetUniformLocation(shader_programme, "modelViewProjectionMatrix");
	GLuint normalMatLoc = glGetUniformLocation(shader_programme, "normalMatrix");
	GLuint colorLoc = glGetUniformLocation(shader_programme, "objectColor");

	if (adancime == 0) {
		glUniform3f(colorLoc, 1.0f, 0.6f, 0.8f); // Roz

		// 1. Coroană MASIVĂ și organică:
		// Generăm o mărime aleatoare între 12.0 și 17.0 (în loc de vechiul 2.5)
		float marimeFloare = 12.0f + (rand() % 50) / 10.0f;

		// O turtim puțin pe axa Y (marimeFloare * 0.8f) ca să semene cu un nor de frunze
		glm::mat4 matFloare = glm::scale(matriceCurenta, glm::vec3(marimeFloare, marimeFloare * 0.8f, marimeFloare));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * matFloare));
		glUniformMatrix4fv(normalMatLoc, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(matFloare))));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		return;
	}

	glUniform3f(colorLoc, 0.35f, 0.2f, 0.1f); // Maro

	// 2. Trunchiuri GROASE: Am schimbat 0.2f cu 0.6f 
	glm::mat4 matCreanga = glm::translate(glm::scale(matriceCurenta, glm::vec3(0.6f * adancime, lungime, 0.6f * adancime)), glm::vec3(0.0f, 0.5f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * matCreanga));
	glUniformMatrix4fv(normalMatLoc, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(matCreanga))));
	glDrawArrays(GL_TRIANGLES, 0, 36);

	int ramuriNoi = (rand() % 2) + 2;
	for (int i = 0; i < ramuriNoi; i++) {
		glm::mat4 matUrm = glm::translate(matriceCurenta, glm::vec3(0.0f, lungime, 0.0f));
		matUrm = glm::rotate(matUrm, ((rand() % 100) / 100.0f - 0.5f) * 1.5f, glm::vec3(1, 0, 0));
		matUrm = glm::rotate(matUrm, ((rand() % 100) / 100.0f - 0.5f) * 1.5f, glm::vec3(0, 0, 1));

		// 3. Extindere: Scădem lungimea mai lent (0.8f în loc de 0.75f) ca arborele să fie mai lat
		deseneazaCreanga(adancime - 1, matUrm, lungime * 0.8f);
	}
}

void init() {
	glEnable(GL_DEPTH_TEST);
	glewInit();

	loadTerrainFromHeightmap("insula_heightmap.png", vertices, normals);

	// Combinare varfuri + normale pentru VBO
	std::vector<glm::vec3> verticesNormals = vertices;
	verticesNormals.insert(verticesNormals.end(), normals.begin(), normals.end());

	glGenVertexArrays(1, &vaoObj);
	glGenBuffers(1, &vboObj);
	glBindVertexArray(vaoObj);
	glBindBuffer(GL_ARRAY_BUFFER, vboObj);
	glBufferData(GL_ARRAY_BUFFER, verticesNormals.size() * sizeof(glm::vec3), &verticesNormals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(vertices.size() * sizeof(glm::vec3)));

	// Initializare Shadere folosind noul Header
	std::string vstext = textFileRead("vertex.vert");
	std::string fstext = textFileRead("fragment.frag");
	const char* vs_c = vstext.c_str();
	const char* fs_c = fstext.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_c, NULL); glCompileShader(vs);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_c, NULL); glCompileShader(fs);

	shader_programme = glCreateProgram();
	glAttachShader(shader_programme, fs); glAttachShader(shader_programme, vs);
	glLinkProgram(shader_programme);

	// Initializare VAO Cub Procedural (folosind array-ul din header)
	glGenVertexArrays(1, &vaoCub); glGenBuffers(1, &vboCub);
	glBindVertexArray(vaoCub); glBindBuffer(GL_ARRAY_BUFFER, vboCub);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubVertices), cubVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	planteazaPadurea(150, 12.0f);
}

void display() {
	glClearColor(0.5f, 0.8f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shader_programme);

	glUniform3fv(glGetUniformLocation(shader_programme, "lightPos"), 1, glm::value_ptr(lightPos));
	glUniform3fv(glGetUniformLocation(shader_programme, "viewPos"), 1, glm::value_ptr(viewPos));
	GLuint colorLoc = glGetUniformLocation(shader_programme, "objectColor");

	// 1. Desenare Insula
	glBindVertexArray(vaoObj);
	glUniform3f(colorLoc, 0.3f, 0.7f, 0.3f);
	modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -5.0f, 0.0f)) * glm::rotate(axisRotAngle, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(shader_programme, "modelViewProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix * viewMatrix * modelMatrix));
	glUniformMatrix4fv(glGetUniformLocation(shader_programme, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(modelMatrix))));
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	// 2. Desenare Padure
	glBindVertexArray(vaoCub);
	for (const auto& cires : padure) {
		srand(cires.seed);
		glm::mat4 matBaza = glm::rotate(glm::mat4(1.0f), axisRotAngle, glm::vec3(0, 1, 0));
		matBaza = glm::translate(matBaza, glm::vec3(cires.pozitie.x, cires.pozitie.y - 5.0f, cires.pozitie.z));
		deseneazaCreanga(4, matBaza, 20.0f);
	}
	glutSwapBuffers();
}

void reshape(int w, int h) {
	glViewport(0, 0, w, h);
	projectionMatrix = glm::perspective(PI / 4, (float)w / h, 0.1f, 1000.0f);
	viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}
void mouseCallback(int xpos, int ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // Inversat deoarece coordonatele Y merg de jos în sus
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.2f; // Cât de rapid se mișcă camera
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// Blocăm camera să nu ne dăm peste cap (limita de 89 de grade)
	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	// Calculăm noul vector de direcție (cameraFront) folosind trigonometrie
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);

	// Actualizăm matricea de vizualizare
	viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glutPostRedisplay();
}
void keyboard(unsigned char key, int x, int y) {
	float cameraSpeed = 10.0f; // Viteza ta de alergare pe insulă

	switch (key) {
		// --- Mișcare FPS (Te raportezi la direcția în care te uiți) ---
	case 'w': case 'W': cameraPos += cameraSpeed * cameraFront; break; // Înainte
	case 's': case 'S': cameraPos -= cameraSpeed * cameraFront; break; // Înapoi
	case 'a': case 'A': cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed; break; // Stânga
	case 'd': case 'D': cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed; break; // Dreapta

		// --- Ca o dronă (sus/jos) ---
	case 'e': case 'E': cameraPos += cameraSpeed * cameraUp; break; // Urcă
	case 'q': case 'Q': cameraPos -= cameraSpeed * cameraUp; break; // Coboară

		// --- Rotire insulă (am mutat pe tastele R și F ca să păstrăm WASD pentru mișcare) ---
	case 'r': case 'R': axisRotAngle += 0.05f; break;
	case 'f': case 'F': axisRotAngle -= 0.05f; break;
	}

	// Menținem spectatorul la suprafață (Să nu cazi sub hartă)
	if (cameraPos.y < 5.0f) cameraPos.y = 5.0f;

	// Actualizăm matricea
	viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glutPostRedisplay();
}
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(900, 700);
	glutCreateWindow("SPG - Insula Primaverii");
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouseCallback);
	glutMainLoop();
	return 0;
}