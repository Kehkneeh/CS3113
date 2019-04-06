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




//Kenny Yip Ky952 CS3113 HW 03 - Space Invaders

#include <iostream>
#include <string>
#include <vector>
using namespace std;


//globals
float screenWidth = 640;
float screenHeight = 640;



SDL_Window* displayWindow;
ShaderProgram program;  //untextured, this existed because I used untextured to test my game logic first before switching to textured. Now unused
ShaderProgram programT; //textured

glm::mat4 modelMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);
glm::mat4 projectionMatrix = glm::mat4(1.0f);




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






class Entity {
public:
    Entity(float posX, float posY, float width, float height, float velocity, float dirX, float dirY, int textID, const SheetSprite& sprite)
    : xPos(posX), yPos(posY), width(width), height(height), velocity(velocity), xDir(dirX), yDir(dirY), textureID(textID), sprite(sprite) {}
    
    void setColor(float r, float g, float b){
        colors[0] = r;
        colors[1] = g;
        colors[2] = b;
    }
    
    
    void Draw(ShaderProgram &p){
        glUseProgram(p.programID);
        
        //used for untextured
//        float vertices[] = {-width/2, -height/2, width/2, -height/2, width/2, height/2,
//            -width/2, -height/2, width/2, height/2, -width/2, height/2};
        
        modelMatrix = glm::mat4(1.0f);
        
        modelMatrix = glm::translate(modelMatrix, glm::vec3(xPos, yPos, 1.0f));
        
        p.SetModelMatrix(modelMatrix);
        p.SetColor(colors[0], colors[1], colors[2], 1.0f);
        sprite.Draw(p);
        
        
    }
    
    
    
    float xPos;
    float yPos;
    float rotation;
    int textureID;
    
    
    
    float width;
    float height;
    float velocity;
    float xDir;
    float yDir;
    
    float health = 1.0f;
    SheetSprite sprite;
    float colors[3] = {1.0f, 1.0f, 1.0f};
};





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
    
    // draw this data (use the .data() method of std::vector to get pointer to data)
//    float* vertexDataArr = vertexData.data();
//    float* texCoordDataArr = texCoordData.data();
    
    
    glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(p.positionAttribute);
    
    glVertexAttribPointer(p.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(p.texCoordAttribute);
    
    // draw this yourself, use text.size() * 6 or vertexData.size()/2 to get number of vertices
    
    glDrawArrays(GL_TRIANGLES, 0 , (int) text.size() * 6);
    
    glDisableVertexAttribArray(p.positionAttribute);
    glDisableVertexAttribArray(p.texCoordAttribute);
    
    
}





//box box collision
bool Collision(const Entity& object1, const Entity& object2){
    float checkX = abs(object1.xPos - object2.xPos) - (object1.width + object2.width)/2;
    float checkY = abs(object1.yPos - object2.yPos) - (object1.height + object2.height)/2;
    return (checkX < 0) && (checkY < 0);
}






//shader program, screen height, screen width
void Setup(ShaderProgram &p, float screenWidth, float screenHeight){
    SDL_Init(SDL_INIT_VIDEO);
    
    displayWindow = SDL_CreateWindow("Ky952 Space Invasion", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, screenWidth, screenHeight);
    
    projectionMatrix = glm::ortho(-screenWidth/screenHeight, screenWidth/screenHeight, -1.0f, 1.0f, -1.0f, 1.0f);
    
//    p.Load("vertex.glsl", "fragment.glsl");
    p.Load("vertex_textured.glsl", "fragment_textured.glsl"); //textured
    
    p.SetProjectionMatrix(projectionMatrix);
    p.SetViewMatrix(viewMatrix);
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); //dark gray for space
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}












enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL};

GameMode mode;







//generate enemies
void CreateEnemies(vector<Entity>& enemies, float EnemyWidth, float EnemyHeight, SheetSprite EnemySprite){
    for (int i = 0; i < 3; i++){
        enemies.push_back(Entity(-0.4f + .4*(i % 3), 0.8f, EnemyWidth, EnemyHeight, 1.0f, 1.0f, 1.0f, 0, EnemySprite));
        enemies[enemies.size()-1].sprite = EnemySprite;
        
    }
}














int main(int argc, char *argv[])
{
    
    
    Setup(programT, screenWidth, screenHeight);
    
    //I have trouble with RESOURCE_FOLDER working properly
    //    GLuint fontTexture = LoadTexture(RESOURCE_FOLDER"font1.png");
    
    
    GLuint fontTexture = LoadTexture("font1.png");
    
    GLuint SpaceShooterSpriteSheet = LoadTexture("spaceshootersheet.png");
    
    
    
    //<SubTexture name="playerShip1_blue.png" x="211" y="941" width="99" height="75"/>
    
    SheetSprite playerSprite = SheetSprite(SpaceShooterSpriteSheet, 211.0f/1024.0f, 941.0f/1024.0f, 99.0f/1024.0f, 75.0f/1024.0f, 0.2f);
    Entity player = Entity(0.0f, -0.8f, 0.1f, 0.1f, 1.0f, 1.0f, 1.0f, 0, playerSprite);
    int playerScore = 0; //number of spaceships destroyed

    
    
    //<SubTexture name="laserBlue14.png" x="842" y="206" width="13" height="57"/>
    SheetSprite bulletSprite = SheetSprite(SpaceShooterSpriteSheet, 842.0f/1024.0f, 206.0f/1024.0f, 13.0f/1024.0f, 57.0f/1024.0f, 0.2f);
    Entity bullet = Entity(player.xPos, player.yPos, 0.025f, 0.05f, 1.0f, 1.0f, 1.0f, 0, bulletSprite);
    bool fired = false;
    
    

    //<SubTexture name="enemyGreen2.png" x="133" y="412" width="104" height="84"/>
    SheetSprite enemySprite = SheetSprite(SpaceShooterSpriteSheet, 133.0f/1024.0f, 412.0f/1024.0f, 104.0f/1024.0f, 84.0f/1024.0f, 0.2f);
    
    mode = STATE_MAIN_MENU;
    
    
    //keyboard stuff
    SDL_Event event;
    bool done = false;
    
    float lastFrameTicks = 0.0f; //sourced from slide 6
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    
    
    //generate enemies
    vector<Entity> enemies;
    float EnemyWidth = 0.1f;
    float EnemyHeight = 0.1f;
    
    float enemiesTimer = 1.0f; //used to determine when to generate enemies
    
    
    
    
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
        
        
        if (mode == STATE_MAIN_MENU){
        
            if(keys[SDL_SCANCODE_SPACE]) {
                mode = STATE_GAME_LEVEL;
                continue;
            }

            
            
            //drawing in game loop
            glClear(GL_COLOR_BUFFER_BIT);
    
            
            glEnableVertexAttribArray(programT.positionAttribute);
            
            
            
            //SPACE INVADERS!
            modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-.75f, .80f, 1.0f));
            programT.SetModelMatrix(modelMatrix);

            DrawText(programT, fontTexture, "Space Invaders", 0.25f, -0.15f);
            
            
            //Player Sprite for title screen decor
            player.Draw(programT);
            
            
            
            //Press 'Space' to begin!
            modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-.25f, -.50f, 1.0f));
            programT.SetModelMatrix(modelMatrix);
            
            DrawText(programT, fontTexture, "Press 'space' to begin!", 0.15f, -0.10f);
            
            SDL_GL_SwapWindow(displayWindow);

        }
        
        
        
        
        else if (mode == STATE_GAME_LEVEL){
            
            //Generating Enemies every 2 seconds?
            enemiesTimer += 1.0f * elapsed;
            
            if (enemiesTimer > 2.0f && (enemies.size() == 0 || (enemies[enemies.size() - 1].yPos != 0.8f ))){ // ensures enemies don't overlap!
                CreateEnemies(enemies, EnemyWidth, EnemyHeight, enemySprite);
                enemiesTimer = 1.0f;
            }
            
            
            
//            gameLevel.ProcessInput(player, bullet, fired, elapsed);
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            
            //Firing Bullet
            if(keys[SDL_SCANCODE_SPACE] && !fired) {
                fired = true;
            }
            
            
            if (fired){
                bullet.yPos += 5.0 * elapsed;
                
                if (bullet.yPos > 1.0f + bullet.height/2){ //out of bounds
                    bullet.xPos = player.xPos;
                    bullet.yPos = player.yPos;
                    fired = false;
                    
                    
                }
            }
            
            // left and right keys for moving space ship; the bullet follows the player movements if it hasn't been fired
            
            if(keys[SDL_SCANCODE_RIGHT] && player.xPos < screenWidth/screenHeight - player.width/2) {
                player.xPos += 1.0 * elapsed;
                if (!fired) { bullet.xPos = player.xPos; }
                
            }
            
            if (keys[SDL_SCANCODE_LEFT] && player.xPos > -screenWidth/screenHeight + player.width/2){
                player.xPos -= 1.0 * elapsed;
                if (!fired) { bullet.xPos = player.xPos; }
            }
            
            
            
            //drawing in game loop
            glClear(GL_COLOR_BUFFER_BIT);
            
            glUseProgram(programT.programID);
            
            glEnableVertexAttribArray(programT.positionAttribute);
            
            player.Draw(programT);
            
            if (fired){
                bullet.Draw(programT);
            }
            
            
            
            for (size_t i = 0; i < enemies.size(); i++) {
                if (fired && Collision(enemies[i], bullet)){
                    fired = false;
                    bullet.yPos = player.yPos;
                    
                    
                    //destroy the enemy
                    Entity temp = enemies[i];
                    enemies[i] = enemies[enemies.size( ) - 1];
                    enemies[enemies.size() - 1] = temp;
                    enemies.pop_back();
                    
                    playerScore ++;
                    
                }
                
                //if enemy crashes into player or 'Q' is pressed, return to main menu
                if (Collision(enemies[i], player) || keys[SDL_SCANCODE_Q]){
                    //game over
                    enemies.clear(); //clear enemies vector
                    
                    //reset these
                    player = Entity(0.0f, -0.8f, 0.1f, 0.1f, 1.0f, 1.0f, 1.0f, 0, playerSprite);

                    
                    
                    playerScore = 0;

                    
                    bullet = Entity(player.xPos, player.yPos, 0.025f, 0.05f, 1.0f, 1.0f, 1.0f, 0, bulletSprite);
                    
                    
                    fired = false;
                    
                    mode = STATE_MAIN_MENU;
                    continue;
                }
                
                
                enemies[i].xPos +=  0.3f * elapsed * enemies[i].xDir;
                
                //moving enemies left and right
                if (enemies[i].xDir == -1){
                    
                    if (enemies[i].xPos < -screenWidth/screenHeight + enemies[i].width/2) {
                        enemies[i].yPos -= 0.3f;
                        enemies[i].xDir = 1;
                    }
                    
                }
                else{
                    if (enemies[i].xPos > screenWidth/screenHeight - enemies[i].width/2) {
                        enemies[i].yPos -= 0.3f;
                        enemies[i].xDir = -1;
                    }
                }
                
                
                enemies[i].Draw(programT);
            }
            
            
            //Draw Player's Score
            modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-.75f, .80f, 1.0f));
            programT.SetModelMatrix(modelMatrix);
            
            DrawText(programT, fontTexture, to_string(playerScore), 0.25f, -0.15f);
            
            
            
            //Press 'Q' to return to Main Menu
            modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-0.95f, -0.9f, 1.0f));
            programT.SetModelMatrix(modelMatrix);
            
            DrawText(programT, fontTexture, "Press 'Q' to return to Main Menu", 0.10f, -0.050f);
            
            glDisableVertexAttribArray(program.positionAttribute);
            SDL_GL_SwapWindow(displayWindow);
            
        }
        
        
        
        
    }
    
    SDL_Quit();
    return 0;
}















 
/*
 
 
 WILL FIGURE THIS OUT SOON ... :(
 
 //game states
 
 
 
 struct MainMenu{
 void Render(){
 }
 
 
 void ProcessInput(Entity &player, Entity &bullet, bool fired, float elapsed){
 
 }
 };
 
 
 
 
 
 
 struct GameLevel{
 void Render(){
 
 }
 
 void ProcessInput(Entity &player, Entity &bullet, bool fired, float elapsed){
 
 const Uint8 *keys = SDL_GetKeyboardState(NULL);
 
 //Firing Bullet
 if(keys[SDL_SCANCODE_SPACE] && !fired) {
 fired = true;
 }
 
 
 if (fired){
 bullet.yPos += 5.0 * elapsed;
 
 if (bullet.yPos > 1.0f + bullet.height/2){ //out of bounds
 bullet.xPos = player.xPos;
 bullet.yPos = player.yPos;
 fired = false;
 
 
 }
 }
 
 // left and right keys for moving space ship; the bullet follows the player movements if it hasn't been fired
 
 if(keys[SDL_SCANCODE_RIGHT] && player.xPos < screenWidth/screenHeight - player.width/2) {
 player.xPos += 1.0 * elapsed;
 if (!fired) { bullet.xPos = player.xPos; }
 
 }
 
 if (keys[SDL_SCANCODE_LEFT] && player.xPos > -screenWidth/screenHeight + player.width/2){
 player.xPos -= 1.0 * elapsed;
 if (!fired) { bullet.xPos = player.xPos; }
 }
 
 }
 };
 
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













//        /* for reference from slides
//         y_position += elapsed * distance_to_travel_in_one_second;
//
//         position += direction_vector * elapsed * units_a_second;
//
//         position.x += direction.x * elapsed * units_a_second;
//
//         position.y += direction.y * elapsed * units_a_second;
//
//         */
