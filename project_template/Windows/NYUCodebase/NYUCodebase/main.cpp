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

GLuint LoadTexturePNG(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SDL_FreeSurface(surface);
	return textureID;
}

GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RGB,
		GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SDL_FreeSurface(surface);
	return textureID;
}

void texturize(ShaderProgram program, GLuint texture, float *vertices, float *texCoords){
	glEnable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, texture);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);
	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1440, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif
	
	glClearColor(0.4f, 0.2f, 0.4f, 1.0f);

	glViewport(0, 0, 1440, 720);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	GLuint moon = LoadTexturePNG("moon.png");
	GLuint earth = LoadTexture("earth.jpg");
	GLuint sun = LoadTexture("sun.jpeg");

	Matrix moonProjectionMatrix;
	Matrix moonModelMatrix;
	Matrix moonViewMatrix;

	Matrix earthProjectionMatrix;
	Matrix earthModelMatrix;
	Matrix earthViewMatrix;

	Matrix sunProjectionMatrix;
	Matrix sunModelMatrix;
	Matrix sunViewMatrix;

	moonProjectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	earthProjectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	sunProjectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);


	float lastFrameTicks;

	float moonVertices[] = { 0.0, 2.0,
		0.0, 1.0,
		1.0, 1.0,
		0.0, 2.0,
		1.0, 1.0,
		1.0, 2.0,
	};
	float moonTexCoords[] = { 0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
		0.0, 1.0,
		1.0, 0.0,
		0.0, 0.0 };

	float earthVertices[] = { -2.0, -0.0,
		-2.0, -0.5,
		-1.5, -0.5,
		-2.0, -0.0,
		-1.5, -0.5,
		-1.5, -0.0 };
	float earthTexCoords[] = { 0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
		0.0, 0.0,
		1.0, 1.0,
		1.0, 0.0 };

	float sunVertices[] = { -0.5, -0.5,
		0.5, -0.5,
		0.5, 0.5,
		-0.5, -0.5,
		0.5, 0.5,
		-0.5, 0.5 };
	float sunTexCoords[] = { 0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
		0.0, 0.0,
		1.0, 1.0,
		1.0, 0.0 };


	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);
		

		

		program.setModelMatrix(moonModelMatrix);
		program.setProjectionMatrix(moonProjectionMatrix);
		program.setViewMatrix(moonViewMatrix);
		//moonModelMatrix.identity();

		program.setModelMatrix(earthModelMatrix);
		program.setProjectionMatrix(earthProjectionMatrix);
		program.setViewMatrix(earthViewMatrix);
		//earthModelMatrix.identity();

		program.setModelMatrix(sunModelMatrix);
		program.setProjectionMatrix(sunProjectionMatrix);
		program.setViewMatrix(sunViewMatrix);
		//sunModelMatrix.identity();

		sunModelMatrix.Rotate(0.1 * (3.14159 / 180));

		texturize(program, moon, moonVertices, moonTexCoords);
		texturize(program, earth, earthVertices, earthTexCoords);
		texturize(program, sun, sunVertices, sunTexCoords);
		
		//float ticks = (float)SDL_GetTicks() / 1000.0f;
		//lastFrameTicks = ticks;
		//float elapsed = ticks - lastFrameTicks;
		

		SDL_GL_SwapWindow(displayWindow);

	}

	SDL_Quit();
	return 0;
}
