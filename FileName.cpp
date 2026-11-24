
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <cstddef>
#include <algorithm>
#include <limits>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//=====================================
// Configuracion
//=====================================

const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 900;

glm::vec3 posicionBarco(0.0f);
float escalaBarco = 1.0f;

//=====================================
// Entrada del teclado
//=====================================

void processInput(GLFWwindow* window, float deltaTime)
{
    float speed = 1.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        posicionBarco.y += speed;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        posicionBarco.y -= speed;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        posicionBarco.x -= speed;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        posicionBarco.x += speed;

    // Aumentar el tamaño del barco
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        escalaBarco += 0.8f * deltaTime;

    // Reducir el tamaño del barco
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        escalaBarco -= 0.8f * deltaTime;

    // Limitar el tamaño del modelo
    escalaBarco = glm::clamp(escalaBarco, 0.2f, 5.0f);
}

//=====================================
// Vertex Shader
//=====================================

const char* vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"\n"
"out vec3 Normal;\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"\n"
"void main()\n"
"{\n"
"    Normal = mat3(transpose(inverse(model))) * aNormal;\n"
"    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"}\n";

//=====================================
// Fragment Shader
//=====================================

const char* fragmentShaderSource =
"#version 330 core\n"
"\n"
"in vec3 Normal;\n"
"out vec4 FragColor;\n"
"\n"
"uniform vec4 materialColor;\n"
"\n"
"void main()\n"
"{\n"
"    // Direccion de la luz principal\n"
"    vec3 lightDirection = normalize(vec3(-1.5, 2.5, 0.5));\n"
"\n"
"    vec3 normal = normalize(Normal);\n"
"\n"
"    // Calcular la luz sobre cada superficie\n"
"    float diffuse = max(dot(normal, lightDirection), 0.0);\n"
"\n"
"    // Luz ambiental para generar contraste\n"
"    float ambient = 0.08;\n"
"\n"
"    // Intensidad de la luz directa\n"
"    float lighting = ambient + diffuse * 2.0;\n"
"\n"
"    // Limitar el brillo maximo\n"
"    lighting = min(lighting, 1.5);\n"
"\n"
"    // Aplicar iluminacion al modelo\n"
"    FragColor = vec4(materialColor.rgb * lighting, materialColor.a);\n"
"}\n";

//=====================================
// Estructuras
//=====================================

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
};

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

struct MeshGPU
{
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;

    unsigned int indexCount = 0;
};

//=====================================
// Convertir matriz Assimp a GLM
//=====================================

glm::mat4 convertirMatriz(const aiMatrix4x4& m)
{
    return glm::mat4(
        glm::vec4(m.a1, m.b1, m.c1, m.d1),
        glm::vec4(m.a2, m.b2, m.c2, m.d2),
        glm::vec4(m.a3, m.b3, m.c3, m.d3),
        glm::vec4(m.a4, m.b4, m.c4, m.d4)
    );
}

//=====================================
// Procesar malla
//=====================================

Mesh procesarMalla(
    aiMesh* mesh,
    const glm::mat4& transform)
{
    Mesh nuevaMalla;

    glm::mat3 normalMatrix =
        glm::transpose(glm::inverse(glm::mat3(transform)));

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex{};

        glm::vec3 position(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        );

        vertex.Position = glm::vec3(
            transform * glm::vec4(position, 1.0f)
        );

        if (mesh->HasNormals())
        {
            glm::vec3 normal(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            );

            vertex.Normal =
                glm::normalize(normalMatrix * normal);
        }
        else
        {
            vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        nuevaMalla.vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        const aiFace& face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            nuevaMalla.indices.push_back(face.mIndices[j]);
        }
    }

    return nuevaMalla;
}

//=====================================
// Procesar nodos
//=====================================

void procesarNodo(
    aiNode* node,
    const aiScene* scene,
    const glm::mat4& parentTransform,
    std::vector<Mesh>& meshes)
{
    glm::mat4 nodeTransform =
        parentTransform * convertirMatriz(node->mTransformation);

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        meshes.push_back(
            procesarMalla(mesh, nodeTransform)
        );
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        procesarNodo(
            node->mChildren[i],
            scene,
            nodeTransform,
            meshes
        );
    }
}

//=====================================
// Normalizar modelo
//=====================================

void normalizarModelo(std::vector<Mesh>& meshes)
{
    glm::vec3 minimum(
        std::numeric_limits<float>::max()
    );

    glm::vec3 maximum(
        std::numeric_limits<float>::lowest()
    );

    for (const Mesh& mesh : meshes)
    {
        for (const Vertex& vertex : mesh.vertices)
        {
            minimum = glm::min(minimum, vertex.Position);
            maximum = glm::max(maximum, vertex.Position);
        }
    }

    glm::vec3 center = (minimum + maximum) * 0.5f;
    glm::vec3 dimensions = maximum - minimum;

    float maxDimension = std::max(
        dimensions.x,
        std::max(dimensions.y, dimensions.z)
    );

    if (maxDimension <= 0.00001f)
        return;

    float scale = 2.0f / maxDimension;

    for (Mesh& mesh : meshes)
    {
        for (Vertex& vertex : mesh.vertices)
        {
            vertex.Position =
                (vertex.Position - center) * scale;
        }
    }
}

//=====================================
// Cargar modelo
//=====================================

std::vector<Mesh> cargarModelo(const std::string& ruta)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        ruta,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals
    );

    std::vector<Mesh> meshes;

    if (!scene || !scene->mRootNode)
    {
        std::cout << "Error al cargar modelo: "
            << importer.GetErrorString() << std::endl;

        return meshes;
    }

    procesarNodo(
        scene->mRootNode,
        scene,
        glm::mat4(1.0f),
        meshes
    );

    normalizarModelo(meshes);

    std::cout << "Modelo cargado: "
        << meshes.size() << " mallas" << std::endl;

    return meshes;
}

//=====================================
// Subir malla a GPU
//=====================================

MeshGPU subirMallaGPU(const Mesh& mesh)
{
    MeshGPU gpu;

    glGenVertexArrays(1, &gpu.VAO);
    glGenBuffers(1, &gpu.VBO);
    glGenBuffers(1, &gpu.EBO);

    glBindVertexArray(gpu.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, gpu.VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        mesh.vertices.size() * sizeof(Vertex),
        mesh.vertices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu.EBO);

    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        mesh.indices.size() * sizeof(unsigned int),
        mesh.indices.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, Position)
    );

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, Normal)
    );

    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    gpu.indexCount =
        static_cast<unsigned int>(mesh.indices.size());

    return gpu;
}

//=====================================
// Dibujar modelo
//=====================================

void dibujarModelo(
    const std::vector<MeshGPU>& modeloGPU,
    const glm::mat4& model,
    GLuint modelLoc,
    GLuint materialColorLoc)
{
    glUniformMatrix4fv(
        modelLoc,
        1,
        GL_FALSE,
        glm::value_ptr(model)
    );

    // Color morado del modelo

    glm::vec4 colorMorado(
        0.7f, 0.2f, 1.0f, 1.0f
    );

    glUniform4fv(
        materialColorLoc,
        1,
        glm::value_ptr(colorMorado)
    );

    for (const MeshGPU& mesh : modeloGPU)
    {
        glBindVertexArray(mesh.VAO);

        glDrawElements(
            GL_TRIANGLES,
            mesh.indexCount,
            GL_UNSIGNED_INT,
            0
        );
    }

    glBindVertexArray(0);
}

//=====================================
// Compilar shader
//=====================================

GLuint compilarShader(GLenum tipo, const char* codigo)
{
    GLuint shader = glCreateShader(tipo);

    glShaderSource(shader, 1, &codigo, NULL);
    glCompileShader(shader);

    GLint success = 0;

    glGetShaderiv(
        shader,
        GL_COMPILE_STATUS,
        &success
    );

    if (!success)
    {
        char infoLog[512];

        glGetShaderInfoLog(
            shader,
            512,
            NULL,
            infoLog
        );

        std::cout << "Error al compilar shader:\n"
            << infoLog << std::endl;

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

//=====================================
// MAIN
//=====================================

int main()
{
    if (!glfwInit())
    {
        std::cout << "Error al iniciar GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE
    );

    GLFWwindow* window = glfwCreateWindow(
        SCR_WIDTH,
        SCR_HEIGHT,
        "Going Merry - OpenGL",
        NULL,
        NULL
    );

    if (window == NULL)
    {
        std::cout << "Error al crear ventana" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Error al cargar GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    //=====================================
    // Cargar modelo
    //=====================================

    std::vector<Mesh> modelo = cargarModelo(
        R"(C:\Users\oband\Downloads\UMNG\Computación gráfica\Project1\Modelo\one_piece_-going_merry.glb)"
    );

    if (modelo.empty())
    {
        std::cout << "No se pudo cargar el modelo." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    //=====================================
    // Subir modelo a GPU
    //=====================================

    std::vector<MeshGPU> modeloGPU;

    for (const Mesh& mesh : modelo)
    {
        modeloGPU.push_back(subirMallaGPU(mesh));
    }

    //=====================================
    // Crear shaders
    //=====================================

    GLuint vertexShader = compilarShader(
        GL_VERTEX_SHADER,
        vertexShaderSource
    );

    GLuint fragmentShader = compilarShader(
        GL_FRAGMENT_SHADER,
        fragmentShaderSource
    );

    if (vertexShader == 0 || fragmentShader == 0)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    GLuint shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);

    GLint linkSuccess = 0;

    glGetProgramiv(
        shaderProgram,
        GL_LINK_STATUS,
        &linkSuccess
    );

    if (!linkSuccess)
    {
        char infoLog[512];

        glGetProgramInfoLog(
            shaderProgram,
            512,
            NULL,
            infoLog
        );

        std::cout << "Error al enlazar shaders:\n"
            << infoLog << std::endl;

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(shaderProgram);

        glfwDestroyWindow(window);
        glfwTerminate();

        return -1;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    //=====================================
    // Uniforms
    //=====================================

    GLuint modelLoc =
        glGetUniformLocation(shaderProgram, "model");

    GLuint viewLoc =
        glGetUniformLocation(shaderProgram, "view");

    GLuint projectionLoc =
        glGetUniformLocation(shaderProgram, "projection");

    GLuint materialColorLoc =
        glGetUniformLocation(shaderProgram, "materialColor");

    //=====================================
    // Camara
    //=====================================

    glm::vec3 cameraPos(0.0f, 0.0f, 4.0f);
    glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);

    glm::mat4 view = glm::lookAt(
        cameraPos,
        cameraTarget,
        cameraUp
    );

    //=====================================
    // Proyeccion
    //=====================================

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)SCR_WIDTH / (float)SCR_HEIGHT,
        0.1f,
        100.0f
    );

    // Tiempo y rotacion

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    float rotationSpeed = glm::radians(30.0f);

    // Loop principal

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();

        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, deltaTime);

        glClearColor(
            0.05f, 0.05f, 0.15f, 1.0f
        );

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT
        );

        glUseProgram(shaderProgram);

        float timeValue = glfwGetTime();

        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(
            model,
            posicionBarco
        );

        model = glm::rotate(
            model,
            timeValue * rotationSpeed,
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        // Aplicar escala al barco
        model = glm::scale(
            model,
            glm::vec3(escalaBarco)
        );

        glUniformMatrix4fv(
            viewLoc,
            1,
            GL_FALSE,
            glm::value_ptr(view)
        );

        glUniformMatrix4fv(
            projectionLoc,
            1,
            GL_FALSE,
            glm::value_ptr(projection)
        );

        dibujarModelo(
            modeloGPU,
            model,
            modelLoc,
            materialColorLoc
        );

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //=====================================
    // Liberar memoria
    //=====================================

    for (const MeshGPU& mesh : modeloGPU)
    {
        glDeleteVertexArrays(1, &mesh.VAO);
        glDeleteBuffers(1, &mesh.VBO);
        glDeleteBuffers(1, &mesh.EBO);
    }

    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
