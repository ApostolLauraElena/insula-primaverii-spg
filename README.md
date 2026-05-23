# 🌸 Insula Primăverii - Proiect Grafică 3D (OpenGL)

**Insula Primăverii** este o scenă 3D interactivă dezvoltată în **C++** utilizând API-ul **OpenGL**. Proiectul explorează concepte și tehnici avansate de grafică pe calculator, precum optimizarea randării prin Instancing, efecte volumetrice pentru vegetație (Shell Rendering), simularea fizică a apei și manipularea terenului prin Heightmaps.

---

## 🚀 Caracteristici Tehnice

### 🌿 Iarbă Volumetrică (Shell Rendering)
* **Shell Texturing:** Iarba este randată folosind 32 de straturi suprapuse (shells), deplasate de-a lungul normalei terenului.
* **Alpha Clipping & Noise:** Perforarea straturilor se face pe baza unei texturi de zgomot (*noise texture*), creând iluzia firelor individuale de iarbă pe măsură ce straturile se înalță.
* **Vânt Procedural:** Vertex Shader-ul aplică o funcție bazată pe timp (sin/cos) pentru a simula direcția și forța vântului direct pe geometria ierbii.

### 🌳 Pădure de Cireși (Hardware Instancing)
* **Instanced Rendering:** Pentru o performanță optimă, zeci de cireși (trunchi + frunze din fișiere `.obj`) sunt desenați simultan cu un singur apel de desenare către GPU (`glDrawArraysInstanced`).
* **Distribuție Inteligentă:** Copacii sunt plasați aleatoriu pe insulă, cu un algoritm de tip distanță minimă (Collision/Distance Check) pentru a preveni suprapunerea modelelor. Variații de scară și rotație sunt aplicate fiecărei instanțe.

### 🌊 Apă Dinamică cu Valuri Gerstner
* **Vertex Displacement:** Geometria planului de apă este deformată în timp real folosind o sumă de **Valuri Gerstner** (Gerstner Waves), care oferă o simulare mult mai realistă a valurilor ascuțite față de funcțiile sinus clasice.
* **Shading Avansat (Fragment Shader):** Combinarea a două texturi de normale (Normal Mapping) animate la viteze diferite pentru detalii acvatice, tranziție de culoare bazată pe adâncime, iluminare speculară Blinn-Phong și efect **Fresnel** pentru reflexii.

### ⛰️ Teren generat prin Heightmap
* **Procesare Imagine:** Terenul insulei este reconstruit dintr-o imagine de tip *heightmap* (grayscale) folosind `stb_image`, transformând datele de intensitate a pixelilor în coordonate reale 3D.
* **Smooth Fading:** Interpolare și estompare a marginilor insulei pentru o tranziție lină către nivelul apei.

### ☀️ Iluminare și Atmosferă
* **Soare Billboard:** Randarea unui model de soare orientat permanent către camera vizuală.
* **Fog Effects (Ceață):** Implementarea unui sistem de ceață dependent de distanță pentru a masca limitele hărții și a crea profunzime.

### 🎮 Navigare FPS
* **Cameră Dinamică:** Sistem de cameră *First-Person* care permite explorarea liberă a insulei, cu clamp pentru a nu depăși granițele apei și a terenului.

---

## 🛠️ Tehnologii Utilizate

* **Limbaj:** C++
* **Grafică:** OpenGL 4.0+
* **Limbaj Shaders:** GLSL (Vertex & Fragment Shaders)
* **Biblioteci:**
  * **GLEW:** Gestionarea extensiilor OpenGL.
  * **FreeGLUT:** Crearea ferestrelor, bucla de randare și gestionarea input-ului.
  * **GLM:** Matematică pentru grafică (matrici, vectori, transformări).
  * **stb_image:** Încărcarea texturilor și a hărților de înălțime.

---

## ⌨️ Controale

| Tasta | Acțiune |
| :--- | :--- |
| **W / A / S / D** | Deplasare liberă (Înainte / Stânga / Înpoi / Dreapta) |
| **Mouse** | Rotirea camerei (Yaw / Pitch) pentru a privi în jur |
| **Q / E** | Zbor / Altitudine (Jos / Sus) |
| **R / F** | Rotirea întregii insule (Model Matrix) în jurul axei Y |
| **T / G** | Ajustarea manuală a unghiului Pitch (Privire Sus / Jos) |

---

## 📸 Capturi de Ecran

*(Aici poți urca imaginile salvate de tine pe parcursul proiectului pentru a oferi un preview vizual rapid. Recomand capturi care să pună în valoare iarba volumetrică, valurile și pădurea.)*
