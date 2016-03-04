#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include <vector>
using namespace std;

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

class Entity{
public:
	Entity(ShaderProgram* shadeProgram, int texture) :program(shadeProgram), textureID(texture){ projection.setOrthoProjection(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f); }
	Entity(ShaderProgram *shadeProgram, int texture, float uCoord, float vCoord, float width, float height, float size) :program(shadeProgram), textureID(texture),
	u(uCoord), v(vCoord), width(width), height(height), size(size){
		projection.setOrthoProjection(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
	}

	void Draw(/*float vertices[]*/){
		

		/*if (vertices[0] != vertices[2]){
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
		}*/

		glBindTexture(GL_TEXTURE_2D, textureID);
		GLfloat texCoords[] = {
			u, v + height,
			u + width, v,
			u, v,
			u + width, v,
			u, v + height,
			u + width, v + height
		};

		float aspect = width / height;
		float vertices[] = {
			-0.5f * size * aspect, -0.5 * size,
			0.5f * size * aspect, 0.5 * size,
			-0.5f * size * aspect, 0.5 * size,
			0.5f * size * aspect, 0.5 * size,
			-0.5f * size * aspect, -0.5 * size,
			0.5f * size * aspect, -0.5 * size
		};
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program->positionAttribute);

		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program->texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
	};

	void setMatrix(){
		program->setModelMatrix(model);
		program->setProjectionMatrix(projection);
		program->setViewMatrix(view);
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
		model.Translate(0.0, y, 0.0);
	};

	void moveDown(float amount){
		float val = -1 * amount;
		y += val;
		model.setPosition(x, y, 0.0);
	};

	void move(float xAxis, float yAxis){
		x += xAxis;
		y += yAxis;
		model.setPosition(x, y, 0.0);
	}

	void translate(float x, float y){
		model.Translate(x, y, 0.0);
	}

	void setPosition(float xPos, float yPos){
		x = xPos;
		y = yPos;
		model.setPosition(x, y, 0.0);
	}

	float x = 0;
	float y = 0;
	float rotation = 0;
	
	int textureID;

	float size;
	float u;
	float v;
	float width;
	float height;

	float speed;
	float directionX = 1.0;
	float directionY = 1.0;

	bool shouldRemove;

	ShaderProgram *program;
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

void movePlayer(Entity& player, float &elapsed){
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	
	if (keys[SDL_SCANCODE_RIGHT]){
		player.setIdentityMatrix();
		player.translate(0, -0.8);
		player.move(0.5 * elapsed, 0.0);
	}
	else if (keys[SDL_SCANCODE_LEFT]){
		player.setIdentityMatrix();
		player.translate(0, -0.8);
		player.move(-0.5 * elapsed, 0.0);
	}
}


void moveEnemy(Entity& enemy, float elapsed){
	if (enemy.x >= 1.7 || enemy.x <= -1.7){
		if (enemy.x > 1.7){
			enemy.setPosition(1.6999, enemy.y);
		}
		else{
			enemy.setPosition(-1.6999, enemy.y);
		}
		enemy.directionX *= -1;
		enemy.moveDown(0.1);
	}
	enemy.move(0.4 * elapsed * enemy.directionX, 0.0);
}

void moveBullet(Entity& bullet, float elapsed, bool playerBullet){
	if (playerBullet){
		bullet.move(0.0, 0.6 * elapsed);
		if (bullet.y > 0.99){
			bullet.shouldRemove = true;
		}
	}
	else{
		bullet.move(0.0, -0.6 * elapsed);
		if (bullet.y < -0.99){
			bullet.shouldRemove = true;
		}
	}
}

void fireBullet(Entity& ship, vector<Entity>* bullets, bool playerBullet){
	if (playerBullet){
		Entity temp = Entity(ship.program, ship.textureID, 856 / 1024.0, 869 / 1024.0, 9 / 1024.0, 57 / 1024.0, 0.1);
		temp.setPosition(ship.x, ship.y);
		bullets->push_back(temp);
	}
	else{
		Entity temp = Entity(ship.program, ship.textureID, 856 / 1024.0, 602 / 1024.0, 9 / 1024.0, 37 / 1024.0, 0.1);
		temp.setPosition(ship.x, ship.y);
		bullets->push_back(temp);
	}
}

void enemyFires(vector<Entity>* enemies, vector<Entity> *enemyBullets){
	int randomNum;
	if (enemies->size() > 1){
		randomNum = rand() % (enemies->size() - 1);
	}
	else{
		randomNum = 0;
	}
	fireBullet((*enemies)[randomNum], enemyBullets, false);
}

void detectCollision(Entity& bullet, Entity& ship, float elapsed){
	if ((bullet.x + bullet.width / 2) >= (ship.x - ship.width / 2) &&
		(bullet.x - bullet.width / 2) <= (ship.x + ship.width / 2) &&
		(bullet.y - bullet.height / 2) <= (ship.y + ship.height / 2) &&
		(bullet.y + bullet.height / 2) >= (ship.y - ship.height / 2)){
		bullet.shouldRemove = true;
		ship.shouldRemove = true;
	}

};
//
//void detectWin(Entity& ball, float& elapsed){
//	if (ball.x >= 1.77 || ball.x <= -1.77){
//		ball.x = 0.0;
//		ball.y = 0.0;
//		glClearColor((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, 1.0);
//	}
//}

GLuint LoadTexture(const char *image_path) {
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

void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	for (int i = 0; i < text.size(); i++) {
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}
	glUseProgram(program->programID);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void titleScreen(int& state, SDL_Event& event){
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	GLuint fontsheet = LoadTexture("font1.png");

	Entity text = Entity(&program, fontsheet);
	text.setPosition(-1.3, 0);
	
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
				state = 3;
			}
			else if (event.type == SDL_KEYDOWN){
				if (event.key.keysym.scancode == SDL_SCANCODE_RETURN){
					done = true;
					state = 1;					
				}
			}
		}

		text.setMatrix();
		
		DrawText(&program, fontsheet, "Space Invaders press enter", 0.1, 0.0);
		

		SDL_GL_SwapWindow(displayWindow);
	}
}

void runGame(int& state, SDL_Event& event){
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	GLuint spriteSheet = LoadTexture("sheet.png");

	Entity player = Entity(&program, spriteSheet, 211.0 / 1024.0, 941 / 1024.0, 99 / 1024.0, 75 / 1024.0, 0.1);
	player.setPosition(0.0, -0.8);
	//Entity playerBullet = Entity(&program, spriteSheet, 856 / 1024.0, 869 / 1024.0, 9 / 1024.0, 57 / 1024.0, 0.1);
	//Entity enemyBullet = Entity(&program, spriteSheet, 856 / 1024.0, 602 / 1024.0, 9 / 1024.0, 37 / 1024.0, 0.1);
	//Entity enemy = Entity(&program, spriteSheet, 425 / 1024.0, 384 / 1024.0, 93 / 1024.0, 84 / 1024.0, 0.1);
	//Entity enemy2 = Entity(&program, spriteSheet, 425 / 1024.0, 384 / 1024.0, 93 / 1024.0, 84 / 1024.0, 0.1);

	vector<Entity> playerBullets;
	vector<Entity> enemies;
	vector<Entity> enemyBullets;

	for (int i = 0, j = 0, l = 0; i < 20; ++i, ++j){
		Entity temp = Entity(&program, spriteSheet, 425 / 1024.0, 384 / 1024.0, 93 / 1024.0, 84 / 1024.0, 0.1);
		if (j == 5){
			j = 0;
		}
		if (j == 0){
			++l;
		}
		enemies.push_back(temp);
		enemies[i].setPosition((1.77 / 5) * j, (0.99 / 5) * l);

	}

	Time counter;
	float elapsed;

	//glUseProgram(program.programID);

	//SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
				state = 3;
			}
			else if (event.type == SDL_KEYDOWN){
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE){
					fireBullet(player, &playerBullets, true);
					enemyFires(&enemies, &enemyBullets);
				}
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);

		elapsed = counter.getTime();

		for (int i = 0; i < enemies.size(); ++i){
			enemies[i].setMatrix();
			enemies[i].Draw();
			moveEnemy(enemies[i], elapsed);
		}

		player.setMatrix();
		player.Draw();

		movePlayer(player, elapsed);

		for (int i = 0; i < playerBullets.size(); ++i){
			playerBullets[i].setMatrix();
			playerBullets[i].Draw();
			moveBullet(playerBullets[i], elapsed, true);
			for (int j = 0; j < enemies.size(); ++j){
				detectCollision(playerBullets[i], enemies[j], elapsed);
			}
		}

		for (int i = 0; i < enemyBullets.size(); ++i){
			enemyBullets[i].setMatrix();
			enemyBullets[i].Draw();
			moveBullet(enemyBullets[i], elapsed, false);
			detectCollision(enemyBullets[i], player, elapsed);
		}

		for (int i = 0; i < playerBullets.size(); ++i){
			if (playerBullets[i].shouldRemove){
				playerBullets.erase(playerBullets.begin() + i);
				--i;
			}
		}
		for (int i = 0; i < enemies.size(); ++i){
			if (enemies[i].shouldRemove){
				enemies.erase(enemies.begin() + i);
				--i;
			}
		}

		for (int i = 0; i < enemyBullets.size(); ++i){
			if (enemyBullets[i].shouldRemove){
				enemyBullets.erase(enemyBullets.begin() + i);
				--i;
			}
		}

		if (player.shouldRemove){
			state = 2;
			break;
		}

		SDL_GL_SwapWindow(displayWindow);// keep at bottom
	}
}

void gameOver(int& state, SDL_Event& event){
	
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

	

	enum gameState { MAIN, GAME, OVER, CLOSE};
	int state = MAIN;

	bool done = false;
	SDL_Event event;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		switch (state){
		case MAIN:
			titleScreen(state, event);
		case GAME:
			runGame(state, event);
		case OVER:
			gameOver(state, event);
		case CLOSE:
			done = true;
			break;
		}
	}

	SDL_Quit();
	return 0;
}
