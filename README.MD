# Going Merry - Visualización 3D con OpenGL

## 1. Nombre del proyecto

**Going Merry: Visualización e interacción con un modelo 3D mediante C++.**

## 2. Descripción del proyecto

Este proyecto consiste en una aplicación gráfica desarrollada en C++ que permite visualizar e interactuar con un modelo tridimensional del barco Going Merry, de la serie One Piece.

Se utiliza OpenGL para representar el modelo en pantalla y Assimp para cargar el archivo 3D en formato GLB. El programa permite modificar el tamaño y la posición del barco, además de visualizar su rotación automática.

## 3. Tecnologías utilizadas

- **C++:** lenguaje principal de programación.
- **OpenGL 3.3:** renderizado de gráficos 3D.
- **GLFW:** creación de la ventana y captura de entradas del teclado.
- **GLAD:** acceso a las funciones de OpenGL.
- **GLM:** operaciones matemáticas y transformaciones 3D.
- **Assimp:** carga y procesamiento del modelo 3D.
- **GLSL:** programación de los shaders.
- **Visual Studio:** entorno de desarrollo.

## 4. Arquitectura del software

El programa se divide en varios procesos que permiten cargar, preparar y mostrar el modelo tridimensional.

**1. Inicialización:** configura GLFW, crea la ventana de 800 × 600 píxeles e inicializa GLAD.

**2. Carga del modelo:** Assimp importa el archivo GLB y procesa sus mallas, vértices, índices y normales.

**3. Preparación de las mallas:** los datos del modelo se organizan y envían a la GPU mediante VAO, VBO y EBO.

**4. Shaders e iluminación:** el Vertex Shader procesa las posiciones y normales de los vértices, mientras que el Fragment Shader calcula el color y la iluminación del modelo.

**5. Cámara y transformaciones:** GLM permite manejar las matrices de modelo, vista y proyección para controlar la posición, orientación y tamaño del barco.

**6. Interacción y renderizado:** GLFW detecta las teclas presionadas y el programa actualiza la escena y dibuja el modelo continuamente.

## 5. Controles

| Tecla | Función |
| W / S | Mover el barco sobre el eje Y. |
| A / D | Mover el barco sobre el eje X. |
|   E   | Aumentar el tamaño.            |
|   Q   | Disminuir el tamaño.           |
|  ESC  | Cerrar el programa.            |

El barco también cuenta con una rotación automática sobre el eje Y. Su escala está limitada entre 0.2 y 5.0.

## 6. Estructura del proyecto

- `FileName.cpp`: contiene el código principal, la carga del modelo y el renderizado.
- `glad.c`: implementación de GLAD.
- `Modelo/`: contiene el archivo 3D `one_piece_-going_merry.glb`.
- `libraries/`: contiene las bibliotecas utilizadas.
- `Project1/`: contiene archivos del proyecto de Visual Studio.
- `Project1.vcxproj`: configuración del proyecto.
- `.gitignore`: excluye archivos temporales y generados.

## 7. Requisitos de ejecución

- Sistema operativo Windows.
- Visual Studio con herramientas de C++.
- Compatibilidad con OpenGL 3.3.
- Bibliotecas GLFW, GLAD, GLM y Assimp.
- Archivo GLB ubicado en la carpeta `Modelo`.

## 8. Autor

Proyecto académico desarrollado en C++ y OpenGL.
