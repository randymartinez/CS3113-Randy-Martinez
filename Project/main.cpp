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
#include "FlareMap.h"

#include <algorithm>
#include <iterator>
#include <random>
#include <math.h>

#if defined(_WINDOWS) || (LINUX)
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define SPRITE_WIDTH 256
#define SPRITE_HEIGHT 512
#define MAP_SIZE_X 30
#define MAP_SIZE_Y 30
#define MAX_ROOMS 10

#define LEVEL_HEIGHT 12
#define LEVEL_WIDTH 12
#define SPRITE_COUNT_X 16
#define SPRITE_COUNT_Y 16
#define TILE_SIZE_X 0.5f
#define TILE_SIZE_Y 1.0f

#define CHAR_COUNT_X 5
#define CHAR_COUNT_Y 5
#define CHAR_SIZE_X 0.7165f
#define CHAR_SIZE_Y 1.0f
// 256, 512
// 273, 381

#define PI 3.14159265

SDL_Window* displayWindow;
GLuint LoadTexture(const char *filePath);

//unsigned int floorData[LEVEL_HEIGHT][LEVEL_WIDTH];
//unsigned int wallData[LEVEL_HEIGHT][LEVEL_WIDTH];
//unsigned int objectData[LEVEL_HEIGHT][LEVEL_WIDTH];

Matrix model_matrix;
Matrix view_matrix;
FlareMap map;
using namespace std;

class Vector2{
public:
    int x,y;
    Vector2(){}
    Vector2(int A, int B){x = A; y = B;}
    
};

struct Door{
    int x = 10;
    int y = 0;
    int closeID = 181;
    int openID = 185;
    bool isOpen = false;
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

class Entity{
    GLuint texture;
    float sheetW= 240; 
    float sheetH = 348;
    int cur_pose;
    bool flip_pose = false;
    pair<int,int> poses[11];
    float width = 25;
    float height = 35;
    float scale;
    SheetSprite sprite;
    Vector2 pos;
    Matrix model;
    bool isAlert;
    bool isActive;
public:
    Entity(GLuint textureID, int x, int y, bool isPlayer){
        int spritey = 279;
        if(isPlayer)
            spritey = 0;
        
        isActive = true;
        scale = 0.25;
        pos.x = x;
        pos.y = y;
        poses[0] = make_pair(35, spritey); // left
        poses[1] = make_pair(62, spritey); // SW
        poses[2] = make_pair(93, spritey); // down
        poses[3] = make_pair(124, spritey);// NW
        poses[4] = make_pair(153, spritey);// up
        cur_pose = 1;

        sprite = SheetSprite(textureID, poses[cur_pose].first/sheetW, poses[cur_pose].second/sheetH, width/sheetW, height/sheetH, 1.0f);


    }

    void state(){
    }
    
    bool move(int x, int y, vector<unsigned int**> mapData){
        if(pos.x == x && pos.y == y){//Same position
            //printf("1\n");
            return false;
        }
        if(mapData[1][y][x] != 0){//An object occupies that space
            //printf("2\n");
            //printf("%d\n", mapData[1][y][x]);
            return false;
        }
        else if(mapData[2][y][x] == 0 && mapData[2][pos.y][pos.x] == 0){//No walls at all
            //printf("3\n");
            pos.x = x;
            pos.y = y;
            return true;
        }
        
        int next_id = mapData[2][y][x]; //Space to be moved to
        int curr_id = mapData[2][pos.y][pos.x]; //Current space
        
        int dir; //SE NE SW NW
        int dir2; //opposite direction
        int tmpx = pos.x - x;
        int tmpy = pos.y - y;
        
        //Finds direction
        if(tmpx == -1 && tmpy == 0){//SE 
            dir = 0;
            dir2 = 3;
        }
        else if(tmpx == 0 && tmpy == 1){//NE
            dir = 1;
            dir2 = 2;
        }
        else if(tmpx == 0 && tmpy == -1){//SW
            dir = 2;
            dir2 = 1;
        }
        else if(tmpx == 1 && tmpy == 0){//NW
            dir = 3;
            dir2 = 0;
        }
        else{//invalid target
            //printf("4\n");
            return false;
        }
        
        bool ids_1 = 112 <= curr_id && curr_id <= 127;
        bool ids_2 = 132 <= curr_id && curr_id <= 143;
        bool ids_3 = 148 <= curr_id && curr_id <= 155;
        bool ids_4 = 176 <= curr_id && curr_id <= 183;
        bool ids_5 = 188 <= curr_id && curr_id <= 203;

        if(ids_1 || ids_2 || ids_3 || ids_4 || ids_5){//Checks if the id is a valid wall
            if(curr_id % 4 == dir || next_id % 4 == dir2){//If a wall obstructs this direction, fail
                //printf("5 curr:%d next:%d \n", curr_id, next_id);
                return false;
            }
        }
        //printf("6\n");
        printf("%d \n", dir);
        if(dir == 0){//SE
            cur_pose = 1;
            flip_pose = true;
        }
        else if(dir == 1){//NE
            cur_pose = 3;
            flip_pose = true;
        }
        else if(dir == 2){//SW
            cur_pose = 1;
            flip_pose = false;
        }
        else{//NW
            cur_pose = 3;
            flip_pose = false;
        }

        sprite = SheetSprite(texture, poses[cur_pose].first/sheetW, poses[cur_pose].second/sheetH, width/sheetW, height/sheetH, 1.0f);
  
        pos.x = x;
        pos.y = y;
        return true;
    }
    
    void Draw(ShaderProgram* program){
        if(isActive){
            model.Identity();
            float rotx = ((pos.x - pos.y));
            float roty = ((pos.x + pos.y));
            //model.Translate(pos.x/12 , -pos.y/12, 0.0f);
            model.Translate((rotx/4.0f) + 0.275f , (-roty/7.0f) -0.75f, 0.0f);
            if(flip_pose)
                model.Scale(-scale, scale, 1.0f);
            else
                model.Scale(scale, scale, 1.0f);
            program->SetModelMatrix(model);
            sprite.Draw(program);
        }
    }
};

class PlayerController{
    Entity* character;
};

void renderLevel(int x, int y, vector<unsigned int**> mapData, std::vector<float>* vertexData, std::vector<float>* texCoordData){
    for(unsigned int i = 0; i < mapData.size(); i++){
        if(mapData[i][y][x] == 0){continue;}
        float u = (float)(((int)(mapData[i][y][x])) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
        float v = (float)(((int)(mapData[i][y][x])) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
        float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
        float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
        float rotX = (x - y) * (TILE_SIZE_X);
        float rotY = (x + y) * (TILE_SIZE_Y/7);

        vertexData->insert(vertexData->end(), {
            TILE_SIZE_X * rotX, -TILE_SIZE_Y * rotY,
            TILE_SIZE_X * rotX, (-TILE_SIZE_Y * rotY) - TILE_SIZE_Y,
            (TILE_SIZE_X * rotX) + TILE_SIZE_X, (-TILE_SIZE_Y * rotY) - TILE_SIZE_Y,
            TILE_SIZE_X * rotX, -TILE_SIZE_Y * rotY,
            (TILE_SIZE_X * rotX) + TILE_SIZE_X, (-TILE_SIZE_Y * rotY) - TILE_SIZE_Y,
            (TILE_SIZE_X * rotX) + TILE_SIZE_X, -TILE_SIZE_Y * rotY
            });
        texCoordData->insert(texCoordData->end(), {
            u, v,
            u, v + (spriteHeight),
            u + spriteWidth, v + (spriteHeight),
            u, v,
            u + spriteWidth, v + (spriteHeight),
            u + spriteWidth, v
            });
    }
}

void sortRender(ShaderProgram* program, GLuint texture, std::vector<unsigned int**> mapData) {
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
    
    for(int x = 0; x < LEVEL_WIDTH; x++){
        if(x < LEVEL_WIDTH/2){
            renderLevel(x, x, mapData, &vertexData, &texCoordData);
        }
        int tmpX = x;
        int tmpY = 0;
        while( tmpX != tmpY && tmpX >= 0 && tmpX >= 0){
            //render tmpX, tmpY
            renderLevel(tmpX, tmpY, mapData, &vertexData, &texCoordData);
            renderLevel(tmpY, tmpX, mapData, &vertexData, &texCoordData);
            tmpX--;
            tmpY++;
        }
    }

    for(int x = 0; x < LEVEL_WIDTH; x++){
        if(x >= LEVEL_WIDTH/2){
            renderLevel(x, x, mapData, &vertexData, &texCoordData);
        }
        int tmpX = x;
        int tmpY = LEVEL_HEIGHT - 1;
        while( tmpX != tmpY && tmpX < LEVEL_WIDTH && tmpX < LEVEL_HEIGHT){
            renderLevel(tmpX, tmpY, mapData, &vertexData, &texCoordData);
            renderLevel(tmpY, tmpX, mapData, &vertexData, &texCoordData);
            tmpX++;
            tmpY--;
        }
    }

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);

	model_matrix.Identity();
	program->SetModelMatrix(model_matrix);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLES, 0, vertexData.size() / 2);
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
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    #if defined(_WINDOWS) || (LINUX)
        glewInit();
    #endif

    glViewport(0, 0, 1280, 720);
    ShaderProgram program;
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    float lastFrameTicks = 0.0f;
    float ticks = 0.0f;
    float elapsed = 0;
    
    Matrix projectionMatrix;
    
    projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    //view_matrix.Translate(-1.0f,2.0f,0.0f);
    //view_matrix.Scale(0.5f, 0.5f, 1.0f);
    view_matrix.Translate(1.0f, 1.0f, 0.0f);
    glUseProgram(program.programID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(program.programID);

    GLuint mapTexture;
    GLuint charTexture2;
    GLuint charTexture1;
    
    charTexture2 = LoadTexture("CharacterSprites.png");
    //charTexture1 = LoadTexture("CharacterSprites.png");
    
    mapTexture = LoadTexture("SpriteSheet.png");
    map.Load("Map1.txt");
    
    
    Entity player(charTexture1, 0, 10, true);
    Entity enemy(charTexture2, 0, 0, false);
    
	SDL_Event event;
	bool done = false;
    
    program.SetViewMatrix(view_matrix);
    
    Door d;
    
    std::vector<unsigned int**> layers;
    layers.push_back((map.floorData)); //Furthest Back Layer
    layers.push_back((map.objectData));
    layers.push_back((map.wallData)); //Closest Layer
    
	while (!done) {
		while (SDL_PollEvent(&event)) {
            //const Uint8 *keys = SDL_GetKeyboardState(NULL);
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}

		}
        

        ticks = (float)SDL_GetTicks() / 1000.0f;
        elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;

		glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.196078f, 0.6f, 0.8f, 0.0f);
        
        sortRender(&program, mapTexture, layers);
        
        //enemy.move(0, 1, layers);
        //player.move(0,9, layers);
        
        enemy.Draw(&program);
        player.Draw(&program);
        
        glDisableVertexAttribArray(program.positionAttribute);
        program.SetProjectionMatrix(projectionMatrix);


        SDL_GL_SwapWindow(displayWindow);

	}
	SDL_Quit();
	return 0;
}








