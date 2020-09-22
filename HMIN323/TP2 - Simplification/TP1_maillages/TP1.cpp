// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <float.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

#include <algorithm>

#include "Mesh.h"

/******************************************************************************/
/***************           Fonctions à completer         **********************/
/******************************************************************************/



void compute_triangle_normals (const std::vector<glm::vec3> & vertices,
                               const std::vector<std::vector<unsigned short> > & triangles,
                               std::vector<glm::vec3> & triangle_normals){

    for (int i = 0; i < triangles.size(); i++)
    {
        std::vector<unsigned short> indexes = triangles[i];

        glm::vec3 v0 = vertices[indexes[0]];
        glm::vec3 v1 = vertices[indexes[1]];
        glm::vec3 v2 = vertices[indexes[2]];

        triangle_normals[i] = glm::normalize(glm::cross(v1 - v0, v2 - v0));
    }
}

void collect_one_ring (const std::vector<glm::vec3> & vertices,
                       const std::vector<std::vector<unsigned short> > & triangles,
                       std::vector<std::vector<unsigned short> > & one_ring) {

    for (int i = 0; i < triangles.size(); i++)
    {
        std::vector<unsigned short> triangle = triangles[i];

        unsigned short i0 = triangle[0];
        unsigned short i1 = triangle[1];
        unsigned short i2 = triangle[2];

        one_ring[i0].push_back(i1);
        one_ring[i0].push_back(i2);

        one_ring[i1].push_back(i2);
        one_ring[i1].push_back(i0);

        one_ring[i2].push_back(i0);
        one_ring[i2].push_back(i1);
    }
}


void compute_vertex_valences (const std::vector<glm::vec3> & vertices,
                              const std::vector<std::vector<unsigned short> > & triangles,
                              std::vector<unsigned int> & valences ) {

    std::vector<std::vector<unsigned short> > one_ring(vertices.size());
    collect_one_ring(vertices, triangles, one_ring);
    for (int i = 0; i < vertices.size(); i++)
    {
        valences[i] = one_ring[i].size();
    }

}

float vec3_angle(glm::vec3 v0, glm::vec3 v1)
{
    glm::vec3 u0 = normalize(v0);
    glm::vec3 u1 = normalize(v1);
    return glm::acos(glm::dot(u0, u1));
}

void compute_smooth_vertex_normals (const std::vector<glm::vec3> & vertices,
                                    const std::vector<std::vector<unsigned short> > & triangles,
                                    unsigned int weight_type, //0 uniforme, 1 aire des triangles, 2 angle du triangle
                                    std::vector<glm::vec3> & vertex_normals){

    vertex_normals.clear();

    switch (weight_type) {
        case 0:
        {
            vertex_normals.resize(vertices.size(), glm::vec3(0.0));

            // Uniform weights
            std::vector<glm::vec3> triangle_normals(triangles.size());

            compute_triangle_normals(vertices, triangles, triangle_normals);

            for (int i = 0; i < triangles.size(); i++)
            {
                std::vector<unsigned short> indexes = triangles[i];
                glm::vec3 normal = triangle_normals[i];
                vertex_normals[indexes[0]] += normal;
                vertex_normals[indexes[1]] += normal;
                vertex_normals[indexes[2]] += normal;
            }

            for (int i = 0; i < vertex_normals.size(); i++)
            {
                vertex_normals[i] = glm::normalize(vertex_normals[i]);
            }
            break;
        }
        case 1:
        {
            vertex_normals.resize(vertices.size(), glm::vec3(0.0));
            std::vector<std::vector<unsigned short> > one_ring(vertices.size());
            collect_one_ring(vertices, triangles, one_ring);

            for (int i = 0; i < vertices.size(); i++)
            {
                glm::vec3 v0 = vertices[i];
                std::vector<unsigned short> neighbours = one_ring[i];

                for (int j = 0; j < neighbours.size(); j += 2)
                {
                    glm::vec3 v1 = vertices[neighbours[j]];
                    glm::vec3 v2 = vertices[neighbours[j + 1]];
                    vertex_normals[i] += glm::cross(v1 - v0, v2 - v0);
                }

                vertex_normals[i] = normalize(vertex_normals[i]);
            }
        }
        case 2:
        {
            vertex_normals.resize(vertices.size(), glm::vec3(0.0));
            std::vector<std::vector<unsigned short> > one_ring(vertices.size());
            collect_one_ring(vertices, triangles, one_ring);

            for (int i = 0; i < vertices.size(); i++)
            {
                glm::vec3 v0 = vertices[i];
                std::vector<unsigned short> neighbours = one_ring[i];

                for (int j = 0; j < neighbours.size(); j += 2)
                {
                    glm::vec3 v1 = vertices[neighbours[j]];
                    glm::vec3 v2 = vertices[neighbours[j + 1]];
                    vertex_normals[i] += vec3_angle(v1 - v0, v2 - v0) * normalize(glm::cross(v1 - v0, v2 - v0));
                }

                vertex_normals[i] = normalize(vertex_normals[i]);
            }
        }
        default:
        {
            vertex_normals.resize(vertices.size(), glm::vec3(1.));
        }
    }

}

/*******************************************************************************/

int main( void )
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "TP2 - Maillages", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "vertex_shader.glsl", "fragment_shader.glsl" );

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

    // Load the texture
    GLuint Texture = loadDDS("uvmap.DDS");

    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

    std::vector<unsigned short> indices; //Triangles concaténés dans une liste
    std::vector<std::vector<unsigned short> > triangles;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;

    //Chargement du fichier de maillage
    std::string filename("suzanne.off");
    loadOFF(filename, indexed_vertices, indices, triangles );
    indexed_uvs.resize(indexed_vertices.size(), glm::vec2(1.)); //List vide de UV

    //****************************************************************/
    //Calculer les normales par sommet
    //int weight = 2;
    //compute_smooth_vertex_normals(indexed_vertices, triangles, weight, indexed_normals);

	Engine::Mesh suzanneMesh(indexed_vertices, indexed_normals, indexed_uvs, indices);
    suzanneMesh.ComputeNormals(Engine::NormalWeightType::PerAngle);

    // A faire : completer la fonction compute_vertex_valences pour calculer les valences
    //***********************************************//
    //std::vector<unsigned int> valences(suzanneMesh._vertices.size());
    //compute_vertex_valences(suzanneMesh._vertices, suzanneMesh._triangles, valences );

    // A faire : normaliser les valences pour avoir une valeur flotante entre 0. et 1. dans valence_field
    //***********************************************//
    //std::vector<float> valence_field(valences.size());
    //unsigned int max_valence = *std::max_element(valences.begin(), valences.end());

    //for (int i = 0; i < valences.size(); i++)
    //{
    //    valence_field[i] = ((float) valences[i]) / max_valence;
    //}
    //****************************************************************/

    // Load it into a VBO

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, suzanneMesh._vertices.size() * sizeof(glm::vec3), &suzanneMesh._vertices[0], GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, suzanneMesh._texCoords.size() * sizeof(glm::vec2), &suzanneMesh._texCoords[0], GL_STATIC_DRAW);

    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, suzanneMesh._normals.size() * sizeof(glm::vec3), &suzanneMesh._normals[0], GL_STATIC_DRAW);


    // A faire : créer le buffer de valence, un buffer de float valence_field
    //***********************************************//
    //GLuint valencebuffer;
    //glGenBuffers(1, &valencebuffer);
    //glBindBuffer(GL_ARRAY_BUFFER, valencebuffer);
    //glBufferData(GL_ARRAY_BUFFER, valence_field.size() * sizeof(float), &valence_field[0], GL_STATIC_DRAW);
    //***********************************************//


    // Generate a buffer for the indices as well
    GLuint elementbuffer;
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, suzanneMesh._indices.size() * sizeof(unsigned short), &suzanneMesh._indices[0] , GL_STATIC_DRAW);

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    bool hasSimplified = false;

    do{

        // Measure speed
        double currentTime = glfwGetTime();
        nbFrames++;
        if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1sec ago
            // printf and reset
            printf("%f ms/frame\n", 1000.0/double(nbFrames));
            nbFrames = 0;
            lastTime += 1.0;
        }

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programID);

        // Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
        // Camera matrix
        glm::mat4 ViewMatrix       = glm::lookAt(
                    glm::vec3(0,0,3), // Camera is at (4,3,3), in World Space
                    glm::vec3(0,0,0), // and looks at the origin
                    glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                    );
        // Model matrix : an identity matrix (model will be at the origin)
        glm::mat4 ModelMatrix      = glm::mat4(1.0f);
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

        glm::vec3 lightPos = glm::vec3(4,4,4);
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(TextureID, 0);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                    0,                  // attribute
                    3,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    (void*)0            // array buffer offset
                    );

        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
                    1,                                // attribute
                    2,                                // size
                    GL_FLOAT,                         // type
                    GL_FALSE,                         // normalized?
                    0,                                // stride
                    (void*)0                          // array buffer offset
                    );

        // 3rd attribute buffer : normals
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glVertexAttribPointer(
                    2,                                // attribute
                    3,                                // size
                    GL_FLOAT,                         // type
                    GL_FALSE,                         // normalized?
                    0,                                // stride
                    (void*)0                          // array buffer offset
                    );


        // A faire : créer le 4eme attribute buffer : valence_field
        //***********************************************//
        //glEnableVertexAttribArray(3);
        //glBindBuffer(GL_ARRAY_BUFFER, valencebuffer);
        //glVertexAttribPointer(
        //            3,                                // attribute
        //            1,                                // size
        //            GL_FLOAT,                         // type
        //            GL_FALSE,                         // normalized?
        //            0,                                // stride
        //            (void*)0                          // array buffer offset
        //            );
        //***********************************************//

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

        // Draw the triangles !
        glDrawElements(
                    GL_TRIANGLES,      // mode
                    suzanneMesh._indices.size(),    // count
                    GL_UNSIGNED_SHORT,   // type
                    (void*)0           // element array buffer offset
                    );

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        // TODO: Factorize mesh buffer updates
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && hasSimplified == false)
        {
            suzanneMesh.Simplify(16);
            hasSimplified = true;

            // Simplification can only reduce size
            // We then use glBufferSubData to avoid reallocating a new buffer

            // Update Buffers
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            glBufferSubData(GL_ARRAY_BUFFER, 0, suzanneMesh._vertices.size() * sizeof(glm::vec3), suzanneMesh._vertices.data());

            glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
            glBufferSubData(GL_ARRAY_BUFFER, 0, suzanneMesh._texCoords.size() * sizeof(glm::vec2), suzanneMesh._texCoords.data());

            glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
            glBufferSubData(GL_ARRAY_BUFFER, 0, suzanneMesh._normals.size() * sizeof(glm::vec3), suzanneMesh._normals.data());

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, suzanneMesh._indices.size() * sizeof(unsigned short), suzanneMesh._indices.data());
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && hasSimplified == false)
        {
            suzanneMesh.AdaptiveSimplify(2);
            suzanneMesh.ComputeNormals(Engine::NormalWeightType::PerAngle);
            hasSimplified = true;

            // Simplification can only reduce size
            // We then use glBufferSubData to avoid reallocating a new buffer

            // Update Buffers
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            glBufferSubData(GL_ARRAY_BUFFER, 0, suzanneMesh._vertices.size() * sizeof(glm::vec3), suzanneMesh._vertices.data());

            glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
            glBufferSubData(GL_ARRAY_BUFFER, 0, suzanneMesh._texCoords.size() * sizeof(glm::vec2), suzanneMesh._texCoords.data());

            glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
            glBufferSubData(GL_ARRAY_BUFFER, 0, suzanneMesh._normals.size() * sizeof(glm::vec3), suzanneMesh._normals.data());

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, suzanneMesh._indices.size() * sizeof(unsigned short), suzanneMesh._indices.data());
        }

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteBuffers(1, &elementbuffer);

    // A faire : supprimer le buffer de valence_field
    //***********************************************//
	//glDeleteBuffers(1, &valencebuffer);
    //***********************************************//

    glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
