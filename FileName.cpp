#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Cargar la cabecera de stb_image
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


// Dimensiones de la ventana
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

// -------------------------------------------------------------
// SHADERS
// -------------------------------------------------------------

const char* vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec4 aColor;\n"
"layout (location = 2) in vec2 aTexCoord;\n"

"out vec4 vColor;\n"
"out vec2 vTexCoord;\n"

"uniform mat4 uMVP;\n"

"void main() {\n"
"    gl_Position = uMVP * vec4(aPos, 1.0);\n"
"    vColor = aColor;\n"
"    vTexCoord = aTexCoord;\n"
"}\0";

const char* fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"

"in vec4 vColor;\n"
"in vec2 vTexCoord;\n"

"uniform sampler2D uTex;\n"

"void main() {\n"
"    vec4 tex = texture(uTex, vTexCoord);\n"
"    FragColor = tex * vColor;\n"
"}\0";

// -------------------------------------------------------------
// CAMBIO DE TAMAÑO DE VENTANA
// -------------------------------------------------------------

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// -------------------------------------------------------------
// CARGAR TEXTURA
// -------------------------------------------------------------

unsigned int loadTexture(const char* filename) {

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* data =
        stbi_load(filename, &width, &height, &nrChannels, 0);

    if (data) {

        GLenum format = GL_RGB;

        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            format,
            width,
            height,
            0,
            format,
            GL_UNSIGNED_BYTE,
            data
        );

        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            GL_REPEAT
        );

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            GL_REPEAT
        );

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            GL_LINEAR_MIPMAP_LINEAR
        );

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER,
            GL_LINEAR
        );

        std::cout << "SUCCESS: Textura cargada correctamente ["
            << filename << "] ("
            << width << "x"
            << height << ")"
            << std::endl;
    }
    else {

        std::cerr << "ERROR: No se pudo cargar la textura: "
            << filename << std::endl;
    }

    stbi_image_free(data);

    return textureID;
}

// -------------------------------------------------------------
// MAIN
// -------------------------------------------------------------

int main() {

    std::cout << "Proyecto: Piramide 3D con Textura - Lorena Obando 6001000"
        << std::endl;

    // ---------------------------------------------------------
    // INICIALIZAR GLFW
    // ---------------------------------------------------------

    if (!glfwInit()) {

        std::cerr << "Error al inicializar GLFW"
            << std::endl;

        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE
    );

    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    // Crear ventana
    GLFWwindow* window = glfwCreateWindow(
        SCR_WIDTH,
        SCR_HEIGHT,
        "Piramide 3D con Textura",
        nullptr,
        nullptr
    );

    if (window == nullptr) {

        std::cerr << "Error al crear la ventana GLFW"
            << std::endl;

        glfwTerminate();

        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(
        window,
        framebuffer_size_callback
    );

    // ---------------------------------------------------------
    // INICIALIZAR GLAD
    // ---------------------------------------------------------

    if (!gladLoadGLLoader(
        (GLADloadproc)glfwGetProcAddress)) {

        std::cerr << "Error al inicializar GLAD"
            << std::endl;

        return -1;
    }

    // Activar profundidad
    glEnable(GL_DEPTH_TEST);

    // ---------------------------------------------------------
    // COMPILAR SHADER DE VÉRTICES
    // ---------------------------------------------------------

    unsigned int vertexShader =
        glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(
        vertexShader,
        1,
        &vertexShaderSource,
        NULL
    );

    glCompileShader(vertexShader);

    // ---------------------------------------------------------
    // COMPILAR SHADER DE FRAGMENTOS
    // ---------------------------------------------------------

    unsigned int fragmentShader =
        glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(
        fragmentShader,
        1,
        &fragmentShaderSource,
        NULL
    );

    glCompileShader(fragmentShader);

    // ---------------------------------------------------------
    // CREAR PROGRAMA DE SHADERS
    // ---------------------------------------------------------

    unsigned int shaderProgram =
        glCreateProgram();

    glAttachShader(
        shaderProgram,
        vertexShader
    );

    glAttachShader(
        shaderProgram,
        fragmentShader
    );

    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ---------------------------------------------------------
    // CARGAR TEXTURA
    // ---------------------------------------------------------

    unsigned int texture1 =
        loadTexture("texturaescamas.jpg");

    // ---------------------------------------------------------
    // VÉRTICES DE LA PIRÁMIDE
    // ---------------------------------------------------------
    //
    //              4
    //             /|\
    //            / | \
    //           /  |  \
    //          0---|---1
    //           \  |  /
    //            \ | /
    //             \|/
    //              2
    //
    // Base cuadrada:
    // 0, 1, 2, 3
    //
    // Punta:
    // 4
    //
    // Cada vértice contiene:
    // posición = 3 floats
    // color    = 4 floats
    // textura  = 2 floats
    //
    // Total = 9 floats por vértice

    float verticesPiramide[] = {

        // -----------------------------------------------------
        // CARA FRONTAL
        // -----------------------------------------------------

        // posición             color                     UV

        -0.6f, -0.6f,  0.6f,
        0.10f, 0.95f, 0.85f, 1.0f,
        0.0f, 0.0f,

         0.6f, -0.6f,  0.6f,
        1.00f, 0.35f, 0.30f, 1.0f,
        1.0f, 0.0f,

         0.0f,  0.7f,  0.0f,
        0.55f, 0.20f, 1.00f, 1.0f,
        0.5f, 1.0f,


        // -----------------------------------------------------
        // CARA DERECHA
        // -----------------------------------------------------

         0.6f, -0.6f,  0.6f,
        1.00f, 0.35f, 0.30f, 1.0f,
        0.0f, 0.0f,

         0.6f, -0.6f, -0.6f,
        0.10f, 0.95f, 0.85f, 1.0f,
        1.0f, 0.0f,

         0.0f,  0.7f,  0.0f,
        0.55f, 0.20f, 1.00f, 1.0f,
        0.5f, 1.0f,


        // -----------------------------------------------------
        // CARA TRASERA
        // -----------------------------------------------------

         0.6f, -0.6f, -0.6f,
        0.10f, 0.95f, 0.85f, 1.0f,
        0.0f, 0.0f,

        -0.6f, -0.6f, -0.6f,
        1.00f, 0.35f, 0.30f, 1.0f,
        1.0f, 0.0f,

         0.0f,  0.7f,  0.0f,
        0.55f, 0.20f, 1.00f, 1.0f,
        0.5f, 1.0f,


        // -----------------------------------------------------
        // CARA IZQUIERDA
        // -----------------------------------------------------

        -0.6f, -0.6f, -0.6f,
        1.00f, 0.35f, 0.30f, 1.0f,
        0.0f, 0.0f,

        -0.6f, -0.6f,  0.6f,
        0.10f, 0.95f, 0.85f, 1.0f,
        1.0f, 0.0f,

         0.0f,  0.7f,  0.0f,
        0.55f, 0.20f, 1.00f, 1.0f,
        0.5f, 1.0f,


        // -----------------------------------------------------
        // BASE
        // -----------------------------------------------------

        -0.6f, -0.6f,  0.6f,
        0.10f, 0.95f, 0.85f, 1.0f,
        0.0f, 0.0f,

         0.6f, -0.6f,  0.6f,
        1.00f, 0.35f, 0.30f, 1.0f,
        1.0f, 0.0f,

         0.6f, -0.6f, -0.6f,
        0.55f, 0.20f, 1.00f, 1.0f,
        1.0f, 1.0f,


        -0.6f, -0.6f,  0.6f,
        0.10f, 0.95f, 0.85f, 1.0f,
        0.0f, 0.0f,

         0.6f, -0.6f, -0.6f,
        0.55f, 0.20f, 1.00f, 1.0f,
        1.0f, 1.0f,

        -0.6f, -0.6f, -0.6f,
        1.00f, 0.35f, 0.30f, 1.0f,
        0.0f, 1.0f
    };

    // ---------------------------------------------------------
    // VAO Y VBO
    // ---------------------------------------------------------

    unsigned int VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(
        GL_ARRAY_BUFFER,
        VBO
    );

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(verticesPiramide),
        verticesPiramide,
        GL_STATIC_DRAW
    );

    // ---------------------------------------------------------
    // ATRIBUTO 0: POSICIÓN
    // ---------------------------------------------------------

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        9 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    // ---------------------------------------------------------
    // ATRIBUTO 1: COLOR
    // ---------------------------------------------------------

    glVertexAttribPointer(
        1,
        4,
        GL_FLOAT,
        GL_FALSE,
        9 * sizeof(float),
        (void*)(3 * sizeof(float))
    );

    glEnableVertexAttribArray(1);

    // ---------------------------------------------------------
    // ATRIBUTO 2: TEXTURA
    // ---------------------------------------------------------

    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        9 * sizeof(float),
        (void*)(7 * sizeof(float))
    );

    glEnableVertexAttribArray(2);

    // ---------------------------------------------------------
    // CONFIGURAR TEXTURA
    // ---------------------------------------------------------

    glUseProgram(shaderProgram);

    glUniform1i(
        glGetUniformLocation(
            shaderProgram,
            "uTex"
        ),
        0
    );

    int mvpLoc =
        glGetUniformLocation(
            shaderProgram,
            "uMVP"
        );

    // ---------------------------------------------------------
    // BUCLE PRINCIPAL
    // ---------------------------------------------------------

    while (!glfwWindowShouldClose(window)) {

        // Color del fondo
        glClearColor(
            0.10f,
            0.35f,
            0.38f,
            1.0f
        );

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT
        );

        glUseProgram(shaderProgram);

        // -----------------------------------------------------
        // ACTIVAR TEXTURA
        // -----------------------------------------------------

        glActiveTexture(GL_TEXTURE0);

        glBindTexture(
            GL_TEXTURE_2D,
            texture1
        );

        // -----------------------------------------------------
        // PROYECCIÓN
        // -----------------------------------------------------

        int width = 0;
        int height = 0;

        glfwGetFramebufferSize(
            window,
            &width,
            &height
        );

        float aspect =
            width > 0
            ? (float)width / (float)height
            : 1.0f;

        glm::mat4 projection =
            glm::perspective(
                glm::radians(45.0f),
                aspect,
                0.1f,
                100.0f
            );

        // -----------------------------------------------------
        // CÁMARA
        // -----------------------------------------------------

        glm::mat4 view =
            glm::lookAt(
                glm::vec3(0.0f, 0.2f, 3.0f),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );

        // -----------------------------------------------------
        // ROTACIÓN SOBRE EL EJE Y
        // -----------------------------------------------------

        float t =
            (float)glfwGetTime();

        glm::mat4 model =
            glm::mat4(1.0f);

        model =
            glm::rotate(
                model,
                t * 0.8f,
                glm::vec3(0.0f, 1.0f, 0.0f)
            );

        // -----------------------------------------------------
        // MVP
        // -----------------------------------------------------

        glm::mat4 mvp =
            projection *
            view *
            model;

        glUniformMatrix4fv(
            mvpLoc,
            1,
            GL_FALSE,
            glm::value_ptr(mvp)
        );

        // -----------------------------------------------------
        // DIBUJAR PIRÁMIDE
        // -----------------------------------------------------

        glBindVertexArray(VAO);

        glDrawArrays(
            GL_TRIANGLES,
            0,
            18
        );

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    // ---------------------------------------------------------
    // LIBERAR RECURSOS
    // ---------------------------------------------------------

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();

    return 0;
}