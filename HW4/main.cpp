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
#include "FlareMap.h"
#include <fstream>
#include <string>
#include <iostream>
#include <vector>

#if defined(_WINDOWS) || (LINUX)
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define TILE_SIZE 0.2
#define LEVEL_HEIGHT 20
#define LEVEL_WIDTH 20
#define SPRITE_COUNT_X 16
#define SPRITE_COUNT_Y 8

using namespace std;

unsigned int levelData[LEVEL_HEIGHT][LEVEL_WIDTH];

void placeEntity(string type, float placeX, float placeY);
GLuint LoadTexture(const char *filePath);
void renderLevel(ShaderProgram* program, GLuint texture);
void generateLevelData(ShaderProgram* program);
void buildColliders();

Matrix model_matrix;
Matrix view_matrix;
FlareMap map;

SDL_Window* displayWindow;

struct Vector2{
    float x;
    float y;
};
class Collider{
protected:
    Vector2 pos;
public:
    Collider(float x, float y) {
        pos.x = x;
        pos.y = y;
    }
    
    bool isColliding(Vector2 coords){
        //cout << pos.x << " " << pos.y << endl;
        bool right = (pos.x + TILE_SIZE/2) >= (coords.x - TILE_SIZE/2);
        bool top = (pos.y + TILE_SIZE/2) >= (coords.y - TILE_SIZE/2);
        bool bottom = (pos.y - TILE_SIZE/2) <= (coords.y + TILE_SIZE/2);
        bool left = (pos.x - TILE_SIZE/2) <= (coords.x + TILE_SIZE/2);
        return right && left && top && bottom;
    }
};

vector<Collider> colliders;

class Player{
    Vector2 pos;
    Matrix Model;
    float acceleration = -1.0f;
    bool isGrounded;
    Vector2 textureIndex;
    bool hasKey;
    int index = 80;
    float velocity = 2.0f;
    GLuint textureID;
    
public:
    Player(float xCoord, float yCoord){
        pos.x = xCoord;
        pos.y = yCoord;
        textureID = LoadTexture("arne_sprites.png");
    }
    void jump(){
        if(isGrounded){
            pos.y += 1 * TILE_SIZE;
            isGrounded = false;
        }
    }
    void move(int direction, float elapsed){
        float tmp = (direction * velocity * elapsed) + pos.x;
        //cout << pos.x << " " << pos.y << endl;
        for(int i = 0; i < colliders.size(); i++){
            if(colliders[i].isColliding(pos)){
                return;
            }
        }
        pos.x = tmp; 
    }
    void Draw(ShaderProgram* program){
        Model.Identity();
        Model.Translate(pos.x, pos.y + acceleration, 0.0f);
        //Model.Scale(TILE_SIZE, TILE_SIZE, 1.0f);
        
        program->SetModelMatrix(Model);
        glBindTexture(GL_TEXTURE_2D, textureID);
        int x = 0;
        int y = 5;
        float u = (float)((index) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
        float v = (float)((index) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
        float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
        float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
        
        float vertices[] = {
            TILE_SIZE * x, -TILE_SIZE * y,
            TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
            (TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
            TILE_SIZE * x, -TILE_SIZE * y,
            (TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
            (TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
            };
        float texCoords[] = {
            u, v,
            u, v + (spriteHeight),
            u + spriteWidth, v + (spriteHeight),
            u, v,
            u + spriteWidth, v + (spriteHeight),
            u + spriteWidth, v
            };
        
        
        glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->positionAttribute);
        
        glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program->texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
    }
};

Player* player;

void buildColliders(){
    for(int y = 0; y < LEVEL_HEIGHT; y++){
        for(int x = 0; x < LEVEL_WIDTH; x++){
            if(map.mapData[y][x] != 0){
                Collider tmp(x,y * (-1));
                colliders.push_back(tmp);;
            }
        }
    }
}


//(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
void placeEntity(string type, float placeX, float placeY){
    if(type == "Player"){
        player = new Player(placeX, placeY * (-1));
    }
    else if(type == "Key"){
    }
}


void renderLevel(ShaderProgram* program, GLuint texture) {
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	for (int y = 0; y < LEVEL_HEIGHT; y++) {
		for (int x = 0; x < LEVEL_WIDTH; x++) {
            if(map.mapData[y][x] != 0){
                float u = (float)(((int)map.mapData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
                float v = (float)(((int)map.mapData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
                float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
                float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
                vertexData.insert(vertexData.end(), {
                    TILE_SIZE * x, -TILE_SIZE * y,
                    TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
                    (TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
                    TILE_SIZE * x, -TILE_SIZE * y,
                    (TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
                    (TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
                    });
                texCoordData.insert(texCoordData.end(), {
                    u, v,
                    u, v + (spriteHeight),
                    u + spriteWidth, v + (spriteHeight),
                    u, v,
                    u + spriteWidth, v + (spriteHeight),
                    u + spriteWidth, v
                    });
            }
		}
	}

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);

	model_matrix.Identity();
	program->SetModelMatrix(model_matrix);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLES, 0, 6 * LEVEL_HEIGHT * LEVEL_WIDTH);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
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


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
    #if defined(_WINDOWS) || (LINUX)
        glewInit();
    #endif

    glViewport(0, 0, 640, 360);
    
    ShaderProgram program;
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    //program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    float lastFrameTicks = 0.0f;
    float ticks = 0.0f;
    float elapsed = 0;
    
    Matrix projectionMatrix;
    
    projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    view_matrix.Translate(-1.0f,2.0f,0.0f);
    glUseProgram(program.programID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    
    map.Load("map_collision.txt");
	for (int i = 0; i < map.entities.size(); i++) {
		placeEntity(map.entities[i].type, map.entities[i].x * TILE_SIZE, map.entities[i].y * -TILE_SIZE);
    }
    buildColliders();
    GLuint tiledTexture = LoadTexture("arne_sprites.png");
    
	SDL_Event event;
	bool done = false;
    
	while (!done) {
		while (SDL_PollEvent(&event)) {
            //const Uint8 *keys = SDL_GetKeyboardState(NULL);
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
            else if(event.type == SDL_KEYDOWN){
                if(event.key.keysym.scancode == SDL_SCANCODE_A){
                    player->move(-1, elapsed);
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_D){
                    player->move(1, elapsed);
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_SPACE){
                    //player->jump();
                }
                
            }
		}
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.196078f, 0.6f, 0.8f, 0.0f);
		
        ticks = (float)SDL_GetTicks() / 1000.0f;
        elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        renderLevel(&program, tiledTexture);
        player->Draw(&program);
        
        
        glDisableVertexAttribArray(program.positionAttribute);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(view_matrix);

        SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}

