# 🌸 Insula Primăverii - Proiect Grafică 3D (OpenGL)

**Insula Primăverii** este o scenă 3D interactivă dezvoltată în **C++** utilizând API-ul **OpenGL**. Proiectul explorează concepte avansate de grafică pe calculator, precum generarea procedurală a vegetației, manipularea terenului prin Heightmaps și animații în timp real folosind Vertex Shaders.

---

## 🚀 Caracteristici Tehnice

### 🌳 Pădure Procedurală de Cireși Japonezi
* **Algoritm Recursiv:** Copacii (Sakura) sunt generați prin recursivitate, imitând structura fractală a ramurilor naturale.
* **Variație Aleatorie:** Fiecare copac este unic, având parametri generați aleatoriu pentru unghiul ramurilor, lungime și densitatea coroanei de flori roz.

### ⛰️ Teren generat prin Heightmap
* **Procesare Imagine:** Terenul insulei este reconstruit dintr-o imagine de tip *heightmap* (grayscale), transformând datele de intensitate a pixelilor în coordonate pe axa Y.
* **Optimizare:** Utilizarea de **Vertex Buffer Objects (VBO)** și **Vertex Array Objects (VAO)** pentru o randare eficientă a sutelor de mii de poligoane.

### 🌊 Sistem de Apă și Atmosferă
* **Vertex Shader Animation:** Mișcarea valurilor este calculată direct pe placa video (GPU) folosind funcții trigonometrice (sin/cos) pentru o performanță fluidă.
* **Fog Effects (Ceață):** Implementarea unui sistem de ceață exponențială pentru a spori adâncimea scenei și a crea o atmosferă de „descoperire”.

### 🎮 Navigare FPS
* **Cameră Dinamică:** Sistem de cameră *First-Person* care permite explorarea liberă a insulei.
* **Interactivitate:** Control complet prin tastatură (WASD) și rotație fluidă prin mouse (Euler angles: Yaw/Pitch).

---

## 🛠️ Tehnologii Utilizate

* **Limbaj:** C++
* **Grafică:** OpenGL 4.0+
* **Limbaj Shaders:** GLSL
* **Biblioteci:**
  * **GLEW:** Gestionarea extensiilor OpenGL.
  * **FreeGLUT:** Crearea ferestrelor și gestionarea input-ului.
  * **GLM:** Matematică pentru grafică (matrici, vectori).
  * **stb_image:** Încărcarea texturilor și a hărților de înălțime.

---

## ⌨️ Controale

| Tasta | Acțiune |
| :--- | :--- |
| **W / A / S / D** | Deplasare pe insulă |
| **Mouse** | Rotirea camerei (Privire în jur) |
| **Q / E** | Zbor (Sus / Jos) |
| **R / F** | Rotirea întregii insule |

---

## 📸 Capturi de Ecran

*(Aici poți urca imaginile salvate de tine pe parcursul proiectului pentru a oferi un preview vizual rapid)*

---
