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

#define TILE_SIZE 0.05f
#define GRAVITY -2.0f
#define SCREEN_WIDTH  560.0f
#define SCREEN_HEIGHT  560.0f


#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6


//Kenny Yip Ky952 CS3113 HW 03 - Space Invaders

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
using namespace std;

/*
 
 
 The mario player stands on the ground blocks, question blocks, and the brick blocks
 If mario collides with question block from underneath, the block will change sprite
 
 
 
 */


//L inear int ERP olation

//velocity, deceleration, time
float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}



//globals
//FlareMap map;
enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL};
GameMode mode;


SDL_Window* displayWindow;
ShaderProgram programT; //textured

glm::mat4 modelMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);
glm::mat4 projectionMatrix;



//Sprites - slide 9
class SheetSprite {
public:
    SheetSprite(){}
    
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size)
    : textureID(textureID), u(u), v(v), width(width), height(height), size(size){}
    
    void Draw(ShaderProgram &p) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        GLfloat texCoords[] = {
            u, v+height,
            u+width, v,
            u, v,
            u+width, v,
            u, v+height,
            u+width, v+height
        };
        
        float aspect = width / height;
        
        float vertices[] = {
            -0.5f * size * aspect, -0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, 0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, -0.5f * size ,
            0.5f * size * aspect, -0.5f * size};
        
        
        // draw our arrays
        glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(p.positionAttribute);
        
        glVertexAttribPointer(p.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(p.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(p.positionAttribute);
        glDisableVertexAttribArray(p.texCoordAttribute);
    }
    

    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
    float size;
    
};






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







void DrawText(ShaderProgram &p, int fontTexture, std::string text, float size, float spacing) { //slide 8
    
    glUseProgram(p.programID);
    
    
    float character_size = 1.0/16.0f;
    
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    
    for(int i=0; i < text.size(); i++) {
        
        int spriteIndex = (int)text[i];
        float texture_x = (float)(spriteIndex % 16) / 16.0f;
        float texture_y = (float)(spriteIndex / 16) / 16.0f;
        
        vertexData.insert(vertexData.end(), {
            ((size+spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + character_size,
            texture_x + character_size, texture_y,
            texture_x + character_size, texture_y + character_size,
            texture_x + character_size, texture_y,
            texture_x, texture_y + character_size,
            
        }); }
    
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    
    glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(p.positionAttribute);
    
    glVertexAttribPointer(p.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(p.texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0 , (int) text.size() * 6);
    
    glDisableVertexAttribArray(p.positionAttribute);
    glDisableVertexAttribArray(p.texCoordAttribute);
    
    
}




struct UniformedSheetSprite{
    
    UniformedSheetSprite(){}
    
    UniformedSheetSprite(GLuint texture, int x, int y) :
    texture(texture), spriteCountX(x), spriteCountY(y) {}
    
    GLuint texture;
    int spriteCountX;
    int spriteCountY;
    
};





void DrawSpriteSheetSprite(ShaderProgram &p, int index, const UniformedSheetSprite& sheet) {
    
    glBindTexture(GL_TEXTURE_2D, sheet.texture);
    
    float u = (float)(((int)index) % sheet.spriteCountX) / (float) sheet.spriteCountX;
    float v = (float)(((int)index) / sheet.spriteCountY) / (float) sheet.spriteCountY;
    
    float spriteWidth = 1.0/(float)sheet.spriteCountX;
    float spriteHeight = 1.0/(float)sheet.spriteCountY;
    
    
    
    float texCoords[] = {
        u, v+spriteHeight,
        u+spriteWidth, v,
        u, v,
        u+spriteWidth, v,
        u, v+spriteHeight,
        u+spriteWidth, v+spriteHeight};
    
    float vertices[] = { -TILE_SIZE, -TILE_SIZE, TILE_SIZE,  TILE_SIZE, -TILE_SIZE,  TILE_SIZE,
                          TILE_SIZE, TILE_SIZE, -TILE_SIZE, -TILE_SIZE, TILE_SIZE, -TILE_SIZE};
    
    
    // draw this data
    glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(p.positionAttribute);
    
    glVertexAttribPointer(p.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(p.texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(p.positionAttribute);
    glDisableVertexAttribArray(p.texCoordAttribute);
    
    
}










enum EntityType {ENTITY_PLAYER, ENTITY_ENEMY, ENTITY_GROUND, ENTITY_BLOCK, ENTITY_QUESTION, ENTITY_COIN};

class Entity {
    
public:
    
    //Without Sprite Sheet
    Entity(float xPosition, float yPosition, float width, float height, float xVelocity, float yVelocity, int textID, EntityType entityType)
    : xPosition(xPosition), yPosition(yPosition), width(width), height(height), xVelocity(xVelocity), yVelocity(yVelocity), textureID(textID), entityType(entityType) {}
    
    //With Sprite Sheet
    Entity(float xPosition, float yPosition, float width, float height, float xVelocity, float yVelocity, int textID, const SheetSprite& sprite, EntityType entityType)
    : xPosition(xPosition), yPosition(yPosition), width(width), height(height), xVelocity(xVelocity), yVelocity(yVelocity), textureID(textID), currSprite(sprite), entityType(entityType)  {}
    
    //With Sprite Sheet
    Entity(float xPosition, float yPosition, float width, float height, float xVelocity, float yVelocity, int textID, const UniformedSheetSprite& sprite, EntityType entityType)
    : xPosition(xPosition), yPosition(yPosition), width(width), height(height), xVelocity(xVelocity), yVelocity(yVelocity), textureID(textID), currUniformedSprite(sprite), entityType(entityType)  {}
    
    
    void setSpriteNum(int index){
        spriteNum = index;
    }
    
    void setCurrUniformedSprite(const UniformedSheetSprite& sheet){
        currUniformedSprite = sheet;
    }
    
    void Draw(ShaderProgram &p, bool uniformed){
        if (uniformed){
            DrawSpriteSheetSprite(p, spriteNum, currUniformedSprite);
        }else {
            currSprite.Draw(p);
        }
    }
    
    
    void Render(ShaderProgram& p, Entity& player){
        
        projectionMatrix = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
        
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(xPosition, yPosition, 1.0f));
        
        viewMatrix = glm::mat4(1.0f);
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-1.0f*player.xPosition, -1.0f*player.yPosition, 0.0f));
        
        p.SetModelMatrix(modelMatrix);
        p.SetViewMatrix(viewMatrix);
        
        Draw(p, true);
        
    }

    void updateX(float elapsed){
        
        if (entityType == ENTITY_PLAYER){
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            
            if(keys[SDL_SCANCODE_LEFT]){
                xAcceleration = -1.0f;
            }else if(keys[SDL_SCANCODE_RIGHT]){
                xAcceleration = 1.0f;
            }else{
                xAcceleration = 0.0f;
            }
            
            //UPDATE player xMovements
            xVelocity = lerp(xVelocity, 0.0f, elapsed);
            xVelocity += xAcceleration * elapsed;
            xPosition += xVelocity * elapsed;
        }
    }
    
    
    void updateY(float elapsed){
        if (entityType == ENTITY_PLAYER){
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            
            if(keys[SDL_SCANCODE_UP] && collidedBottom) {
                collidedBottom = false;
                setSpriteNum(5); //jump
                yVelocity = 1.5f;
            }
            
            yAcceleration = GRAVITY;
            yVelocity += yAcceleration * elapsed;
            yPosition += yVelocity * elapsed;
            
        }
    }
    
    
    void collideX(Entity& entity){
        //no collision on all four sides
        if (xPosition + width/2.0f < entity.xPosition - entity.width/2.0f ||
            xPosition - width/2.0f > entity.xPosition + entity.width/2.0f ||
            yPosition + height/2.0f < entity.yPosition - entity.height/2.0f ||
            yPosition - height/2.0f > entity.yPosition + entity.height/2.0f){
            return;
        }
        
                                    //distance                        - radius1     - radius2
        float xPenetration = fabs( fabs(xPosition - entity.xPosition) - width/2.0f - entity.width/2.0f);
        
        
        //right collision
        if (xPosition > entity.xPosition){
            xPosition += (xPenetration + 0.00001f); //very small amount
            collidedRight = true;
            
        } else{
            //left collision
            
            xPosition -= (xPenetration - 0.00001f);
            collidedLeft = true;
        }
        
        xVelocity = 0.0f; //reset because of collision
    }
    
    
   void collideY(Entity& entity){
        //no collision on all four sides
        if (xPosition + width/2.0f < entity.xPosition - entity.width/2.0f ||
            xPosition - width/2.0f > entity.xPosition + entity.width/2.0f ||
            yPosition + height/2.0f < entity.yPosition - entity.height/2.0f ||
            yPosition - height/2.0f > entity.yPosition + entity.height/2.0f){
            return;
        }
        
                                    //distance                        - radius1     - radius2
        float yPenetration = fabs( fabs(yPosition - entity.yPosition) - height/2.0f - entity.height/2.0f);
        
        
        //top collision
        if (yPosition > entity.yPosition){
            yPosition += (yPenetration + 0.00001f);
            collidedBottom = true;
            setSpriteNum(0);
            
        } else{
        //bottom collision
            yPosition -= (yPenetration - 0.00001f);
            collidedTop = true;
            if (entity.entityType == ENTITY_QUESTION){
                entity.setSpriteNum(3);
            }
        }
        
        yVelocity = 0.0f;  //reset because of collision
    }
    
    
    void resetCollide(){
        collidedTop = false;
//        collidedBottom = false;
        collidedLeft =  false;
        collidedRight = false;
    }
    
    float xPosition;
    float yPosition;
    float rotation;
    int textureID;
    
    float width;
    float height;
    
    float xVelocity;
    float yVelocity;
    
    float xAcceleration;
    float yAcceleration;
    
    bool isStatic;
    
    EntityType entityType;
    
    bool collidedTop;
    bool collidedBottom = true;
    bool collidedLeft;
    bool collidedRight;
    
    SheetSprite currSprite;
    int spriteNum = 0;
    
    UniformedSheetSprite currUniformedSprite;
    UniformedSheetSprite faceRight;
    UniformedSheetSprite faceLeft;
    
};




//box box collision
bool Collision(const Entity& object1, const Entity& object2){
    float checkX = abs(object1.xPosition - object2.xPosition) - (object1.width + object2.width)/2;
    float checkY = abs(object1.yPosition - object2.yPosition) - (object1.height + object2.height)/2;
    return (checkX < 0) && (checkY < 0);
}




//shader program, screen height, screen width
void Setup(ShaderProgram &p){
    SDL_Init(SDL_INIT_VIDEO);
    
    displayWindow = SDL_CreateWindow("Ky952 Super Mario Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
    
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    projectionMatrix = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    p.Load("vertex_textured.glsl", "fragment_textured.glsl"); //textured
    p.SetProjectionMatrix(projectionMatrix);
    p.SetViewMatrix(viewMatrix);
    
    //93,148,251
    glClearColor(0.36f, 0.58f, 0.98f, 1.0f); //blue sky
    glClearColor(0.474f, 0.455f, 1.0f, 1.0f); //purple sky
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}




int main(int argc, char *argv[])
{
    
    
    Setup(programT);
    
    
    
    //I have trouble with RESOURCE_FOLDER working properly
    //    GLuint fontTexture = LoadTexture(RESOURCE_FOLDER"font1.png");
    
    
    //LOADING TEXTURES
    GLuint fontTexture = LoadTexture("font1.png");
    
    GLuint marioPlayerLeft1Texture = LoadTexture("marioPlayerLeft1.png"); //mario level 1 (little)
    UniformedSheetSprite marioPlayerLeft1 = UniformedSheetSprite(marioPlayerLeft1Texture, 21, 1);
    
    GLuint marioPlayerLeft2Texture = LoadTexture("marioPlayerLeft2.png"); //mario level 2 (big)
    UniformedSheetSprite marioPlayerLeft2 = UniformedSheetSprite(marioPlayerLeft2Texture, 21, 1);
    
    GLuint blockTilesTexture = LoadTexture("blockTiles.png");
    UniformedSheetSprite blockTiles = UniformedSheetSprite(blockTilesTexture, 28, 4);
    
    GLuint titleTexture = LoadTexture("SMBLogo.png");
    

    
    //CREATING ENTITIES
    
    //Entity(float posX, float posY, float width, float height, float xVelocity, float yVelocity, int textID, const UniformedSheetSprite& sprite)
    Entity player = Entity(0.0f, -4.0f*TILE_SIZE, TILE_SIZE, TILE_SIZE, 0.0f, 0.0f, 0, marioPlayerLeft1, ENTITY_PLAYER);
    
    vector<Entity> groundBlocks; //blocks used for standing on
    
    //Entity(float posX, float posY, float width, float height, float xVelocity, float yVelocity, int textID, const UniformedSheetSprite& sprite)
    for (int i = 1; i <= 8; i++){
        for (int j = 0; j < 150; j++ ){
            groundBlocks.push_back(Entity(j*(TILE_SIZE + TILE_SIZE),  - 5.0f*TILE_SIZE -i*(TILE_SIZE + TILE_SIZE) - TILE_SIZE/2.0f, TILE_SIZE, TILE_SIZE, 0.0f, 0.0f, 0, blockTiles, ENTITY_GROUND));
        }
    }
    
    vector<Entity> brickBlocks; //bricks that can be smashed
    for (int i = 0; i < 3; i++){
        brickBlocks.push_back(Entity(8.5f*TILE_SIZE + 2.0f*i*(TILE_SIZE + TILE_SIZE), - 3.0f*TILE_SIZE, TILE_SIZE, TILE_SIZE, 0.0f, 0.0f, 0, blockTiles, ENTITY_BLOCK));
        brickBlocks[brickBlocks.size()-1].setSpriteNum(1);
        
    }
    
    vector<Entity> questionBlocks; //bricks that can contain coin or power ups
    
    for (int i = 0; i < 2; i++){
        questionBlocks.push_back(Entity(2.0f*i*(TILE_SIZE + TILE_SIZE) + 10.5f*TILE_SIZE,  - 3.0f*TILE_SIZE, TILE_SIZE, TILE_SIZE, 0.0f, 0.0f, 0, blockTiles, ENTITY_QUESTION));
        questionBlocks[questionBlocks.size()-1].setSpriteNum(24);
    }

    
    mode = STATE_MAIN_MENU;
//    mode = STATE_GAME_LEVEL;

    
    
    //keyboard stuff
    SDL_Event event;
    bool done = false;
    
    
    float lastFrameTicks = 0.0f; //sourced from slide 6
    float accumulator = 0.0f; //sourced from slide 9
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    while (!done) {
        //quitting
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        
        
        //sourced from slide 6
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        elapsed += accumulator;
        if(elapsed < FIXED_TIMESTEP) {
            accumulator = elapsed;
            continue;
        }
        
        
        //TITLE SCREEN
        if (mode == STATE_MAIN_MENU){
        
            if(keys[SDL_SCANCODE_SPACE]) {
                mode = STATE_GAME_LEVEL;
                continue;
            }

            //drawing in game loop
            glClear(GL_COLOR_BUFFER_BIT);
            
            glEnableVertexAttribArray(programT.positionAttribute);

            //Super Mario Bros Title
            float vertices1T[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
            glVertexAttribPointer(programT.positionAttribute, 2, GL_FLOAT, false, 0, vertices1T);
            glEnableVertexAttribArray(programT.positionAttribute);
            
            float texCoords1T[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
            glVertexAttribPointer(programT.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords1T);
            glEnableVertexAttribArray(programT.texCoordAttribute);
            
            
            modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 0.75f, 1.0f));
            modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.5f, 0.0f));
            programT.SetModelMatrix(modelMatrix);

            glBindTexture(GL_TEXTURE_2D, titleTexture);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        
            
            //Press 'Space' to begin!
            modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-.25f, -.50f, 1.0f));
            programT.SetModelMatrix(modelMatrix);
            DrawText(programT, fontTexture, "Press 'space' to begin!", 0.15f, -0.10f);
            
            SDL_GL_SwapWindow(displayWindow);
        }
        
        
        
        
        //LEVEL 1
        else if (mode == STATE_GAME_LEVEL){
            
//            if (keys[SDL_SCANCODE_Q]){
//                mode = STATE_MAIN_MENU;
//                continue;
//            }


            //drawing in game loop
            glClear(GL_COLOR_BUFFER_BIT);
            glUseProgram(programT.programID);
            glEnableVertexAttribArray(programT.positionAttribute);

            
            while(elapsed >= FIXED_TIMESTEP) {
                player.resetCollide(); //makes all the collided false

                
                //UPDATING AND COLLISION CHECKING
                player.updateY(FIXED_TIMESTEP);
                for (Entity& groundBlock : groundBlocks){
                    player.collideY(groundBlock);
                }
                for (Entity& brickBlock : brickBlocks){
                    player.collideY(brickBlock);
                }
                
                for (Entity& questionBlock : questionBlocks){
                    player.collideY(questionBlock);
                }
                
                
                player.updateX(FIXED_TIMESTEP);
                for (Entity& groundBlock : groundBlocks){
                    player.collideX(groundBlock);
                }
                for (Entity& brickBlock : brickBlocks){
                    player.collideX(brickBlock);
                }
                for (Entity& questionBlock : questionBlocks){
                    player.collideX(questionBlock);
                }
                
                
                elapsed -= FIXED_TIMESTEP;
            }
            
            
            accumulator = elapsed;
            
            
            //RENDERING
            player.Render(programT, player);
    
            for (Entity& groundBlock : groundBlocks){
                groundBlock.Render(programT, player);
            }
            
            for (Entity& brickBlock : brickBlocks){
                brickBlock.Render(programT, player);
            }
            
            for (Entity& questionBlock : questionBlocks){
                questionBlock.Render(programT, player);
            }
            
            glDisableVertexAttribArray(programT.positionAttribute);
            SDL_GL_SwapWindow(displayWindow);
        }
    }
    
    SDL_Quit();
    return 0;
}





















































/*
 
 
 WILL FIGURE THIS OUT SOON ... :(
 
 //game states

 
 MainMenu mainMenu;
 GameLevel gameLevel;
 
 void Render() {
 switch(mode) {
 case STATE_MAIN_MENU:
 mainMenu.Render();
 break;
 case STATE_GAME_LEVEL:
 gameLevel.Render();
 break;
 default:
 break;
 }
 }
 
 
 void Update(float elapsed) {
 switch(mode) {
 case STATE_MAIN_MENU:
 mainMenu.Update(elapsed);
 break;
 case STATE_GAME_LEVEL:
 gameLevel.Update(elapsed);
 break;
 
 default:
 break;
 }
 
 }
 
 
 void ProcessInput(Entity &player, Entity &bullet, bool fired, float elapsed) {
 switch(mode) {
 case STATE_MAIN_MENU:
 mainMenu.ProcessInput(player, bullet, fired, elapsed);
 break;
 case STATE_GAME_LEVEL:
 gameLevel.ProcessInput(player, bullet, fired, elapsed);
 break;
 default:
 break;
 }
 }

 */


