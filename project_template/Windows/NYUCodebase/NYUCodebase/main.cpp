#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

class Entity{
public:

	Entity(ShaderProgram& shadeProgram):program(shadeProgram){
		projection.setOrthoProjection(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
	}

	void Draw(float vertices[]){
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);

		if (vertices[0] != vertices[2]){
			width = abs(vertices[0] - vertices[2]);
		}
		else{
			width = abs(vertices[0] - vertices[4]);
		}
		if (vertices[1] != vertices[3]){
			height = abs(vertices[1] - vertices[3]);
		}
		else{
			height = abs(vertices[1] - vertices[5]);
		}
	};

	void setMatrix(){
		program.setModelMatrix(model);
		program.setProjectionMatrix(projection);
		program.setViewMatrix(view);
	};

	void setIdentityMatrix(){
		model.identity();
	}

	void moveLeft(float amount){
		x = -1 * amount;
		model.Translate(x, 0.0, 0.0);
	};

	void moveRight(float amount){
		x = amount;
		model.Translate(x, 0.0, 0.0);
	};

	void moveUp(float amount){
		float val = amount;
		y += val;
		model.Translate(0.0, val, 0.0);
	};

	void moveDown(float amount){
		float val = -1 * amount;
		y += val;
		model.Translate(0.0, val, 0.0);
	};

	void move(float xAxis, float yAxis){
		x += xAxis;
		y += yAxis;
		model.Translate(x, y, 0.0);
	}

	float x = 0;
	float y = 0;
	float rotation;
	
	int textureID;

	float width;
	float height;

	float speed;
	float directionX = 1.0;
	float directionY = 1.0;

	ShaderProgram program;
	Matrix model, projection, view;
};

class Time{

	float elapsed;
	float lastFrameTicks = 0.0f;
	float ticks;
public:
	float getTime(){
		ticks = (float)SDL_GetTicks() / 1000.0f;
		elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		return elapsed;
	}
};

void movePaddles(Entity& leftPaddle, Entity& rightPaddle, float &elapsed){
	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	//left paddle
	if (keys[SDL_SCANCODE_W]){
		leftPaddle.moveUp(0.7 * elapsed);
	}
	else if (keys[SDL_SCANCODE_S]){
		leftPaddle.moveDown(0.7 * elapsed);
	}

	if (keys[SDL_SCANCODE_UP]){
		rightPaddle.moveUp(0.7 * elapsed);
	}
	else if (keys[SDL_SCANCODE_DOWN]){
		rightPaddle.moveDown(0.7 * elapsed);
	}
}


void moveBall(Entity& ball, float elapsed){
	ball.setIdentityMatrix();
	ball.move(0.7 * elapsed * ball.directionX, 0.7 * elapsed * ball.directionY);
}

void detectCollision(Entity& ball, Entity& leftPaddle, Entity& rightPaddle, float elapsed){
	if (ball.y >= 0.99 || ball.y <= -0.99){
		ball.directionY *= -1;
	}

	if ((ball.x + ball.width/2) >= (rightPaddle.x - rightPaddle.width / 2) && 
		(ball.x - ball.width/2) <= (rightPaddle.x + rightPaddle.width / 2) &&
		(ball.y - ball.height/2) <= (rightPaddle.y + rightPaddle.height/2) &&
		(ball.y + ball.height / 2) >= (rightPaddle.y - rightPaddle.height / 2) ){
		ball.directionX *= -1;
	}

	if ((ball.x + ball.width / 2) >= (leftPaddle.x - leftPaddle.width / 2) &&
		(ball.x - ball.width / 2) <= (leftPaddle.x + leftPaddle.width / 2) &&
		(ball.y - ball.height / 2) <= (leftPaddle.y + leftPaddle.height / 2) &&
		(ball.y + ball.height / 2) >= (leftPaddle.y - leftPaddle.height / 2)){
		ball.directionX *= -1;
	}

};

void detectWin(Entity& ball, float& elapsed){
	if (ball.x >= 1.77 || ball.x <= -1.77){
		ball.x = 0.0;
		ball.y = 0.0;
		glClearColor((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, 1.0);
	}
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	glViewport(0, 0, 1280, 720);

	ShaderProgram program(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");

	Entity rightpaddle(program);
	rightpaddle.moveRight(1.50);

	Entity ball(program);

	Entity leftpaddle(program);
	leftpaddle.moveLeft(1.50);

	Time counter;
	float elapsed;

	float paddleVertex[] = { -0.025, -0.175,
		0.025, -0.175,
		0.025, 0.175,
		-0.025, -0.175,
		0.025, 0.175,
		-0.025, 0.175
	};

	float ballVertex[] = { -0.035, -0.035,
		0.035, -0.035,
		0.035, 0.035,
		-0.035, -0.035,
		0.035, 0.035,
		-0.035, 0.035
	};

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);

		elapsed = counter.getTime();

		rightpaddle.setMatrix();
		rightpaddle.Draw(paddleVertex);

		ball.setMatrix();
		ball.Draw(ballVertex);

		leftpaddle.setMatrix();
		leftpaddle.Draw(paddleVertex);

		detectCollision(ball, leftpaddle, rightpaddle, elapsed);
		
		movePaddles(leftpaddle, rightpaddle, elapsed);
		moveBall(ball, elapsed);

		detectWin(ball, elapsed);

		SDL_GL_SwapWindow(displayWindow);// keep at bottom
	}

	SDL_Quit();
	return 0;
}
