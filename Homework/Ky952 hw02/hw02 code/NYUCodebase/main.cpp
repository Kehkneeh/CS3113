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

//Kenny Yip Ky952 CS3113 HW 02 - Pong

#include <iostream>
#include <string>
using namespace std;


SDL_Window* displayWindow;

/* reference (I will try to use this over the break)
class Entity {
    public:
        void Draw(ShaderProgram &p);
        float x;
        float y;
        float rotation;
        int textureID;
    
        float width;
        float height;
        float velocity;
        float direction_x;
        float direction_y;
};
*/



bool collision(int firstX, int firstY, int secondX, int secondY){
    
    float p1 = abs(firstX - secondX) - ((0.1 + 0.1)/2); //width difference
    float p2 = abs(firstY - secondY) - ((0.1 + 0.5)/2); //height difference
    return p1 <= 0 && p2 <= 0;
}




int main(int argc, char *argv[])
{
    
    SDL_Init(SDL_INIT_VIDEO);

    displayWindow = SDL_CreateWindow("Ky952 Pong Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    

#ifdef _WINDOWS
    glewInit();
#endif
    
    //x, y, width, height
    glViewport(0, 0, 640, 360);
    
    ShaderProgram program; //untextured, textured
    
    program.Load("vertex.glsl", "fragment.glsl");

    
    //sourced from slide 4 (for drawing polygons)

    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    
    glm::mat4 modelMatrix = glm::mat4(1.0f);            //identity matrix
    
    
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    
    
    //left, right, bottom, top, near, far
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f); //creates the scaled bounds
    glUseProgram(program.programID); //
    
    
    glClearColor(0.53f, 0.81f, 0.98f, 1.0f);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    

    SDL_Event event;
    bool done = false;
    
    float lastFrameTicks = 0.0f; //sourced from slide 6
    
    
    //pong Angle
    float pongAngle = 0.0f;
    
    //pong displacements (positions) & directions
    float pongXDisplacement = 0.0f;
    float pongYDisplacement = 0.0f;
    
    int pongXDirection = 1;    //1 to keep going, -1 to reverse direction
    int pongYDirection = 1;

    
    
    
    //THE PONG GOES TOWARDS THE LOSER EACH TIME
    
    //paddle displacements (positions)
    float rightPaddleDisplacement = 0.0f;
    float leftPaddleDisplacement = 0.0f;
    
    
     const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    
    //game loop
    while (!done) {
        
        //sourced from slide 6
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        
        pongAngle += elapsed;
        
        //hits top or bottom border, reverse y direction
        if (pongYDisplacement >= 1.0 || pongYDisplacement <= -1.0) {
            pongYDirection *= -1;
        }
        
        
        //hits left paddle
        /*
         Hello professor, I've been having difficulties with the collision detection function using the formula discussed in class.
         Perhaps the scaling + translating is affecting my numbers and I can't seem to figure out which ones they are to get the
         function to work.
         
         I've been having trouble getting the collision detection to work.
         I decided to go with a different method (logic seems to check?) where I confirm the collision
         by checking the range of the x and y positions/displacements of the pong with respect to the paddles.
         
         */
        
        
        //using this until I figure out the box-box collision detection (the 1.4 came from 1.5 displacement - .1 box width)
        if(pongXDisplacement <= -1.40 &&
           pongYDisplacement <= leftPaddleDisplacement &&
           pongYDisplacement >= leftPaddleDisplacement - 0.5f){
            
//        if (collision(pongXDisplacement, pongYDisplacement, -2.0, leftPaddleDisplacement)){
            pongXDirection *= -1;
        }

        //hits right paddle
        
        if (pongXDisplacement >= 1.40 &&
            pongYDisplacement <= rightPaddleDisplacement &&
            pongYDisplacement >= rightPaddleDisplacement - 0.5f){
            
//        if (collision(pongXDisplacement, pongYDisplacement, 2.0, rightPaddleDisplacement)){
            pongXDirection *= -1;
        }
        
        
        //must reset the variables
        
        //hits right border (left wins)
        if (pongXDisplacement >= 1.777){
            pongXDisplacement = 1.0f;
            pongYDirection = 1.0f;
            
            //reset
            rightPaddleDisplacement = 0.0f;
            leftPaddleDisplacement = 0.0f;
            
            pongAngle = 0.0f;
            
            pongXDisplacement = 0.0f;
            pongYDisplacement = 0.0f;
            
            
            
        //hits left border (right wins)
        } else if (pongXDisplacement <= -1.777){
            pongXDisplacement = -1.0f;
            pongYDirection = -1.0f;
            
            //reset
            rightPaddleDisplacement = 0.0f;
            leftPaddleDisplacement = 0.0f;
            
            pongAngle = 0.0f;
            
            pongXDisplacement = 0.0f;
            pongYDisplacement = 0.0f;
            
        }
        
        //pong travelling
        pongXDisplacement += 0.5f * elapsed * pongXDirection;
        pongYDisplacement += 0.5f * elapsed * pongYDirection;
        
        
        
        /* for reference from slides
         y_position += elapsed * distance_to_travel_in_one_second;
        
         position += direction_vector * elapsed * units_a_second;
         
         position.x += direction.x * elapsed * units_a_second;
         
         position.y += direction.y * elapsed * units_a_second;
         
        */
        
        
        
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        
        
        // up/down keys for right paddle
        if(keys[SDL_SCANCODE_UP] && rightPaddleDisplacement < 0.75f) {
            rightPaddleDisplacement += 1.0 * elapsed;
            
        }
        
        if (keys[SDL_SCANCODE_DOWN] && rightPaddleDisplacement > -0.75f){
            rightPaddleDisplacement -= 1.0 * elapsed;
        }
        
        // up/down keys for left paddle
        if (keys[SDL_SCANCODE_W] && leftPaddleDisplacement < 0.75f){
            leftPaddleDisplacement += 1.0 * elapsed;
            
        }
        if (keys[SDL_SCANCODE_S] && leftPaddleDisplacement > -0.75f){
            leftPaddleDisplacement -= 1.0 * elapsed;
            
        }
        
        //drawing in game loop
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(program.programID);
        
        glEnableVertexAttribArray(program.positionAttribute);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        
        
        //Rectangles for game

        //Pong
        float pongVertices[] = {-0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f};
        modelMatrix = glm::mat4(1.0f);
        
        modelMatrix = glm::translate(modelMatrix, glm::vec3(pongXDisplacement, pongYDisplacement, 1.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.10f, 0.10f, 0.10f)); //scales the pong ball
//        modelMatrix = glm::rotate(modelMatrix, pongAngle, glm::vec3(pongXDisplacement, pongYDisplacement, 0.0f));
        
        
        program.SetModelMatrix(modelMatrix);
        program.SetColor(1.0f, 0.0f, 0.0f, 1.0f); //red
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, pongVertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        
        
        
        
        //Paddle Left (left side to detect collision)
        
        float pLeftVertices[] = {-0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f};
        modelMatrix = glm::mat4(1.0f);

        modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.50f, leftPaddleDisplacement, 1.0f));
        
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.10f, 0.5f, 1.0f));
        
        
        program.SetModelMatrix(modelMatrix);
        program.SetColor(0.0f, 1.0f, 0.0f, 1.0f); //green
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, pLeftVertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        
        
        
        //Paddle Right (right side to detect collision)
        
        float pRightVertices[] = {-0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f};
        modelMatrix = glm::mat4(1.0f);
        
        modelMatrix = glm::translate(modelMatrix, glm::vec3(1.50f, rightPaddleDisplacement, 1.0f));
        
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.10f, 0.5f, 1.0f));
        
        
        program.SetModelMatrix(modelMatrix);
        program.SetColor(1.0f, 1.0f, 0.0f, 1.0f); //yellow
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, pRightVertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        
        
        
        
        
        
        
        //for reference
        
        //index size, type, normalized, stride, *pointer (array/ vertex data)
        //glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertice);
        
        //mode, first, count (number of points)
        //glDrawArrays(GL_TRIANGLES, 0, 6);
    
        
        
        glDisableVertexAttribArray(program.positionAttribute);

        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
