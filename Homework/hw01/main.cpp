#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"

#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

//Kenny Yip Ky952 CS3113 HW 01

#include <iostream>
#include <string>
using namespace std;

SDL_Window* displayWindow;

//loads texture images (sourced from PDF)
GLuint LoadTexture(const char *filePath) {
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    if(image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);
    return retTexture;
}


int main(int argc, char *argv[])
{
    
    SDL_Init(SDL_INIT_VIDEO);

    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    

#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 360);
    
    ShaderProgram program, programT; //untextured, textured
    
    program.Load("vertex.glsl", "fragment.glsl");
    programT.Load("vertex_textured.glsl", "fragment_textured.glsl"); //textured
    
    GLuint mario = LoadTexture("Mario.png");
    GLuint mario8bit = LoadTexture("Mario8bit.png");
    GLuint marioGC = LoadTexture("MarioGC.png");
    
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    glUseProgram(program.programID);
    
//    glUseProgram(programT.programID);
    
    glClearColor(0.53f, 0.81f, 0.98f, 1.0f);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    

    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(program.programID);
        
        glEnableVertexAttribArray(program.positionAttribute);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        
        
        //UNTEXTURED
        
        //the two triangles create 1 ground (rectangle when put together)
        
        //ground 1/2 (top half)
        
        float vertices1[] = {-1.777f, 0.0f, -1.777f, -0.25f, 1.777f, 0.0f};
        
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -0.75f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
        
        program.SetModelMatrix(modelMatrix);
        program.SetColor(0.0f, 1.0f, 0.0f, 1.0f); //green
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices1);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        
        //ground 2/2 (bottom half)

        float vertices2[] = {-1.777f, -0.25f, 1.777f, -0.25f, 1.777f, 0.0f};
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -0.75f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));

        program.SetModelMatrix(modelMatrix);
        program.SetColor(0.0f, 1.0f, 0.0f, 1.0f); //green
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        
        //mountain triangle 1/3

        float vertices3[] = {0.0f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f};
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.75f, -0.625f, 0.0f)); //approx left side
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25f, 0.25f, 1.0f));

        program.SetModelMatrix(modelMatrix);
        program.SetColor(0.50f, 0.50f, 0.50f, 1.0f);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices3);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        
        //mountain triangle 2/3
        
        float vertices4[] = {0.0f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f};
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-.25f, -0.625f, 0.0f)); //approx right side
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25f, 0.25f, 1.0f));
        
        program.SetModelMatrix(modelMatrix);
        program.SetColor(0.50f, 0.50f, 0.50f, 1.0f);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices4);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        
        //mountain triangle 3/3
        
        float vertices5[] = {0.0f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f};
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5f, -0.5f, 0.0f)); // middle
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.5f, 1.0f));
        
        program.SetModelMatrix(modelMatrix);
        program.SetColor(0.40f, 0.40f, 0.40f, 1.0f);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices5);
        glDrawArrays(GL_TRIANGLES, 0, 3);


        
        
        
        //TEXTURED
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        glUseProgram(programT.programID);
        glEnableVertexAttribArray(programT.positionAttribute);
        glEnableVertexAttribArray(programT.texCoordAttribute);
        programT.SetProjectionMatrix(projectionMatrix);
        programT.SetViewMatrix(viewMatrix);
     
        
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.75f, 1.0f));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(1.5f, -0.5f, 0.0f));
        programT.SetModelMatrix(modelMatrix);
        
        
        //Mario
        
        float vertices1T[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(programT.positionAttribute, 2, GL_FLOAT, false, 0, vertices1T);
        glEnableVertexAttribArray(programT.positionAttribute);
        
        float texCoords1T[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(programT.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords1T);
        glEnableVertexAttribArray(programT.texCoordAttribute);
        
        glBindTexture(GL_TEXTURE_2D, mario);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
//        glDisableVertexAttribArray(program.positionAttribute);
//        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        //mario 8bit
        
        float vertices2T[] = { -1.5f, -0.5f, -0.5f, 0.5f, -1.5f, 0.5f, -0.5f, 0.5f, -1.5f, -0.5f, -0.5f, -0.5f };
        glVertexAttribPointer(programT.positionAttribute, 2, GL_FLOAT, false, 0, vertices2T);
        glEnableVertexAttribArray(programT.positionAttribute);
        
        float texCoords2T[] = { 0.0, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0 };
        glVertexAttribPointer(programT.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords2T);
        glEnableVertexAttribArray(programT.texCoordAttribute);
        
        glBindTexture(GL_TEXTURE_2D, mario8bit);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        
        //marioGC
        
        float vertices3T[] = { 1.5f, -0.5f, 0.5f, 0.5f, 1.5f, 0.5f, 0.5f, 0.5f, 1.5f, -0.5f, 0.5f, -0.5f };
        glVertexAttribPointer(programT.positionAttribute, 2, GL_FLOAT, false, 0, vertices3T);
        glEnableVertexAttribArray(programT.positionAttribute);
        
        float texCoords3T[] = { 0.0, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0 };
        glVertexAttribPointer(programT.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords3T);
        glEnableVertexAttribArray(programT.texCoordAttribute);
        
        
        
        glBindTexture(GL_TEXTURE_2D, marioGC);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(programT.positionAttribute);
        glDisableVertexAttribArray(programT.texCoordAttribute);
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
