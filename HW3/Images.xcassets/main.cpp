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
    float speed = 1.0f;
public:
    //Hitbox
    bool isActive = true;
    float top;
    float bot;
    float right;
    float left;
    float width = 9.0f/1024.0f;
    float height = 54.0f/1024.0f;
    float aspect = width/height;
    
    Bullet(GLuint texture, Vector2 pos){
        sprite = SheetSprite(texture, 856.0f/1024.0f, 421.0f/1024.0f, 9.0f/1024.0f, 54.0f/1024.0f, 1.0f);
        position = pos;
        
        top = position.y + ((yscale) * aspect * height);
        bot = position.y - ((yscale) * aspect * height);
        right = position.x + ((xscale) * aspect * width);
        left = position.x - ((xscale) * aspect * width);
        
        //top=position.y + (yscale/2);
        //bot=position.y - (yscale/2);
        //right=position.x + (xscale/2);
        //left=position.x - (xscale/2);
    }
    void Move(float elapsed){
        position.y += speed*elapsed;
        top=position.y + (yscale/2);
        bot=position.y - (yscale/2);
    }
    void Draw(ShaderProgram* program){
        //printf("top %f", top+0.6);
        //printf("bot %f", bot-0.6);
        //printf("right %f", right+0.3);
        //printf("left %f\n\n", left-0.3);
        
        if(isActive){
            model.Identity();
            model.Translate(position.x, position.y, 0.0f);
            model.Scale(xscale,yscale, 1.0f);
            program->SetModelMatrix(model);
            program->SetViewMatrix(view);
            sprite.Draw(program);
        }
    }
};

class Invader{
    Vector2 position;
    SheetSprite sprite;
    float vertices[12] = {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};
    float scale = 0.3f;
    Matrix model;
    Matrix view;
    bool isActive = true;
public:
    //Hitbox
    float top;
    float bot;
    float right;
    float left;
    float width = 93.0f/1024.0f;
    float height = 84.0f/1024.0f;
    float aspect = width/height;
    
    Invader(GLuint texture, Vector2 pos){
        sprite = SheetSprite(texture, 423.0f/1024.0f, 728.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 1.0f);
        position = pos;
        top = position.y + ((scale) * aspect * height);
        bot = position.y - ((scale) * aspect * height);
        right = position.x + ((scale) * aspect * width);
        left = position.x - ((scale) * aspect * width);
        
        //top = position.y + (scale/2);
        //bot = position.y - (scale/2);
        //right = position.x + (scale/2);
        ///left = position.x - (scale/2);
        
    }
    void Collision(Bullet* laser){
        if(((laser->bot <= top) && (laser->top >= bot)) && ((laser->right <= left) && (laser->left >= right))){        
            isActive = false;
            laser->isActive = false;
            printf("We did it!");
            //exit(1);
        }
    }
    void Draw(ShaderProgram* program){
        if(isActive){
            model.Identity();
            model.Translate(position.x, position.y, 0.0f);
            model.Scale(scale, scale, 1.0f);
            program->SetModelMatrix(model);
            program->SetViewMatrix(view);
            sprite.Draw(program);
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
    Matrix projectionMatrix;  
    ShaderProgram program;
    program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    float lastFrameTicks = 0.0f;
    float ticks = 0.0f;
    float elapsed = 0;
    
    GLuint texture = LoadTexture(RESOURCE_FOLDER"sheet.png");
    projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    
    glUseProgram(program.programID);
    
    std::vector<Invader> enemies;
    
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 11; j++){
            enemies.push_back(Invader(texture, Vector2(-2.0f + (j * 0.4f), 1.5f - (i * 0.4f))));
        }
    }
    
    
    float top;
    program.SetProjectionMatrix(projectionMatrix);
    Bullet A(texture, Vector2(0.0f, -1.5f));
    Bullet B(texture, Vector2(2.0f, -1.5f));
    
	SDL_Event event;
	bool done = false;
    int count = 0;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		
		ticks = (float)SDL_GetTicks() / 1000.0f;
        elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
		
		glClear(GL_COLOR_BUFFER_BIT);
        
        //printf("Bullet Top %f Bot %f Left %f Right %f \n",A.top, A.bot, A.left, A.right);
        
        for(Invader tmp : enemies){
            tmp.Draw(&program);
            tmp.Collision(&A);
            if(count == 49){
                top = tmp.top;
                //printf("HELLO\n");
            }
            count++;
        }
        
        A.Move(elapsed);
        A.Draw(&program);
        
        B.Move(elapsed);
        B.Draw(&program);
        
        
        

        //printf("%f\n", elapsed);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        SDL_GL_SwapWindow(displayWindow);
    }
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







