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

#if defined(_WINDOWS) || (LINUX)
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;
GLuint LoadTexture(const char *filePath);
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
    ShaderProgram program1;
    ShaderProgram program2;
    program1.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    program2.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    GLuint emojiTexture = LoadTexture(RESOURCE_FOLDER"emoji.png");
    GLuint pizzaTexture = LoadTexture(RESOURCE_FOLDER"sterling_john.jpg");
    GLuint duaneTexture = LoadTexture(RESOURCE_FOLDER"Duane.png");
    
    Matrix projectionMatrix;
    Matrix modelMatrix1;
    Matrix modelMatrix2;
    Matrix modelMatrix3;
    Matrix viewMatrix;
    
    projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    
    glUseProgram(program1.programID);
    glUseProgram(program2.programID);
    
    modelMatrix2.Translate(1,1,0);
    modelMatrix3.Translate(-2,1,0);
        

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);
        
        program1.SetModelMatrix(modelMatrix1);
        glBindTexture(GL_TEXTURE_2D, emojiTexture);
        float vertices1[] = {-0.3f, -0.3f, 0.3f, -0.3f, 0.3f, 0.3f, -0.3f, -0.3f, 0.3f, 0.3f, -0.3f, 0.3f};
        glVertexAttribPointer(program1.positionAttribute, 2, GL_FLOAT, false, 0, vertices1);
        glEnableVertexAttribArray(program1.positionAttribute);
        float texture1[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        glVertexAttribPointer(program1.texCoordAttribute, 2, GL_FLOAT, false, 0, texture1);
        glEnableVertexAttribArray(program1.texCoordAttribute);
        modelMatrix1.Rotate(1.0f * (3.1415926f / 180.0f));
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        
        program1.SetModelMatrix(modelMatrix2);
        glBindTexture(GL_TEXTURE_2D, pizzaTexture);
        float vertices2[] = {-0.3f, -0.3f, 0.3f, -0.3f, 0.3f, 0.3f, -0.3f, -0.3f, 0.3f, 0.3f, -0.3f, 0.3f};
        glVertexAttribPointer(program1.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
        glEnableVertexAttribArray(program1.positionAttribute);
        float texture2[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        glVertexAttribPointer(program1.texCoordAttribute, 2, GL_FLOAT, false, 0, texture2);
        glEnableVertexAttribArray(program1.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        
        program1.SetModelMatrix(modelMatrix3);
        glBindTexture(GL_TEXTURE_2D, duaneTexture);
        float vertices3[] = {-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f};
        glVertexAttribPointer(program1.positionAttribute, 2, GL_FLOAT, false, 0, vertices3);
        glEnableVertexAttribArray(program1.positionAttribute);
        float texture3[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        glVertexAttribPointer(program1.texCoordAttribute, 2, GL_FLOAT, false, 0, texture3);
        glEnableVertexAttribArray(program1.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);        

        
        
        glDisableVertexAttribArray(program1.positionAttribute);
        glDisableVertexAttribArray(program1.texCoordAttribute);
        
        
        //program2.SetModelMatrix(modelMatrix3);
        program1.SetProjectionMatrix(projectionMatrix);
        program1.SetViewMatrix(viewMatrix);
        
        

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







