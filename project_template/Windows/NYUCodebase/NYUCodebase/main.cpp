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
#include <time.h>
#include <math.h>
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
		projection.setOrthoProjection(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
		//projection.setOrthoProjection(-2.777f, 2.777f, -2.0f, 2.0f, -2.0f, 2.0f);
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
		model.setPosition(xPos, yPos, 0.0);
	}

	void rotate(float value){//in degrees
		rotation += value;
		model.Rotate(value * 3.14159 / 180);
	}

	void setRotation(float value){// given in degrees
		model.Rotate(-1 * rotation * 3.14159 / 180);
		rotation = value;
		model.Rotate(value * 3.14159 / 180);
	}
	
	void checkLocation(){

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
	}

	bool sprite;
	bool onScreen;

	float x = 0;
	float y = 0;
	
	float rotation = 0;//in degrees

	//world coordinates
	float WorldX = 0;
	float WorldY = 0;
	vector<float> UR;
	vector<float> UL;
	vector<float> LL;
	vector<float> LR;
	
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

	float lifetime = 0;

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


void fireParticle(Entity& player, vector<Entity>* particles, int direction){
	if (particles->size() > 5){ return; }
	GLuint smoke = LoadTexture("whitePuff06.png");
	Entity temp = Entity(player.program, smoke);
	temp.size = 0.4;
	temp.width = 400 * temp.size / 1024.0f; //400
	temp.height = 383 * temp.size / 1024.0f; //383
	temp.setPosition(player.x, player.y);
	
	switch (direction){
	case 1:
		temp.directionX = 0;
		temp.directionY = -1;
		temp.speed = 0.25;
		break;
	default:
		break;
	}

	particles->push_back(temp);
}

void movePlayer(Entity& player, vector<Entity>* airParticles, float &elapsed){
	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	int direction = 0;
	
	if (keys[SDL_SCANCODE_RIGHT]){
		direction += 3;
	}
	else if (keys[SDL_SCANCODE_LEFT]){
		direction += 9;
	}
	if (keys[SDL_SCANCODE_UP]){
		direction += 1;
	}
	else if (keys[SDL_SCANCODE_DOWN]){
		direction += 5;
	}

	if (direction > 0){
		player.setRotation(0);
	}
	switch (direction){
	case 1:
		player.move(0.0, 0.15*elapsed);
		player.setRotation(0);
		break;
	case 3:
		player.move(0.15 * elapsed, 0.0);
		player.setRotation(-90);
		break;
	case 4:
		player.move(0.15 * elapsed, 0.15 * elapsed);
		player.setRotation(-45);
		break;
	case 5:
		player.move(0.0, -0.15 * elapsed);
		player.setRotation(180);
		break;
	case 8:
		player.move(0.15 * elapsed, -0.15 * elapsed);
		player.setRotation(-135);
		break;
	case 9:
		player.move(-0.15 * elapsed, 0.0);
		player.setRotation(90);
		break;
	case 10:
		player.move(-0.15 * elapsed, 0.15 * elapsed);
		player.setRotation(45);
		break;
	case 14:
		player.move(-0.15 * elapsed, -0.15 * elapsed);
		player.setRotation(135);
		break;
	default:
		break;
	}

	
}

void movePlayer2(Entity& player, float &elapsed){
	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	int direction = 0;

	if (keys[SDL_SCANCODE_D]){
		direction += 3;
	}
	else if (keys[SDL_SCANCODE_A]){
		direction += 9;
	}
	if (keys[SDL_SCANCODE_W]){
		direction += 1;
	}
	else if (keys[SDL_SCANCODE_S]){
		direction += 5;
	}

	if (direction > 0){
		player.setRotation(0);
	}

	switch (direction){
	case 1:
		player.move(0.0, 0.15 * elapsed);
		player.setRotation(0);
		break;
	case 3:
		player.move(0.15 * elapsed, 0.0);
		player.setRotation(-90);
		break;
	case 4:
		player.move(0.15 * elapsed, 0.15 * elapsed);
		player.setRotation(-45);
		break;
	case 5:
		player.move(0.0, -0.15 * elapsed);
		player.setRotation(180);
		break;
	case 8:
		player.move(0.15 * elapsed, -0.15 * elapsed);
		player.setRotation(-135);
		break;
	case 9:
		player.move(-0.15 * elapsed, 0.0);
		player.setRotation(90);
		break;
	case 10:
		player.move(-0.15 * elapsed, 0.15 * elapsed);
		player.setRotation(45);
		break;
	case 14:
		player.move(-0.15 * elapsed, -0.15 * elapsed);
		player.setRotation(135);
		break;
	default:
		break;
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
	
	float tempRotation = atan2((rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f);
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
	astroids[astroids.size() - 1].rotate(tempRotation);
	int position = 0;
	if (astroids[astroids.size() - 1].x > 1.77){
		position += 3;
	}
	else if (astroids[astroids.size() - 1].x < -1.77){
		position += 9;
	}
	if (astroids[astroids.size() - 1].y > 1){
		position += 1;
	}
	else if (astroids[astroids.size() - 1].y < -1){
		position += 5;
	}
	switch (position){
	case 1:
		astroids[astroids.size() - 1].directionX = rand() % 2;
		astroids[astroids.size() - 1].directionY = -1;
		break;
	case 3:
		astroids[astroids.size() - 1].directionY = rand() % 2;
		astroids[astroids.size() - 1].directionX = -1;
		break;
	case 4:
		astroids[astroids.size() - 1].directionX = -1;
		astroids[astroids.size() - 1].directionY = -1;
		break;
	case 5:
		astroids[astroids.size() - 1].directionX = rand() % 2;
		astroids[astroids.size() - 1].directionY = 1;
		break;
	case 8:
		astroids[astroids.size() - 1].directionX = -1;
		astroids[astroids.size() - 1].directionY = 1;
		break;
	case 9:
		astroids[astroids.size() - 1].directionY = rand() % 2;
		astroids[astroids.size() - 1].directionX = 1;
		break;
	case 10:
		astroids[astroids.size() - 1].directionX = 1;
		astroids[astroids.size() - 1].directionY = -1;
		break;
	case 14:
		astroids[astroids.size() - 1].directionX = 1;
		astroids[astroids.size() - 1].directionY = 1;
	default:
		break;
	}
}


void moveAstroid(Entity& astroid, float elapsed){
	float rotation = astroid.rotation;
	astroid.rotate(-rotation);
	astroid.move(0.20 * elapsed * astroid.directionX, 0.20 * elapsed * astroid.directionY);
	astroid.rotate(rotation + 0.01);
}

void enemyFires(vector<Entity>* enemies, vector<Entity> *enemyBullets){
	int randomNum;
	if (enemies->size() > 1){
		randomNum = rand() % (enemies->size() - 1);
	}
	else{
		randomNum = 0;
	}
	fireParticle((*enemies)[randomNum], enemyBullets, false);
}


bool detectCollision(Entity& first, Entity& second){
	float firstRotation = first.rotation;
	float secondRotation = second.rotation;
	first.rotate(-1 *(firstRotation * 3.14159 / 180));
	second.rotate(-1 * (secondRotation * 3.14159 / 180));

	if ((first.x + first.width / 2) >= (second.x - second.width / 2) &&
		(first.x - first.width / 2) <= (second.x + second.width / 2) &&
		(first.y - first.height / 2) <= (second.y + second.height / 2) &&
		(first.y + first.height / 2) >= (second.y - second.height / 2)){
		first.rotate(firstRotation * 3.14159 / 180);
		second.rotate(secondRotation * 3.14159 / 180);
		return true;
	}
	first.rotate(firstRotation * 3.14159 / 180);
	second.rotate(secondRotation * 3.14159 / 180);
	return false;
};

int collidedWith(vector<Entity> &objects, Entity& item){
	if (objects.size() == 0){ return -1; }
	for (int i = 0; i < objects.size() - 1; ++i){
		if (detectCollision(objects[i], item)){
			return i;
		}
	}
	return -1;
}

vector<float> randVals(){
	int xVal = rand() % 170;
	int yVal = rand() % 9;
	int sign = rand() % 2;
	int sign2 = rand() % 2;
	if (sign == 0){ sign = -1; }
	else{ sign = 1; }
	if (sign2 == 0){ sign2 = -1; }
	else{ sign2 = 1; }

	vector<float> vals;
	vals.push_back(xVal / 100.0 * sign);
	vals.push_back(yVal / 10.0 * sign2);
	return vals;
}

void spawnSpaceship(vector<Entity> &astroids, Entity& spaceShip){
	spaceShip.setMatrix();
	do{
		vector<float> vals = randVals();
		spaceShip.setPosition(vals[0], vals[1]);
	} while (collidedWith(astroids, spaceShip) > -1);
	spaceShip.onScreen = true;
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
	Entity text2 = Entity(&program, fontsheet);
	Entity text3 = Entity(&program, fontsheet);
	Entity text4 = Entity(&program, fontsheet);
	Entity text5 = Entity(&program, fontsheet);
	text.setPosition(-1.5, 0.2);
	text2.setPosition(-0.4, 0.1);
	text3.setPosition(-1.2, 0);
	text4.setPosition(-0.7, -0.1);
	text5.setPosition(-0.2, -0.2);
	
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
				state = 5;
			}
			else if (event.type == SDL_KEYDOWN){
				if (event.key.keysym.scancode == SDL_SCANCODE_N){
					done = true;
					state = 1;					
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_Y){
					done = true;
					state = 2;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
					done = true;
					state = 5;
				}
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);

		text.setMatrix();
		
		DrawText(&program, fontsheet, "Welcome to Godspeed Trucking Frontier Organization", 0.06, 0.00);
		text2.setMatrix();
		DrawText(&program, fontsheet, "Or GTFO for short", 0.06, 0.0);
		text3.setMatrix();
		DrawText(&program, fontsheet, "please return to spaceship and continue voyage", 0.06, 0.0);
		text4.setMatrix();
		DrawText(&program, fontsheet, "Do you have any enemies?", 0.06, 0.0);
		text5.setMatrix();
		DrawText(&program, fontsheet, "(Y/N)", 0.06, 0.0);
		

		SDL_GL_SwapWindow(displayWindow);
	}
}

void runGame(int& state, SDL_Event& event){
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	GLuint spriteSheet = LoadTexture("sheet.png");
	GLuint alien = LoadTexture("alienBlue.png");
	GLuint alien2 = LoadTexture("alienBeige.png");

	Mix_Chunk *teleportSound;
	teleportSound = Mix_LoadWAV("sfxSpaceShipTeleport.wav");

	Mix_Chunk *shootSound;
	shootSound = Mix_LoadWAV("sfxShoot.wav");
	//<SubTexture name="alienBlue.png" x="412" y="175" width="66" height="92"/>
	Entity player = Entity(&program, alien);
	player.width = 66 / 1024.0f;
	player.height = 92 / 1024.0f;
	player.size = 1.5;
	vector<float> playerPosition = randVals();
	player.setPosition(playerPosition[0], playerPosition[1]);
	
	bool secondPlayer = false;
	if (state == 2){
		secondPlayer = true;
	}
	Entity player2 = Entity(&program, alien2);
	player2.width = 66 / 1024.0f;
	player2.height = 92 / 1024.0f;
	player2.size = 1.5;
	playerPosition = randVals();
	player2.setPosition(playerPosition[0], playerPosition[1]);

	vector<Entity> airParticles;

	vector<Entity> astroids;
	float lastAstroidSpawnTime = 0;
	for (int i = 0; i < 20; ++i){
		spawnAstroid(astroids, program, spriteSheet);
	}
	
	Time counter;
	counter.lastFrameTicks = SDL_GetTicks() / 1000.0f;
	float elapsed;

	Entity spaceShip = Entity(&program, spriteSheet, 444 / 1024.0, 91 / 1024.0, 91 / 1024.0, 91 / 1024.0, 2.5);
	spawnSpaceship(astroids, spaceShip);

	//SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
				state = 5;
			}
			else if (event.type == SDL_KEYDOWN){
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
					done = true;
					state = 0;
				}
			}
		}
		glClear(GL_COLOR_BUFFER_BIT);

		

		if (((float)SDL_GetTicks() / 1000.0f - lastAstroidSpawnTime) > 0.5){
			spawnAstroid(astroids, program, spriteSheet);
			lastAstroidSpawnTime = (float)SDL_GetTicks() / 1000.0f;
			if (!spaceShip.onScreen){
				spawnSpaceship(astroids, spaceShip);
				Mix_PlayChannel(-1, teleportSound, 0);
			}
		}
		
		elapsed = counter.getTime();

		for (int i = 0; i < astroids.size(); ++i){
			astroids[i].setMatrix();
			astroids[i].Draw();
			moveAstroid(astroids[i], elapsed);
			if (detectCollision(astroids[i], player)){
				player.shouldRemove = true;
			}
			if (secondPlayer){
				if (detectCollision(astroids[i], player2)){
					player2.shouldRemove = true;
				}
			}
		}

		player.setMatrix();
		player.Draw();
		if (secondPlayer){
			player2.setMatrix();
			player2.Draw();
			movePlayer2(player2, elapsed);
		}

		movePlayer(player, &airParticles, elapsed);

		if (spaceShip.onScreen){
			spaceShip.setMatrix();
			spaceShip.Draw();
		}

		if (collidedWith(astroids, spaceShip) > -1){
			spaceShip.onScreen = false;
		}

		for (int i = 0; i < astroids.size(); ++i){
			if (astroids[i].shouldRemove){
				astroids.erase(astroids.begin() + i);
				--i;
				
			}
		}

		if (secondPlayer){
			if (player2.shouldRemove){
				state = 3;
				done = true;
			}
			if (detectCollision(spaceShip, player2)){
				state = 4;
				done = true;
			}
		}
		
		if (player.shouldRemove){
			state = 3;
			done = true;
		}
		if (detectCollision(spaceShip, player)){
			state = 4;
			done = true;
		}

		SDL_GL_SwapWindow(displayWindow);// keep at bottom
	}

	Mix_FreeChunk(teleportSound);
	//Mix_FreeChunk(shootSound);
	
}

void gameOver(int& state, SDL_Event& event){
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	GLuint fontsheet = LoadTexture("font1.png");

	Entity text = Entity(&program, fontsheet);
	Entity text1 = Entity(&program, fontsheet);
	Entity text2 = Entity(&program, fontsheet);
	text.setPosition(-1.3, 0);
	text1.setPosition(-1.3, -0.2);
	text2.setPosition(-1.3, -0.4);

	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
				state = 5;
			}
			else if (event.type == SDL_KEYDOWN){
				if (event.key.keysym.scancode == SDL_SCANCODE_RETURN){
					done = true;
					state = 0;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
					done = true;
					state = 5;
				}
			}
		}
		text.setMatrix();
		if (state == 3){
			DrawText(&program, fontsheet, "It appears someone died...", 0.08, 0.0);
		}
		if (state == 4){
			DrawText(&program, fontsheet, "The spaceship is now taking off", 0.08, 0.0);
		}

		text1.setMatrix();
		if (state == 3){
			DrawText(&program, fontsheet, "Press Enter for reincarnation", 0.08, 0.0);
		}
		if (state == 4){
			DrawText(&program, fontsheet, "Press Enter to request a new spaceship", 0.08, 0.0);
		}

		text2.setMatrix();
		DrawText(&program, fontsheet, "Press Esc to GTFO", 0.08, 0.0);


		SDL_GL_SwapWindow(displayWindow);
	}
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif

	glViewport(0, 0, 1280, 720);

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

	enum gameState { MAIN, GAME, GAME2, OVER, WIN, CLOSE};
	int state = MAIN;

	Mix_Music *music;
	music = Mix_LoadMUS("Blue.mp3");

	Mix_Chunk *winSound;
	winSound = Mix_LoadWAV("SpaceShip.wav");

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	bool done = false;
	SDL_Event event;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		//Mix_PlayMusic(music, -1);

		switch (state){
		case MAIN:
			glClear(GL_COLOR_BUFFER_BIT);
			titleScreen(state, event);
			break;
		case GAME:
			Mix_PlayMusic(music, -1);
			glClear(GL_COLOR_BUFFER_BIT);
			runGame(state, event);
			break;
		case GAME2:
			Mix_PlayMusic(music, -1);
			glClear(GL_COLOR_BUFFER_BIT);
			runGame(state, event);
			break;
		case OVER:
			//Mix_PlayChannel(-1, dieSound, 0);
			gameOver(state, event);
			break;
		case WIN:
			//Mix_PlayChannel(-1, winSound, 0);
			gameOver(state, event);
			break;
		case CLOSE:
			done = true;
			break;
		}
	}

	Mix_FreeChunk(winSound);
	Mix_FreeMusic(music);

	SDL_Quit();
	return 0;
}
