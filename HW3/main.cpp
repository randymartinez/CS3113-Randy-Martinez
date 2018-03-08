#if defined(_WINDOWS) || (LINUX)
    #include <GL/glew.h>
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"
#include "ShaderProgram.h"
#include <vector>

#if defined(_WINDOWS) || (LINUX)
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define MAX_BULLETS 5
SDL_Window* displayWindow;
GLuint LoadTexture(const char *filePath);

class Vector2{
public:
    float x;
    float y;
    
    Vector2(){}
    Vector2(float A, float B){x = A; y = B;}
    
};

class SheetSprite {
    GLuint textureID;
    float u;
    float v;
    float width;
    float height;
    float size;
public:
    SheetSprite(){}
    SheetSprite(GLuint textureID, float u, float v, float width, float height, float size): textureID(textureID), u(u), v(v), width(width), height(height), size(size){}
    
    void Draw(ShaderProgram *program) {
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
        
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->positionAttribute);
        
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program->texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);

    }
};

class Bullet{
    SheetSprite sprite;
    float vertices[12] = {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};
    Matrix model;
    Matrix view;
    Vector2 position;
    float xscale = 0.2f;
    float yscale = 0.2f;
    float speed = 0.1f;
public:
    //Hitbox
    bool isActive = false;
    float top;
    float bot;
    float right;
    float left;
    float width = 9.0f/1024.0f;
    float height = 54.0f/1024.0f;
    float aspect = width/height;
    
    Bullet(GLuint texture){
        sprite = SheetSprite(texture, 856.0f/1024.0f, 421.0f/1024.0f, 9.0f/1024.0f, 54.0f/1024.0f, 1.0f);
    }
    void Shoot(Vector2 pos){
        isActive = true;
        position = pos;
        top = position.y + ( height * aspect);
        bot =  position.y - (height * aspect);
        right =  position.x + (width * aspect);
        left = position.x - (width * aspect);
    }
    void Draw(ShaderProgram* program, float elapsed){
        //printf("%f \n",elapsed);
        position.y += speed*elapsed;
        printf("y: %f \n", position.y);
        if(isActive && position.y < 2.0f){
            top=position.y + (yscale/2);
            bot=position.y - (yscale/2);
            model.Identity();
            model.Translate(position.x, position.y, 0.0f);
            model.Scale(xscale,yscale, 1.0f);
            program->SetModelMatrix(model);
            program->SetViewMatrix(view);
            sprite.Draw(program);
        }
        else{isActive = false;}
    }
};

class Invader{
    SheetSprite sprite;
    float vertices[12] = {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};
    float scale = 0.3f;
    Matrix model;
    Matrix view;
public:
    //Hitbox
    float top;
    float bot;
    float right;
    float left;
    float width = 93.0f/1024.0f;
    float height = 84.0f/1024.0f;
    float aspect = width/height;
    bool isActive = true;
    Vector2 position;
    
    Invader(GLuint texture, Vector2 pos){
        sprite = SheetSprite(texture, 423.0f/1024.0f, 728.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 1.0f);
        position = pos;
        top = (position.y + ( height * aspect)) + 0.1f;
        bot =  (position.y - (height * aspect)) - 0.1f;
        right =  (position.x + (width * aspect)) + 0.1f;
        left = (position.x - (width * aspect)) - 0.1f;
        
    }
    void Collision(Bullet* laser){
        if( isActive && laser->right >= left && laser->left <= right && laser->top >= bot && laser->bot <= top){
            laser->isActive = false;
            this->isActive = false;
        }
    }

    void Draw(ShaderProgram* program){
        top = (position.y + ( height * aspect)) + 0.1f;
        bot =  (position.y - (height * aspect)) - 0.1f;
        right =  (position.x + (width * aspect)) + 0.1f;
        left = (position.x - (width * aspect)) - 0.1f;
        if(isActive){
            model.Identity();
            model.Translate(position.x, position.y, 0.0f);
            model.Scale(scale, scale, 1.0f);
            program->SetModelMatrix(model);
            sprite.Draw(program);
        }
    }
};

class Player{
    Vector2 position;
    SheetSprite sprite;
    float vertices[12] = {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};
    float scale = 0.3f;
    float speed = 10.0f;
    Matrix model;
    Matrix view;
public:
    //Hitbox
    float top;
    float bot;
    float right;
    float left;
    float width = 99.0f/1024.0f;
    float height = 75.0f/1024.0f;
    float aspect = width/height;
    bool isActive = true;
    std::vector<Bullet> lasers;
    
    Player(GLuint texture){
        //sprite = SheetSprite(texture, 423.0f/1024.0f, 728.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 1.0f);
        sprite = SheetSprite(texture, 224.0f/1024.0f, 832.0f/1024.0f, 99.0f/1024.0f, 75.0f/1024.0f, 1.0f);
        //playerShip1_red.png" x="224" y="832" width="99" height="75"
        position = Vector2(0.0f, -1.75f);
        top = (position.y + ( height * aspect));
        bot =  (position.y - (height * aspect));
        right =  (position.x + (width * aspect));
        left = (position.x - (width * aspect));
        
        //Laser object pool
        for(int i = 0; i < MAX_BULLETS; i++){lasers.push_back(Bullet(texture));}
        
    }
    void Collision(Bullet* laser){
        if( isActive && laser->right >= left && laser->left <= right && laser->top >= bot && laser->bot <= top){
            laser->isActive = false;
            this->isActive = false;
        }
    }
    
    void Shoot(){
        for(int i = 0; i < MAX_BULLETS; i++){
            if(!lasers[i].isActive){
                lasers[i].Shoot(Vector2(position.x,position.y));
                return;
            }
        }
    }
    
    void Move(int direction, float elapsed){
        if(position.x + (elapsed*speed*direction) < 2.5f && position.x + (elapsed*speed*direction) > -2.5f){
            position.x += (elapsed*speed*direction);
        }
    }

    void Draw(ShaderProgram* program){
        if(isActive){
            model.Identity();
            model.Translate(position.x, position.y, 0.0f);
            model.Scale(scale, scale, 1.0f);
            program->SetModelMatrix(model);
            sprite.Draw(program);
        }
    }
};

class Army{
    Vector2 position;
    int rightMost = 0;
    int leftMost = 10;
    int botMost = 54;
    int direction = 1;
    float speed = 1.0f;
public:
    std::vector<Invader> enemies;
    
    Army(GLuint texture){
       for(int i = 0; i < 5; i++){
           for(int j = 0; j < 11; j++){
               enemies.push_back(Invader(texture, Vector2(-2.0f + (j * 0.4f), 1.5f - (i * 0.4f))));
            }
        }
    }
    
    void Move(float elapsed){
        if(!enemies[botMost].isActive || !enemies[leftMost].isActive || !enemies[rightMost].isActive){
            for(size_t i = 0; i < enemies.size(); i++){
                if(enemies[i].isActive){

                    if(enemies[i].position.y > enemies[botMost].position.y || !enemies[botMost].isActive){
                        botMost = i;
                    }
                    if(enemies[i].position.x > enemies[rightMost].position.x || !enemies[rightMost].isActive){
                        rightMost = i;
                    }
                    if(enemies[i].position.x < enemies[leftMost].position.x || !enemies[leftMost].isActive){
                        leftMost = i;
                    }
                }
            }
        }
        if(enemies[rightMost].position.x > 0.5f || enemies[leftMost].position.x < -0.1f){
            direction *= -1;
            for(int i = 0; i < 55; i++){
                enemies[i].position.y -= (4.0f * elapsed);
            }
        }
        
        for(int i = 0; i < 55; i++){
            enemies[i].position.x += direction * (speed * elapsed);
        }
    }
    
    bool reachedPlayer(){
        if(botMost <= -1.75f){return true;}
        return false;
    }
    
    bool isEmpty(){
        for(size_t i = 0; i < enemies.size(); i++){
            if(enemies[i].isActive){return false;}
        }
        return true;
    }
};

class letters{
    SheetSprite sprite;
    float vertices[12] = {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};
    Matrix model;
    Matrix view;
    Vector2 position;
public:
    letters(GLuint texture,char character, Vector2 coords){
        //15x5
        int ascii = character%26;
        float x;
        float y;
        float width;
        float height;
        
        sprite = SheetSprite(texture, 856.0f/1024.0f, 421.0f/1024.0f, 9.0f/1024.0f, 54.0f/1024.0f, 1.0f);
    }
};

class State{
    public:
        State(){}
        
        void Game(){
            Matrix projectionMatrix;  
            ShaderProgram program;
            program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
            float lastFrameTicks = 0.0f;
            float ticks = 0.0f;
            float elapsed = 0;
            
            GLuint texture = LoadTexture(RESOURCE_FOLDER"sheet.png");
            projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
            
            glUseProgram(program.programID);
            
            Player P1(texture);
            Army A(texture);
            
            bool won;
            program.SetProjectionMatrix(projectionMatrix);
            SDL_Event event;
            bool Done = false;
            while(!Done){
                ticks = (float)SDL_GetTicks() / 1000.0f;
                elapsed = ticks - lastFrameTicks;
                lastFrameTicks = ticks;
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                        Done = true;
                    }
                    else if(event.type == SDL_KEYDOWN){
                        if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {P1.Shoot();}
                        else if(event.key.keysym.scancode == SDL_SCANCODE_RIGHT){P1.Move(1, elapsed);}
                        else if(event.key.keysym.scancode == SDL_SCANCODE_LEFT){P1.Move(-1, elapsed);}
                    }
                }
            
                glClear(GL_COLOR_BUFFER_BIT);
                
                for(int i = 0; i < 55; i++){
                    for(int j = 0; j < MAX_BULLETS; j++){
                        if(P1.lasers[j].isActive && A.enemies[i].isActive){
                            A.enemies[i].Collision(&P1.lasers[j]);
                            P1.lasers[j].Draw(&program, elapsed);
                        }
                    }
                    A.enemies[i].Draw(&program);
                }
                
                A.Move(elapsed);
                if(A.isEmpty()){
                    Done = true;
                    won = true;
                }
                if(A.reachedPlayer()){
                    Done = true;
                    won = false;
                }
                P1.Draw(&program);
                
                glDisableVertexAttribArray(program.positionAttribute);
                glDisableVertexAttribArray(program.texCoordAttribute);
                SDL_GL_SwapWindow(displayWindow);
            }
        }
        
        void Menu(){
            Matrix projectionMatrix;  
            ShaderProgram program;
            program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
            GLuint texture = LoadTexture(RESOURCE_FOLDER"font1.png");
            projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
            
            glUseProgram(program.programID);
            program.SetProjectionMatrix(projectionMatrix);
            SDL_Event event;
            bool Done = false;
            //std::Vector<letters> text;
            
            while(!Done){
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                        Done = true;
                    }
                    else if(event.type == SDL_KEYDOWN){
                        if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {Done = true;}
                    }
                }
            
                glClear(GL_COLOR_BUFFER_BIT);
                
                
                
                glDisableVertexAttribArray(program.positionAttribute);
                glDisableVertexAttribArray(program.texCoordAttribute);
                SDL_GL_SwapWindow(displayWindow);
            }
            
        }
};


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
    #if defined(_WINDOWS) || (LINUX)
        glewInit();
    #endif

    glViewport(0, 0, 1280, 720);    
    State gameState;
    
    gameState.Menu();
    gameState.Game();
        
 
    SDL_Quit();
    return 0;
}

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







 
