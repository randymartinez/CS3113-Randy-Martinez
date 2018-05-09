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
#include "FMap.h"

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

float lerp(float v0, float v1, float t);
void pause_screen();
void game_over();
Matrix model_matrix;
Matrix view_matrix;
Matrix projectionMatrix;
FlareMap map;
FMap text;
class Entity;

ShaderProgram program;
std::vector<unsigned int**> layers;
Entity* entities[LEVEL_HEIGHT][LEVEL_WIDTH];

GLuint characterSprites;

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
    int obj_id;
    int wall_id;
    float sheetW= 240; 
    float sheetH = 348;
    int cur_pose;
    bool flip_pose = false;
    pair<int,int> poses[11];
    float width = 25;
    float height = 35;
    float scale;
    SheetSprite sprite;
    Matrix model;
    bool isAlert;
    bool isActive;
    float velocity = 1.0f;
public:
    int AP;
    Vector2 pos;
    Vector2 prevPos;
    bool isPlayer;
    Entity(int x, int y, bool isP){
        isPlayer = isP;
        int spritey = 279;
        if(isPlayer)
            spritey = 0;
        AP = 0;
        isActive = true;
        scale = 0.25;
        pos.x = x;
        pos.y = y;
        prevPos.x = x;
        prevPos.y = y;
        wall_id = layers[1][pos.y][pos.x];
        obj_id = layers[2][pos.y][pos.x];
        poses[0] = make_pair(35, spritey); // left
        poses[1] = make_pair(62, spritey); // SW
        poses[2] = make_pair(93, spritey); // down
        poses[3] = make_pair(124, spritey);// NW
        poses[4] = make_pair(153, spritey);// up
        cur_pose = 1;

        sprite = SheetSprite(characterSprites, poses[cur_pose].first/sheetW, poses[cur_pose].second/sheetH, width/sheetW, height/sheetH, 1.0f);


    }

    void search(){
        int prevX = pos.x;
        int prevY = pos.y;
        
        for(int i = 0; i < AP; i++){
            int x = pos.x;
            int y = pos.y;
    
            std::mt19937 rng;
            rng.seed(std::random_device()());
            //std::uniform_int_distribution<std::mt19937::result_type> dist6(0,3);//direction
            
            vector<pair<int, int>> moves;
            
            //cout << "OLD " <<prevX << " " << prevY << endl;
            if(this->isValid(x+1, y) && !(x+1 == prevX && y == prevY)){//SE
                //cout << "SE " << x+1 << " " << y << endl;
                moves.push_back(make_pair(x+1,y));
            }
            if(this->isValid(x, y-1) && !(x == prevX && y-1 == prevY)){//NE
                //cout << "NE " << x << " " << y-1 << endl;
                moves.push_back(make_pair(x, y-1));
            }
            if(this->isValid(x, y+1) && !(x == prevX && y+1 == prevY)){//SW
                //cout << "SW " << x << " " << y+1 << endl;
                moves.push_back(make_pair(x,y+1));
            }
            if(this->isValid(x-1, y) && !(x-1 == prevX && y == prevY)){//NW
                //cout << "NW " << x-1 << " " << y << endl;
                moves.push_back(make_pair(x-1, y));
            }
            if(moves.size() == 0)
                break;
            std::uniform_int_distribution<std::mt19937::result_type> dist6(0,moves.size()-1);
            pos.x =moves[dist6(rng)].first;
            pos.y = moves[dist6(rng)].second;

            prevX = x;
            prevY = y;
        }
        AP = 0;

    }
    
    void moveCam(){//tiled to world
        float rotX = (pos.x - pos.y) * (TILE_SIZE_X);
        float rotY = (pos.x + pos.y) * (TILE_SIZE_Y/7);
        view_matrix.Translate(-rotX, rotY, 0.0f);
    }
    bool isValid(int x, int y){

        if(x > 11 || x < 0 || y > 11 || y < 0){
            //printf("ERROR: Out of Bounds\n");
            return false;
        }
        if(pos.x == x && pos.y == y){//Same position
            //printf("1\n");
            return false;
        }

        if(layers[2][y][x] != 0){//An object occupies that space
            //printf("ERROR: Object occupying space\n");
            //printf("%d\n", mapData[1][y][x]);
            return false;
        }
        else if(layers[1][y][x] == 0 && layers[1][pos.y][pos.x] == 0){//No walls at all
            //printf("SUCCESS: No walls \n");
            if(isPlayer){
                layers[2][y][x] = obj_id;
                layers[1][y][x] = wall_id;
                prevPos.x = pos.x;
                prevPos.y = pos.y;
                pos.x = x;
                pos.y = y;
                AP--;
                obj_id = layers[2][y][x];
                wall_id = layers[1][y][x];
                
                layers[2][y][x] = 0;
                layers[1][y][x] = 0;
            }
            return true;
        }
        int next_id = layers[1][y][x]; //Space to be moved to
        int curr_id = layers[1][pos.y][pos.x]; //Current space
        
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

        sprite = SheetSprite(characterSprites, poses[cur_pose].first/sheetW, poses[cur_pose].second/sheetH, width/sheetW, height/sheetH, 1.0f);
        
        bool ids_1 = 112 <= curr_id && curr_id <= 127;
        bool ids_2 = 132 <= curr_id && curr_id <= 143;
        bool ids_3 = 148 <= curr_id && curr_id <= 151;
        bool ids_4 = 176 <= curr_id && curr_id <= 183;
        bool ids_5 = 188 <= curr_id && curr_id <= 203;
        
        bool ids_edge_1 = curr_id == 152 && (dir == 0 || dir == 1); //NE SE
        bool ids_edge_2 = curr_id == 153 && (dir == 3 || dir == 1); // NW NE
        bool ids_edge_3 = curr_id == 154 && (dir == 2 || dir == 0); //SW SE
        bool ids_edge_4 = curr_id == 155 && (dir == 3 || dir == 2); // NW SW
        
        bool ids_edge_5 = next_id == 152 && (dir2 == 3 || dir2 == 2); // Next faces NW SW
        bool ids_edge_6 = next_id == 153 && (dir2 == 2 || dir2 == 0); // Next faces SW SE
        bool ids_edge_7 = next_id == 154 && (dir2 == 3 || dir2 == 1); // Next faces NW NE
        bool ids_edge_8 = next_id == 155 && (dir2 == 0 || dir2 == 1); // Next faces NE SE
        
        //cout << curr_id <<" " <<dir << endl;
        
        if(ids_edge_1 || ids_edge_2 || ids_edge_3 || ids_edge_4){ //On Corner walls
            return false;
        }

        if(ids_1 || ids_2 || ids_3 || ids_4 || ids_5){//Checks if the id is a valid wall
            if((curr_id % 4 == dir && curr_id != 0) || (next_id % 4 == dir2 && next_id != 0)){//If a wall obstructs this direction, fail
                //printf("ERROR: Walls occupies direction");
                return false;
            }
        }
        
        if(isPlayer){
            //layers[2][y][x] = obj_id;
            //layers[1][y][x] = wall_id;
            
            pos.x = x;
            pos.y = y;
            AP--;
            //obj_id = layers[2][y][x];
            //wall_id = layers[1][y][x];
            
            //layers[2][y][x] = 0;
            //layers[1][y][x] = 0;
        }
        
        //printf("SUCCESS: default\n");
        return true;
    }

    void Draw(float elapsed){
        if(isActive){
            model.Identity();
            
            float rotx = ((pos.x - pos.y)/4.0f) + 0.275f;
            float roty = ((pos.x + pos.y)/-7.0f) -0.75;
            
            /*float prevRotx = ((prevPos.x - prevPos.y)/4.0f) + 0.275f;
            float prevRoty = ((prevPos.x + prevPos.y)/-7.0f) -0.75;
            
            int tmpx = lerp(rotx, 0.0f, elapsed);
            int tmpy = lerp(roty, 0.0f, elapsed);*/
            
            model.Translate(rotx , roty, 0.0f);
            if(flip_pose)
                model.Scale(-scale, scale, 1.0f);
            else
                model.Scale(scale, scale, 1.0f);
            program.SetModelMatrix(model);
            sprite.Draw();
        }
    }
};
class PlayerController{
    Entity* character;
};

void renderLevel(int x, int y, std::vector<float>* vertexData, std::vector<float>* texCoordData){
    for(unsigned int i = 0; i < layers.size(); i++){
        if(layers[i][y][x] == 0){continue;}
        float u = (float)(((int)(layers[i][y][x])) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
        float v = (float)(((int)(layers[i][y][x])) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
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

void sortRender(GLuint texture) {
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
    
    renderLevel(0, 0, &vertexData, &texCoordData);
    for(int x = 0; x < LEVEL_WIDTH; x++){
        for(int i = 0; i < x; i++){
            renderLevel(i, x-i, &vertexData, &texCoordData);
            renderLevel(x-i, i, &vertexData, &texCoordData);
        }

    }
    for(int x = 0; x < LEVEL_WIDTH; x++){
        if(x >= LEVEL_WIDTH/2){
            renderLevel(x, x, &vertexData, &texCoordData);
        }
        int tmpX = x;
        int tmpY = LEVEL_HEIGHT - 1;
        while( tmpX != tmpY && tmpX < LEVEL_WIDTH && tmpX < LEVEL_HEIGHT){
            renderLevel(tmpX, tmpY, &vertexData, &texCoordData);
            renderLevel(tmpY, tmpX, &vertexData, &texCoordData);
            tmpX++;
            tmpY--;
        }
    }

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program.positionAttribute);
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program.texCoordAttribute);

	model_matrix.Identity();
	program.SetModelMatrix(model_matrix);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLES, 0, vertexData.size() / 2);
	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}

void renderText(GLuint texture) {
    float tile_size = 0.2;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	for (int y = 0; y < LEVEL_HEIGHT; y++) {
		for (int x = 0; x < LEVEL_WIDTH; x++) {
            if(text.mapData[y][x] == 0){
                continue;
            }
            float u = (float)(((int)text.mapData[y][x]) % 16) / (float)16;
            float v = (float)(((int)text.mapData[y][x]) / 16) / (float)16;
            float spriteWidth = 1.0f / (float)16;
            float spriteHeight = 1.0f / (float)16;
            vertexData.insert(vertexData.end(), {
                tile_size * x, -tile_size * y,
                tile_size * x, (-tile_size * y) - tile_size,
                (tile_size * x) + tile_size, (-tile_size * y) - tile_size,
                tile_size * x, -tile_size * y,
                (tile_size * x) + tile_size, (-tile_size * y) - tile_size,
                (tile_size * x) + tile_size, -tile_size * y
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
    
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program.positionAttribute);
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program.texCoordAttribute);

	model_matrix.Identity();
	program.SetModelMatrix(model_matrix);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLES, 0, vertexData.size()/2);
	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
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

void game(){
    Mix_Music *music;
    music = Mix_LoadMUS("Desert Nomads.wav");
    Mix_PlayMusic(music, -1);
    
    //Mix_Chunk *bump;
    //bump = Mix_LoadWAV("bump.wav");
    
    //Mix_Chunk *score;
    //score = Mix_LoadWAV("score.wav");
    
    view_matrix.Translate(0.0f,2.0f,0.0f);
    program.SetViewMatrix(view_matrix);
    GLuint mapTexture;
    float lastFrameTicks = 0.0f;
    float ticks = 0.0f;
    float elapsed = 0;
    float elapsed_push = elapsed;
    characterSprites = LoadTexture("CharacterSprites.png");
    //charTexture1 = LoadTexture("CharacterSprites.png");
    string maps[] = {"Map1.txt", "Map2.txt", "Map3.txt"};
    mapTexture = LoadTexture("SpriteSheet.png");
    int curr_map = 0;
    map.Load(maps[curr_map]);
    
    
    layers.push_back((map.floorData)); //Furthest Back Layer
    layers.push_back((map.wallData));
    layers.push_back((map.objectData));//Closest Layer
    
    Entity player(5, 5, true);
    Entity enemy(0, 0, false);
    entities[0][0] = &player;
    entities[4][4] = &enemy;
    //enemy.move(1,0);

	SDL_Event event;
	bool done = false;
    
    Door d;

    vector<Entity*> queue;
    queue.push_back(&player);
    queue.push_back(&enemy);
    int turn = 0;
    
    //player.move(1,10, layers);
    //player.move(2,10, layers);
          
	while (!done) {
		while (SDL_PollEvent(&event)) {
            //const Uint8 *keys = SDL_GetKeyboardState(NULL);
            
                if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                    SDL_Quit();
                    exit(0);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_P){
                    pause_screen();
                }
                if(elapsed_push > 0.5){
                    if(event.key.keysym.scancode == SDL_SCANCODE_W){//NE
                        player.isValid(player.pos.x, player.pos.y - 1);
                            //player.move(player.pos.x, player.pos.y - 1);
                    }else if(event.key.keysym.scancode == SDL_SCANCODE_A){//NW
                        player.isValid(player.pos.x - 1, player.pos.y);
                            //player.move(player.pos.x - 1, player.pos.y);
                    }else if(event.key.keysym.scancode == SDL_SCANCODE_S){//SW
                        player.isValid(player.pos.x, player.pos.y + 1);
                            //player.move(player.pos.x, player.pos.y + 1);
                    }else if(event.key.keysym.scancode == SDL_SCANCODE_D){//SE
                        player.isValid(player.pos.x + 1, player.pos.y);
                            //player.move(player.pos.x + 1, player.pos.y);
                    }
                    
                    else if(event.type == SDL_MOUSEBUTTONDOWN) {//1280, 720
                        //float unitX = ((((float)event.motion.x / 1280.0f) * 3.554f ) - 1.777f) - 0.3f;
                        //float unitY = ((((float)(720.0f-event.motion.y) / 720.0f) * 2.0f ) - 1.0f);
                        //cout << "World X " << unitX << " Y " << unitY << endl;
                        //cout << "Tile X " << mapx << " Y " << mapy<< "\n===========" << endl;
                    }
                    elapsed_push = 0;
                }


		}
        elapsed_push+= elapsed;
        if(queue[turn]->AP <= 0){
            turn = (turn+1) % (queue.size());
            queue[turn]->AP = 3;
            if(!queue[turn]->isPlayer)
                queue[turn]->search();
        }
            

        ticks = (float)SDL_GetTicks() / 1000.0f;
        elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
		glClear(GL_COLOR_BUFFER_BIT);
        //glClearColor(0.196078f, 0.6f, 0.8f, 0.0f);
        
        sortRender(mapTexture);
        
        //enemy.move(0, 1, layers);
        
        enemy.Draw(elapsed);
        player.Draw(elapsed);
        
        if(player.pos.x == enemy.pos.x && player.pos.y == enemy.pos.y){//you lose
            game_over();
        }
        if(player.pos.x == 10 && player.pos.y == 0){//Goal
            if(curr_map == 2)
                game_over();
            enemy.pos.x = 0;
            enemy.pos.y = 0;
            player.pos.x = 0;
            player.pos.y = 11;
            curr_map++;
            map.Load(maps[curr_map]);
            layers.clear();
            layers.push_back(map.floorData);
            layers.push_back(map.wallData);
            layers.push_back(map.objectData);
        }
        glDisableVertexAttribArray(program.positionAttribute);
        program.SetProjectionMatrix(projectionMatrix);


        SDL_GL_SwapWindow(displayWindow);

	}
	Mix_FreeMusic(music);
}
void start_screen(){
    GLuint fontTexture;
    fontTexture = LoadTexture("font1.png");
    text.Load("text.txt");
    SDL_Event event;
    view_matrix.Translate(-2.0f, 0.0f, 0.0f);
    program.SetViewMatrix(view_matrix);
    view_matrix.Identity();
    while(1){
        while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                    SDL_Quit();
                    exit(0);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE){
                    return;
                }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        renderText(fontTexture);
        glDisableVertexAttribArray(program.positionAttribute);
        program.SetProjectionMatrix(projectionMatrix);


        SDL_GL_SwapWindow(displayWindow);
    }
}

void pause_screen(){
    GLuint fontTexture;
    fontTexture = LoadTexture("font1.png");
    text.Load("pause.txt");
    SDL_Event event;
    //view_matrix.Translate(-2.0f, 0.0f, 0.0f);
    //program.SetViewMatrix(view_matrix);
    view_matrix.Identity();
    while(1){
        while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                    SDL_Quit();
                    exit(0);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_N){
                    return;
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_Y){
                    exit(0);
                }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        renderText(fontTexture);
        glDisableVertexAttribArray(program.positionAttribute);
        program.SetProjectionMatrix(projectionMatrix);


        SDL_GL_SwapWindow(displayWindow);
    }
}

void game_over(){
    GLuint fontTexture;
    fontTexture = LoadTexture("font1.png");
    text.Load("game_over.txt");
    SDL_Event event;
    //view_matrix.Translate(-2.0f, 0.0f, 0.0f);
    //program.SetViewMatrix(view_matrix);
    view_matrix.Identity();
    while(1){
        while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                    SDL_Quit();
                    exit(0);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE){
                    exit(0);
                }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        renderText(fontTexture);
        glDisableVertexAttribArray(program.positionAttribute);
        program.SetProjectionMatrix(projectionMatrix);


        SDL_GL_SwapWindow(displayWindow);
    }
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

    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    //view_matrix.Scale(0.5f, 0.5f, 1.0f);
    view_matrix.Translate(1.0f, 1.0f, 0.0f);
    glUseProgram(program.programID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(program.programID);
    program.SetViewMatrix(view_matrix);
    while(1){
        start_screen();
        game();
    }
    
	SDL_Quit();
	return 0;
}

float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}





