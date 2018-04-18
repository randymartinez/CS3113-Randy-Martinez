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

#if defined(_WINDOWS) || (LINUX)
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;
GLuint LoadTexture(const char *filePath);


class Ball{
    float vertices[12] = {-0.1f, -0.1f, 0.1f, -0.1f, 0.1f, 0.1f, -0.1f, -0.1f, 0.1f, 0.1f, -0.1f, 0.1f};
    Matrix model;
    Matrix view;
    
public: 
    Ball(){}

    float velX = 2.0f;
    float velY = 2.0f;
    float angle = 0;
    float x = 0;
    float y = 0;
    float width = 0.05f;
    float height = 0.05f;
    
    void reset(){
        x = 0;
        y = 0;
    }
    
    void draw(ShaderProgram program){
        model.Identity();
        model.Translate(x, y, 0.0f);
        program.SetModelMatrix(model);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        program.SetViewMatrix(view);
    }
};

class Paddle{
    float vertices[12] = {-0.1f, -0.1f, 0.1f, -0.1f, 0.1f, 0.1f, -0.1f, -0.1f, 0.1f, 0.1f, -0.1f, 0.1f};
    Matrix model;
    Matrix view;
    
    Matrix ptsModel;
    Matrix ptsView;
    float ptsVertices[12] = {-0.1f, -0.1f, 0.1f, -0.1f, 0.1f, 0.1f, -0.1f, -0.1f, 0.1f, 0.1f, -0.1f, 0.1f};

public:
    Paddle(bool x){ side = x;}

    int points = 0;
    float y = 0.0f;
    float x;
    float width = 0.05f;
    float height = 0.45f;
    bool side;
    float y_max = 2.0f;
    
    void move(int direction, float elapsed){
        float tmp = elapsed*direction*5 + y;
        if (abs(tmp) < y_max){
            y = tmp;
        }
    }
    bool Contact(float Bx, float By){
        bool xBounds = x - (width) <= Bx && Bx <= x + (width);
        bool yBounds = y - height <= By && By <= y + height;
        return xBounds && yBounds;
    }
    void addPts(){
        points++;
    }
    int getPts(){
        return points;
    }
    void reset(){
        points = 0;
        y = 0;
    }
    void draw(ShaderProgram& program){
        model.Identity();
        ptsModel.Identity();
        if(side){
            x = 3.0f;
            for(int i = 0; i < points; i++){
                ptsModel.Translate(x-(i), 1.9f, 0.0f);
                program.SetModelMatrix(ptsModel);
                glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ptsVertices);
                glEnableVertexAttribArray(program.positionAttribute);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }
        else{
            x = -3.0f;
            for(int i = 0; i < points; i++){
                ptsModel.Translate(x-(i), 1.9f, 0.0f);
                program.SetModelMatrix(ptsModel);
                glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ptsVertices);
                glEnableVertexAttribArray(program.positionAttribute);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }
        model.Translate(x, y, 0.0f);
        model.Scale(1.0f,4.5f,0.0f);
        program.SetViewMatrix(view);
        
        program.SetModelMatrix(model);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
};


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    #if defined(_WINDOWS) || (LINUX)
        glewInit();
    #endif

    glViewport(0, 0, 1280, 720);
    ShaderProgram program;
    program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    float lastFrameTicks = 0.0f;
    float ticks = 0.0f;
    float elapsed = 0;
    
    Matrix projectionMatrix;
    
    Ball ball;
    Paddle PadL(0);
    Paddle PadR(1);
    
    
    
    projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    
    glUseProgram(program.programID);

	SDL_Event event;
	bool done = false;
    
    Mix_Music *music;
    music = Mix_LoadMUS("Tetris.wav");
    Mix_PlayMusic(music, -1);
    
    Mix_Chunk *bump;
    bump = Mix_LoadWAV("bump.wav");
    
    Mix_Chunk *score;
    score = Mix_LoadWAV("score.wav");
    
	while (!done) {
		while (SDL_PollEvent(&event)) {
            //const Uint8 *keys = SDL_GetKeyboardState(NULL);
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}

            else if(event.type == SDL_KEYDOWN){
                if(event.key.keysym.scancode == SDL_SCANCODE_UP && event.key.keysym.scancode == SDL_SCANCODE_W) {
                    PadR.move(1, elapsed);
                    PadL.move(1, elapsed);
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_DOWN && event.key.keysym.scancode == SDL_SCANCODE_S) {
                    PadR.move(-1, elapsed);
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_W) {
                    PadL.move(1, elapsed);
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_S) {
                    PadL.move(-1, elapsed);
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_UP) {
                    PadR.move(1, elapsed);
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
                    PadR.move(-1, elapsed);
                }
            }
            else if(event.type == SDL_KEYUP){
                PadR.move(0, elapsed);
                PadL.move(0, elapsed);
            }

		}
		ball.x += elapsed * ball.velX;
        ball.y += elapsed * ball.velY;
        
        if(ball.y + ball.height >= 2.0f){//Ceiling
            Mix_PlayChannel(-1, bump, 0);
            ball.velY *= -1.0f;
            
        }
        if(ball.y - ball.height <= -2.0f){//Floor
            Mix_PlayChannel(-1, bump, 0);
            ball.velY *= -1.0f;
            
        }
        if(ball.x + ball.height >= 3.50f){//Right Wall
            Mix_PlayChannel(-1, score, 0);
            ball.velX *= -1.0f;
            PadL.addPts();
            ball.reset();
            
        }
        if(ball.x - ball.height <= -3.50f){//Left Wall
            Mix_PlayChannel(-1, score, 0);
            ball.velX *= -1.0f;
            PadR.addPts();
            ball.reset();
            
        }

        if(PadR.Contact(ball.x, ball.y) || PadL.Contact(ball.x, ball.y)){//Touches paddle
            Mix_PlayChannel(-1, bump, 0);
            ball.velX *= -1.0f;
        }
        
        if(PadR.getPts() > 1){
            PadR.reset();
            PadL.reset();
            ball.reset();
        }
        else if(PadL.getPts() > 1){
            PadR.reset();
            PadL.reset();
            ball.reset();
        }
        

        ticks = (float)SDL_GetTicks() / 1000.0f;
        elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;

		glClear(GL_COLOR_BUFFER_BIT);
        
        ball.draw(program);
        PadL.draw(program);
        PadR.draw(program);

        glDisableVertexAttribArray(program.positionAttribute);
        program.SetProjectionMatrix(projectionMatrix);


        SDL_GL_SwapWindow(displayWindow);

	}
    Mix_FreeMusic(music);
	SDL_Quit();
	return 0;
}








