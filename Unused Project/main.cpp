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
#include <SDL_mixer.h>
#include <vector>
#include <math.h>

#if defined(_WINDOWS) || (LINUX)
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define PI 3.14159265

using namespace std;

SDL_Window* displayWindow;
GLuint LoadTexture(const char *filePath);

GLuint texture;
ShaderProgram program;
Matrix view_matrix;


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
    
    void Draw() {
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
        
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);

    }
};

class Entity{
    float vertices[12] = {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};
    Matrix model;
    SheetSprite sprite;
    float spritex;
    float spritey;
    
public: 
    Entity(float i, float j, int t){
        x = i;
        y = j;
        type = t;
        if(type == 0){//Ball
            sprite = SheetSprite(texture, 1403.0f/2048.0f, 652.0f/1024.0f, 50.0f/1024.0f, 65.0f/1024.0f, 1.0f);
            xScale = 0.50f;
            yScale = 0.25f;
            xVelocity = 0.10f;
            yVelocity = 0.10f;
            isHeld = false;
        }else if(type == 1){//P1
            sprite = SheetSprite(texture, 0.0f/2048.0f, 910.0f/1024.0f, 173.0f/1024.0f, 64.0f/1024.0f, 1.0f);
            xScale = 0.50f;
            yScale = 0.25f;
            xVelocity = 1.0f;
        }else if(type == 2){//P2
            sprite = SheetSprite(texture, 0.0f/2048.0f, 910.0f/1024.0f, 173.0f/1024.0f, 64.0f/1024.0f, 1.0f);
            xScale = 0.50f;
            yScale = 0.25f;
            xVelocity = 1.0f;
        }else if(type == 3){//Bricks 128->256
            spritex = 0.0f;
            spritey = 128;
            sprite = SheetSprite(texture, 0.0f/2048.0f, 128.0f/1024.0f, 192.0f/1024.0f, 128.0f/1024.0f, 1.0f);
            xScale = 0.50f;
            yScale = 0.25f;
            isDamaged = false;
            isDestroyed = false;
        }
        height = yScale;
        width = xScale;
        
    }
    
    void move(int direction){//Paddle only function
        dir = direction;
        if(direction* xVelocity + x > 2.25 || direction* xVelocity + x < -2.25){
            x = direction* 2.25;
        }
        x += direction * xVelocity;
        if(ball != nullptr){
            ball->x = x;
            if(type == 1){
                ball->y = y + height;
            }else{
                ball->y = y - height;
            }
        }
    }
    
    void launch(){
        if(ball != nullptr){
            ball->x += dir;
            ball->y += -y/3.75f;
            ball->isHeld = false;
            ball = nullptr;
        }
    }
    
    bool collision(float Bx, float By){//generic
        bool xBounds = x - (width) <= Bx && Bx <= x + (width);
        bool yBounds = y - height <= By && By <= y + height;
        return xBounds && yBounds;
    }
    
    void onCollision(){
        if(type == 0){//balls
            //cout << "BALL X " << x << " Y " << y << endl;
            xVelocity *= -1;
            yVelocity *= -1;
        }else if(type == 1 || type == 2){
            //cout << "PADDLE X " << x << " Y " << y << endl;
            return;
        }else{//Brick
            if(isDamaged){
                isDestroyed = true;
            }else{
                spritey += 128;
                isDamaged = true;
                sprite = SheetSprite(texture, spritex/2048.0f, spritey/1024.0f, 192.0f/1024.0f, 128.0f/1024.0f, 1.0f);
            }
        }
    
    }
    
    Entity* ball;
    int dir;
    bool isHeld;
    
    float x;
    float y;
    
    float height;
    float width;
    
    float xScale;
    float yScale;
    
    float xVelocity;
    float yVelocity;
    
    bool isDamaged;
    bool isDestroyed;
    
    int type;
    
    void draw(){
        model.Identity();
        
        model.Translate(x, y, 0.0f);
        if(type == 2){
            model.Scale(xScale,-yScale, 1.0f);
        }else{
            model.Scale(xScale,yScale, 1.0f);
        }
        program.SetModelMatrix(model);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        sprite.Draw();
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
};

void playerInput(SDL_Event* event, Entity* P1, Entity* P2){
    if(event->key.keysym.scancode == SDL_SCANCODE_A) {
        P1->move(-1);
        }
    else if(event->key.keysym.scancode == SDL_SCANCODE_D) {
        P1->move(1);
    }
    if(event->key.keysym.scancode == SDL_SCANCODE_LEFT) {
        P2->move(-1);
        }
    else if(event->key.keysym.scancode == SDL_SCANCODE_RIGHT) {
        P2->move(1);
    }
    if(event->key.keysym.scancode == SDL_SCANCODE_W){
        P1->launch();
    }
    if(event->key.keysym.scancode == SDL_SCANCODE_UP){
        P2->launch();
    }
 
}
void game_state(){
    /*Mix_Music *music;
    music = Mix_LoadMUS("Tetris.wav");
    Mix_PlayMusic(music, -1);
    
    Mix_Chunk *bump;
    bump = Mix_LoadWAV("bump.wav");
    
    Mix_Chunk *score;
    score = Mix_LoadWAV("score.wav");
    
    

	Mix_FreeChunk(bump);
    Mix_FreeChunk(score);
    Mix_FreeMusic(music);
	SDL_Quit();*/
    
    
    Matrix projectionMatrix;  

    float lastFrameTicks = 0.0f;
    float ticks = 0.0f;
    float elapsed = 0;
    texture = LoadTexture(RESOURCE_FOLDER"assets/breakout.png");
    projectionMatrix.SetOrthoProjection(-4.0, 4.0, -4.0f, 4.0f, -1.0f, 1.0f);
    
    glUseProgram(program.programID);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(view_matrix);
    SDL_Event event;
    
    Entity P1(0,-3.75, 1);
    Entity P2(0, 3.75, 2);
    
    std::vector<Entity*> balls;
    balls.push_back(new Entity(0,0.5,0));
    
    vector<Entity*> bricks;
    
    /*for(int i = 0; i < 10; i++){
        bricks.push_back(new Entity((i/10), 0, 3));
    }*/
    bricks.push_back(new Entity(-1.50,0,3));
    bricks.push_back(new Entity(-0.75,0,3));
    bricks.push_back(new Entity(0,0,3));
    bricks.push_back(new Entity(0.75,0,3));
    bricks.push_back(new Entity(1.50,0,3));
    
    
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
                playerInput(&event, &P1, &P2);
            }
        }
    
        glClear(GL_COLOR_BUFFER_BIT);
        
        P1.draw();
        P2.draw();

        for(int i = 0; i < bricks.size(); i++){
            if(bricks[i]->collision(balls[0]->x, balls[0]->y)){
                balls[0]->xVelocity *= -1;
                balls[0]->yVelocity *= -1;
                bricks[i]->onCollision();
                if(bricks[i]->isDestroyed){
                    delete bricks[i];
                    bricks.erase(bricks.begin() + i);
                    continue;
                }
            }
            bricks[i]->draw();

            
        }
        for(int i = 0; i < balls.size(); i++){
            if(balls[i]->isHeld == true){
                balls[i]->draw();
                continue;
            }
            if(P1.collision(balls[i]->x, balls[i]->y)){
                cout << balls[i]->x << " " << balls[i]->y << endl;
                cout << P1.x << " " << P1.y << endl;
                balls[i]->xVelocity *= -1;
                balls[i]->yVelocity *= -1;
            }
            if(P2.collision(balls[i]->x, balls[i]->y)){
                cout << balls[i]->x << " " << balls[i]->y << endl;
                cout << P2.x << " " << P2.y << endl;
                balls[i]->xVelocity *= -1;
                balls[i]->yVelocity *= -1;
            }

            if(balls[i]->y > 3.75 && balls[i]->isHeld == false){
                balls[i]->isHeld = true;
                balls[i]->x = P2.x;
                balls[i]->y = P2.y - P2.height;
                P2.ball = balls[i];
                balls[i]->draw();
                continue;
            }
            if(balls[i]->y < -3.75f && balls[i]->isHeld == false){
                balls[i]->isHeld = true;
                balls[i]->x = P1.x;
                balls[i]->y = P1.y + P1.height;
                P1.ball = balls[i];
                balls[i]->draw();
                continue;
            }
            if(balls[i]->x > 3.75 || balls[i]->x < -3.75f){
                balls[i]->xVelocity *= -1;
            }
            balls[i]->x += balls[i]->xVelocity;
            balls[i]->y += balls[i]->yVelocity;
            balls[i]->draw();
        }
        
        
        
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        SDL_GL_SwapWindow(displayWindow);
    }
}
    
int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 720, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    #if defined(_WINDOWS) || (LINUX)
        glewInit();
    #endif
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, 720, 720);
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    //program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    game_state();

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






