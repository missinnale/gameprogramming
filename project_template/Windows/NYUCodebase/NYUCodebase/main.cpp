#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include <vector>
#include <SDL_mixer.h>
using namespace std;

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

class Entity{
public:
	Entity(ShaderProgram* shadeProgram, int texture) :program(shadeProgram), textureID(texture){ projection.setOrthoProjection(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f); sprite = false; }
	Entity(ShaderProgram *shadeProgram, int texture, float uCoord, float vCoord, float width, float height, float size) :program(shadeProgram), textureID(texture),
	u(uCoord), v(vCoord), width(width), height(height), size(size){
		//projection.setOrthoProjection(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
		projection.setOrthoProjection(-2.777f, 2.777f, -2.0f, 2.0f, -2.0f, 2.0f);
		sprite = true;
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
			0, 1,
			1, 0,
			0, 0,
			1, 0,
			0, 1,
			1, 1
		};
		if (sprite){
			texCoords[0] = u;
			texCoords[1] = v + height;
			texCoords[2] = u + width;
			texCoords[3] = v;
			texCoords[4] = u;
			texCoords[5] = v;
			texCoords[6] = u + width;
			texCoords[7] = v;
			texCoords[8] = u;
			texCoords[9] = v + height;
			texCoords[10] = u + width;
			texCoords[11] = v + height;
		}

		float aspect = width / height;
		/*float vertices[] = {
			-0.5f * size * aspect, -0.5 * size,
			0.5f * size * aspect, 0.5 * size,
			-0.5f * size * aspect, 0.5 * size,
			0.5f * size * aspect, 0.5 * size,
			-0.5f * size * aspect, -0.5 * size,
			0.5f * size * aspect, -0.5 * size
		};*/
		float vertices[] = {
			-width/2 * size, -height/2 * size,
			width/2 * size, height/2 * size,
			-width/2 * size, height/2 * size,
			width/2 * size, height/2 * size,
			-width/2 * size, -height/2 * size,
			width/2 * size, -height/2 *size
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
		model.Translate(xAxis, yAxis, 0.0);
	}

	void translate(float x, float y){
		model.Translate(x, y, 0.0);
	}

	void setPosition(float xPos, float yPos){
		x = xPos;
		y = yPos;
		model.setPosition(x, y, 0.0);
	}

	void rotate(float value){
		rotation += value * 180/3.14159;
		model.Rotate(value);
	}

	void checkLocation(){
		model.Rotate(-(rotation * 3.14159/180));

		if (!onScreen){
			if (x < 1.77 && x > -1.77 && y < 1 && y > -1){
				onScreen = true;
			}
		}
		else{
			if (x > 1.77 && x < -1.77 && y > 1 && y < -1){
				shouldRemove = true;
			}
		}

		model.Rotate(rotation * 3.14159 / 180);
	}

	bool sprite;
	bool onScreen;

	float x = 0;
	float y = 0;
	//in degrees
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

	bool shouldRemove = false;

	ShaderProgram *program;
	Matrix model, projection, view;
};

class Time{

	float elapsed;
	
	float ticks;
public:
	float lastFrameTicks = 0.0f;
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
		//player.setIdentityMatrix();
		//player.translate(0, -0.8);
		//player.move(0.5 * elapsed, 0.0);
		player.rotate(-0.005);
	}
	else if (keys[SDL_SCANCODE_LEFT]){
		//player.setIdentityMatrix();
		//player.translate(0, -0.8);
		//player.move(-0.5 * elapsed, 0.0);
		player.rotate(0.005);
	}
	if (keys[SDL_SCANCODE_SPACE]){
		player.move(0.0, 0.25 * elapsed);
	}
}

void spawnAstroid(vector<Entity> &astroids, ShaderProgram &program, GLuint &spriteSheet){
	int astroidType = (rand() % 100 + 1)/10;
	Entity temp = Entity(&program, spriteSheet, 224 / 1024.0, 664 / 1024.0, 101 / 1024.0, 84 / 1024.0, 1.5);
	switch (astroidType)
	{
	case 1:
		temp = Entity(&program, spriteSheet, 224 / 1024.0, 664 / 1024.0, 101 / 1024.0, 84 / 1024.0, 1.5);
		break;
	case 2:
		temp = Entity(&program, spriteSheet, 0 / 1024.0, 520 / 1024.0, 120 / 1024.0, 98 / 1024.0, 1.5);
		break;
	case 3:
		temp = Entity(&program, spriteSheet, 518 / 1024.0, 810 / 1024.0, 89 / 1024.0, 82 / 1024.0, 1.5);
		break;
	case 4:
		temp = Entity(&program, spriteSheet, 327 / 1024.0, 452 / 1024.0, 98 / 1024.0, 96 / 1024.0, 1.5);
		break;
	case 5:
		temp = Entity(&program, spriteSheet, 651 / 1024.0, 447 / 1024.0, 43 / 1024.0, 43 / 1024.0, 1.5);
		break;
	case 6:
		temp = Entity(&program, spriteSheet, 237 / 1024.0, 452 / 1024.0, 45 / 1024.0, 40 / 1024.0, 1.5);
		break;
	case 7:
		temp = Entity(&program, spriteSheet, 406 / 1024.0, 234 / 1024.0, 28 / 1024.0, 28 / 1024.0, 1.5);
		break;
	case 8:
		temp = Entity(&program, spriteSheet, 778 / 1024.0, 587 / 1024.0, 29 / 1024.0, 26 / 1024.0, 1.5);
		break;
	case 9:
		temp = Entity(&program, spriteSheet, 346 / 1024.0, 814 / 1024.0, 18 / 1024.0, 18 / 1024.0, 1.5);
		break;
	case 10:
		temp = Entity(&program, spriteSheet, 399 / 1024.0, 814 / 1024.0, 16 / 1024.0, 15 / 1024.0, 1.5);
	default:
		break;
	}
	
	temp.rotate(atan2((rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f));
	//temp.rotate(-83 * 3.14159 / 180);
	astroids.push_back(temp);
	astroids[astroids.size()-1].setMatrix();
	int xVal = rand() % 190;
	int yVal = rand() % 17;
	while (xVal < 180 && yVal < 12){
		xVal = rand() % 190;
		yVal = rand() % 17;
	}
	int sign = rand() % 2;
	int sign2 = rand() % 2;
	if (sign == 0){ sign = -1; }
	else{ sign = 1; }
	if (sign2 == 0){ sign2 = -1; }
	else{ sign2 = 1; }
	
	astroids[astroids.size() - 1].setPosition(xVal/100.0 * sign , yVal/10.0 * sign2 );

	if (astroids[astroids.size() - 1].x > 1.77){
		if (astroids[astroids.size() - 1].rotation > 0 && astroids[astroids.size() - 1].rotation < 180){
			astroids[astroids.size() - 1].directionY = 1;
		}
		else if (astroids[astroids.size() - 1].rotation == 0){
			astroids[astroids.size() - 1].directionX = -1;
		}
		else if (astroids[astroids.size() - 1].rotation == 180 || astroids[astroids.size() - 1].rotation == -180){
			astroids[astroids.size() - 1].directionX = 1;
		}
		else{
			astroids[astroids.size() - 1].directionY = -1;
		}
	}
	else if (astroids[astroids.size() - 1].x < -1.77){
		if (astroids[astroids.size() - 1].rotation < 0 && astroids[astroids.size() - 1].rotation > -180){
			astroids[astroids.size() - 1].directionY = 1;
		}
		else if (astroids[astroids.size() - 1].rotation == 0){
			astroids[astroids.size() - 1].directionX = 1;
		}
		else if (astroids[astroids.size() - 1].rotation == 180 || astroids[astroids.size() - 1].rotation == -180){
			astroids[astroids.size() - 1].directionX = -1;
		}
		else{
			astroids[astroids.size() - 1].directionY = -1;
		}
	}
	if (astroids[astroids.size() - 1].y > 1){
		if (astroids[astroids.size() - 1].rotation > 90 && astroids[astroids.size() - 1].rotation > -90){
			astroids[astroids.size() - 1].directionY = 1;
			astroids[astroids.size() - 1].directionX = -1;
		}
		else if (astroids[astroids.size() - 1].rotation == 90){
			astroids[astroids.size() - 1].directionX = -1;
		}
		else if (astroids[astroids.size() - 1].rotation == -90 ){
			astroids[astroids.size() - 1].directionX = 1;
		}
		else{
			astroids[astroids.size() - 1].directionY = -1;
		}
	}
	else if (astroids[astroids.size() - 1].y < -1){
		if (astroids[astroids.size() - 1].rotation < 90 && astroids[astroids.size() - 1].rotation > -90){
			astroids[astroids.size() - 1].directionY = 1;
			astroids[astroids.size() - 1].directionX = -1;
		}
		else if (astroids[astroids.size() - 1].rotation == 90){
			astroids[astroids.size() - 1].directionX = 1;
		}
		else if (astroids[astroids.size() - 1].rotation == -90){
			astroids[astroids.size() - 1].directionX = -1;
		}
		else{
			astroids[astroids.size() - 1].directionY = -1;
		}
	}
}


void moveAstroid(Entity& astroid, float elapsed){

	astroid.move(0.1 * elapsed * astroid.directionX, 0.1 * elapsed * astroid.directionY);
	astroid.checkLocation();
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
	GLuint alien = LoadTexture("alienBlue.png");

	Mix_Chunk *killSound;
	killSound = Mix_LoadWAV("sfxKill.wav");

	Mix_Chunk *shootSound;
	shootSound = Mix_LoadWAV("sfxShoot.wav");
	//<SubTexture name="alienBlue.png" x="412" y="175" width="66" height="92"/>
	Entity player = Entity(&program, alien);
	player.width = 66 / 1024.0f;
	player.height = 92 / 1024.0f;
	player.size = 1.5;
	player.setPosition(0.0, -0.8);
	//Entity playerBullet = Entity(&program, spriteSheet, 856 / 1024.0, 869 / 1024.0, 9 / 1024.0, 57 / 1024.0, 0.1);
	//Entity enemyBullet = Entity(&program, spriteSheet, 856 / 1024.0, 602 / 1024.0, 9 / 1024.0, 37 / 1024.0, 0.1);
	//Entity enemy = Entity(&program, spriteSheet, 425 / 1024.0, 384 / 1024.0, 93 / 1024.0, 84 / 1024.0, 0.1);
	//Entity enemy2 = Entity(&program, spriteSheet, 425 / 1024.0, 384 / 1024.0, 93 / 1024.0, 84 / 1024.0, 0.1);

	vector<Entity> astroids;
	float lastAstroidSpawnTime = 0;
	
	Time counter;
	counter.lastFrameTicks = SDL_GetTicks() / 1000.0f;
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
					//fireBullet(player, &playerBullets, true);

					Mix_PlayChannel(-1, shootSound, 0);
				}
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);

		if (((float)SDL_GetTicks() / 1000.0f - lastAstroidSpawnTime) > 0.5){
			spawnAstroid(astroids, program, spriteSheet);
			lastAstroidSpawnTime = (float)SDL_GetTicks() / 1000.0f;
		}
		
		elapsed = counter.getTime();

		for (int i = 0; i < astroids.size(); ++i){
			astroids[i].setMatrix();
			astroids[i].Draw();
			moveAstroid(astroids[i], elapsed);
		}

		player.setMatrix();
		player.Draw();

		movePlayer(player, elapsed);

		/*for (int i = 0; i < playerBullets.size(); ++i){
			playerBullets[i].setMatrix();
			playerBullets[i].Draw();
			moveBullet(playerBullets[i], elapsed, true);
			for (int j = 0; j < enemies.size(); ++j){
				detectCollision(playerBullets[i], enemies[j], elapsed);
			}
		}*/

		/*for (int i = 0; i < enemyBullets.size(); ++i){
			enemyBullets[i].setMatrix();
			enemyBullets[i].Draw();
			moveBullet(enemyBullets[i], elapsed, false);
			detectCollision(enemyBullets[i], player, elapsed);
		}*/

		/*for (int i = 0; i < playerBullets.size(); ++i){
			if (playerBullets[i].shouldRemove){
				playerBullets.erase(playerBullets.begin() + i);
				--i;
			}
		}*/
		for (int i = 0; i < astroids.size(); ++i){
			if (astroids[i].shouldRemove){
				astroids.erase(astroids.begin() + i);
				--i;
				//Mix_PlayChannel(-1, killSound, 0);
			}
		}

		/*for (int i = 0; i < enemyBullets.size(); ++i){
			if (enemyBullets[i].shouldRemove){
				enemyBullets.erase(enemyBullets.begin() + i);
				--i;
			}
		}*/

		/*if (player.shouldRemove || enemies.size() == 0){
			state = 2;
			
			break;
		}*/

		SDL_GL_SwapWindow(displayWindow);// keep at bottom
	}

	Mix_FreeChunk(killSound);
	Mix_FreeChunk(shootSound);
	
}

void gameOver(int& state, SDL_Event& event){
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

		DrawText(&program, fontsheet, "Game Over: Press enter to play again", 0.08, 0.0);


		SDL_GL_SwapWindow(displayWindow);
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

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

	enum gameState { MAIN, GAME, OVER, CLOSE};
	int state = MAIN;

	Mix_Music *music;
	music = Mix_LoadMUS("spaceInvadersMusic.mp3");

	Mix_Chunk *dieSound;
	dieSound = Mix_LoadWAV("sfxDie.wav");

	bool done = false;
	SDL_Event event;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		Mix_PlayMusic(music, -1);

		switch (state){
		case MAIN:
			titleScreen(state, event);
			break;
		case GAME:
			glClear(GL_COLOR_BUFFER_BIT);
			runGame(state, event);
			break;
		case OVER:
			Mix_PlayChannel(-1, dieSound, 0);
			gameOver(state, event);
			break;
		case CLOSE:
			done = true;
			break;
		}
	}

	Mix_FreeChunk(dieSound);
	Mix_FreeMusic(music);

	SDL_Quit();
	return 0;
}
