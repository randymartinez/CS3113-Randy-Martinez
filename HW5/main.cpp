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
#include "SatCollision.h"
#include <math.h>
#include <cmath>
#include "SatCollision.cpp"

#if defined(_WINDOWS) || (LINUX)
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define NUM_ENTITIES 3

SDL_Window* displayWindow;
void Update(ShaderProgram program, float elapsed);

class Vector3 {
public:
	Vector3() {}
	Vector3(float x_, float y_, float z_) {
		x = x_;
		y = y_;
		z = z_;
	}
	Vector3(const Vector3& test) {
		x = test.x;
		y = test.y;
		z = test.z;
	}

	void operator=(const Vector3& test) {
		this->x = test.x;
		this->y = test.y;
		this->z = test.z;
	}

	float length() {
		return sqrtf((x*x) + (y*y));
	}

	void normalize() {
		float length = sqrtf((x*x) + (y*y));
		x /= length;
		y /= length;
	}

	float x;
	float y;
	float z;
};

class Entity{
public: 
    Entity():parent(nullptr){
        velocity.x = 1.0f;
        velocity.y = 1.0f;
        rSpeed = 20.0f;
        rDirection = 1;
        angle = 45;
    }
    void walls(){
        if(position.y >= 2.0f){//Ceiling
            velocity.y *= -1.0f;
        }
        if(position.y <= -2.0f){//Floor
            velocity.y *= -1.0f;
        }
        if(position.x >= 3.55f){//Right Wall
            velocity.x *= -1.0f;
        }
        if(position.x <= -3.55f){//Left Wall
            velocity.x *= -1.0f;
        }
    }
    
    void draw(ShaderProgram program, float elapsed){
        program.SetModelMatrix(model);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        
        if(parent){
            model = model * parent->model;
        }
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    float vertices[12] = {-0.1f, -0.1f, 0.1f, -0.1f, 0.1f, 0.1f, -0.1f, -0.1f, 0.1f, 0.1f, -0.1f, 0.1f};
    Matrix model;
    Matrix matrix;
    Entity* parent;
    Vector3 velocity;
    Vector3 position;
    Vector3 size;
    float rSpeed;
    int rDirection;
    float angle;
    
};

Entity entities[NUM_ENTITIES];

void Setup(){
    entities[0].position.x = -1.0f;
    entities[0].position.y = -1.0f;
    entities[0].size.x = 1.0f;
    entities[0].size.y = 1.0f;
    
    entities[1].position.x = 0.0f;
    entities[1].position.y = 0.0f;
    entities[1].size.x = 2.0f;
    entities[1].size.y = 2.0f;
    
    entities[2].position.x = -1.0f;
    entities[2].position.y = 1.0f;
    entities[2].size.x = 3.0f;
    entities[2].size.y = 3.0f;
}

void fillVector(const Entity& e,  std::vector<std::pair<float, float>>& points) {
	std::vector<std::pair<float,float>> coord;

	coord.push_back(std::make_pair(-1.0f,1.0f));
    coord.push_back(std::make_pair(1.0f,1.0f));
    coord.push_back(std::make_pair(1.0f,-1.0f));
    coord.push_back(std::make_pair(-1.0f,-1.0f));
    

	for (size_t index = 0; index < coord.size(); ++index) {
		std::pair<float, float> temp;
		temp.first = coord[index].first;
		temp.second = coord[index].second;
        temp.first = e.matrix.m[0][0] * temp.first + e.matrix.m[1][0] * temp.second + e.matrix.m[2][0] * 0 + e.matrix.m[3][0] * 1.0f;
		temp.second = e.matrix.m[0][1] * temp.first + e.matrix.m[1][1] * temp.second + e.matrix.m[2][1] * 0 + e.matrix.m[3][1] * 1.0f;
		points.push_back(temp);
	}
}


void Update(ShaderProgram program, float elapsed){
    Matrix temp_translate;
    
    std::vector<std::pair<float, float>> e1Points;
    std::vector<std::pair<float, float>> e2Points;
    std::vector<std::pair<float, float>> e3Points;
    
    fillVector(entities[0], e1Points);
	fillVector(entities[1], e2Points);
	fillVector(entities[2], e3Points);
    
    
    for(int i = 0; i < NUM_ENTITIES; i++){
        entities[i].model.Identity();
        entities[i].position.x += (entities[i].velocity.x * elapsed);
        entities[i].position.y += (elapsed * entities[i].velocity.y);
        entities[i].model.Translate(entities[i].position.x, entities[i].position.y, 0.0f);
        entities[i].angle += std::fmod((elapsed * entities[i].rSpeed * entities[i]. rDirection), 360);
        entities[i].model.Rotate(entities[i].angle * acos(-1) /180);
        entities[i].walls();
        entities[i].model.Scale(entities[i].size.x, entities[i].size.y, 1.0f);
        entities[i].draw(program, elapsed);
    }
	std::pair<float,float> penetration12;
	std::pair<float,float> penetration13;
	std::pair<float,float> penetration23;
    
    //undefined reference to `CheckSATCollision(std::vector<std::pair<float, float>, std::allocator<std::pair<float, float> > > const&, std::vector<std::pair<float, float>, std::allocator<std::pair<float, float> > > const&, std::pair<float, float>&)'

    
	//test for Entity one and Entity two
	if (CheckSATCollision(e1Points, e2Points, penetration12)){
		if (penetration12.first < 0) { penetration12.first *= -1; }
		if (penetration12.second < 0) { penetration12.second *= -1; }
		temp_translate.Identity();
		entities[1].model.Translate(-penetration12.first / 2, penetration12.second/2, 0.0f);
		temp_translate.SetPosition(-penetration12.first / 2, penetration12.second/2, 0.0f);
		entities[1].matrix = temp_translate * entities[1].matrix;
	}
	
	//test for Entity one and Entity three
	if (CheckSATCollision(e1Points, e3Points, penetration13)) {
		if (penetration13.first < 0) { penetration13.first *= -1; }
		if (penetration13.second < 0) { penetration13.second *= -1; }
		temp_translate.Identity();
		entities[1].model.Translate(-penetration13.first / 2, penetration13.second/2, 0.0f);
		temp_translate.SetPosition(-penetration13.first / 2, penetration13.second/2, 0.0f);
		entities[1].matrix = temp_translate * entities[1].matrix;
		entities[1].rDirection *= -1;

		temp_translate.Identity();
		entities[3].model.Translate(penetration13.first / 2, -penetration13.second/2, 0.0f);
		temp_translate.SetPosition(penetration13.first / 2, -penetration13.second/2, 0.0f);
		entities[3].matrix = temp_translate * entities[3].matrix;
		entities[3].rDirection *= -1; 
	}
	
	//test for Entity two and Entity two
	if (CheckSATCollision(e2Points, e3Points, penetration23)) {
		if (penetration23.first < 0) { penetration23.first *= -1; }
		if (penetration23.second < 0) { penetration23.second *= -1; }

		temp_translate.Identity();
		entities[3].rDirection *= -1;
		entities[3].model.Translate((penetration23.first/2), -(penetration23.second/2), 0.0f);
		temp_translate.SetPosition((penetration23.first/2), -(penetration23.second/2), 0.0f);
		entities[3].matrix = temp_translate * entities[3].matrix;
	}

	//clears the vectors of the edges
	//e1Points.clear();
	//e2Points.clear();
	//e3Points.clear();
    
}

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
    ShaderProgram program;
    program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    float lastFrameTicks = 0.0f;
    float ticks = 0.0f;
    float elapsed = 0;
    
    Matrix projectionMatrix;
    
    Matrix view;
    program.SetViewMatrix(view);
    projectionMatrix.SetOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    
    glUseProgram(program.programID);

	SDL_Event event;
	bool done = false;
    Setup();
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
        
        Update(program, elapsed);

        glDisableVertexAttribArray(program.positionAttribute);
        program.SetProjectionMatrix(projectionMatrix);


        SDL_GL_SwapWindow(displayWindow);

	}

	SDL_Quit();
	return 0;
}








