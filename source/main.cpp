#include <nds.h>
#include <gl2d.h>
#include <stdio.h>
#include <vector>
#include <math.h>

enum state {
	START,
	PLAY,
	END
};

void drawBackground();
void drawHeart(int x);
class Projectile {
public:
	Projectile(float x, float y, float xVel, float yVel) {
		this->x = x;
		this->y = y;
		this->xVel = xVel;
		this->yVel = yVel;
	}

	void update() {
		x += xVel;
		y += yVel;
	}

	void draw() {
		glBoxFilled((int) x, (int) y, (int) x + 5, (int) y + 5, RGB15(31, 0, 0));
	}

	int getY() {
		return y;
	}

	int getX() {
		return x;
	}

private:
	float x;
	float y;
	float xVel;
	float yVel;
};

class Enemy {
public:
	Enemy(int x, int y) {
		this->x = x;
		this->y = y;
	}

	void update() {
        if (pathIndex == 0) {
			x += 1;
			if (x >= 210)
				pathIndex++;
		} else if (pathIndex == 1) {
			y -= 1;
			if (y <= 90)
				pathIndex++;
		} else if (pathIndex == 2) {
			x -= 1;
			if (x <= 50)
				pathIndex++;
		} else if (pathIndex == 3){
			y -= 1;
			if (y <= 50)
				pathIndex++;
		} else {
			x += 1;
		}
    }

	int getX() {
		return x;
	}

	int getY() {
		return y;
	}

	void draw() {
		glBoxFilled(x - 5, y - 5, x + 5, y + 5, RGB15(0, 0, 31));
	}

private:
	int x;
	int y;
	int pathIndex = 0;
	int direction = 1;
};


int main(void) {
	touchPosition touchXY;

	// a vector to hold the position of the box
	int boxPos[2] = { 0, 0 };

	std::vector<Projectile> projectiles;
	std::vector<Enemy> enemies;

	int score = 0;
	int lives = 3;
	int frames = 0;
	state gameState = START;

	enemies.push_back(Enemy(0, 130));

	// initialize the console so we can print to it
	consoleDemoInit();

	//set mode 5, enable BG0 and set it to 3D
	videoSetMode( MODE_5_3D );
	
	// initialize gl2d
	glScreen2D();

    while(1) {
		int keys = keysDown();

		switch(gameState) {
			case(START): {
				glBegin2D();
				drawBackground();
				glEnd2D();
				swiWaitForVBlank();
				scanKeys();
				if (keys & KEY_START) {
					gameState = PLAY;
				}

				iprintf("\x1b[10;6HPress Start to Play");

				break;
			}
			case(PLAY): {
				scanKeys();
				if (keys & KEY_START) break;

				// get the touch position and gradually move the box towards it
				touchRead(&touchXY);
				if (touchXY.px != 0 && touchXY.py != 0) {
					boxPos[0] += (touchXY.px - boxPos[0]) / 2;
					boxPos[1] += (touchXY.py - boxPos[1]) / 2;
				}


				// Start 2D mode
				glBegin2D();

				drawBackground();

				// Draw cannon 
				// x and y distance from cannon to box as a vector
				int dist[2] = { 245 - boxPos[0], 192 / 2 - boxPos[1] };

				//normalize the vector
				float magnitude = sqrt(dist[0] * dist[0] + dist[1] * dist[1]);

				glLine(245, 192 / 2 - 3, 245 - (dist[0] / 8), 192 / 2 - 3 - (dist[1] / 8), RGB15(31, 0, 0));
				glLine(245, 192 / 2 - 2, 245 - (dist[0] / 8), 192 / 2 - 2 - (dist[1] / 8), RGB15(31, 0, 0));
				glLine(245, 192 / 2 - 1, 245 - (dist[0] / 8), 192 / 2 - 1 - (dist[1] / 8), RGB15(31, 0, 0));
				glLine(245, 192 / 2 , 245 - (dist[0] / 8), 192 / 2 - (dist[1] / 8), RGB15(31, 0, 0));
				glLine(245, 192 / 2 + 1, 245 - (dist[0] / 8), 192 / 2 + 1 - (dist[1] / 8), RGB15(31, 0, 0));
				glLine(245, 192 / 2 + 2, 245 - (dist[0] / 8), 192 / 2 + 2 - (dist[1] / 8), RGB15(31, 0, 0));
				glLine(245, 192 / 2 + 3, 245 - (dist[0] / 8), 192 / 2 + 3 - (dist[1] / 8), RGB15(31, 0, 0));

				glBoxFilled(240, 91, 250, 101, RGB15(15, 0, 0));


				// shoot projectiles from cannon
				if (keys & KEY_UP) {
					projectiles.push_back(Projectile(245, 192 / 2, (float) -dist[0] / magnitude * 2, (float) -dist[1] / magnitude * 2));
				}

				// update projectiles
				for (int i = 0; i < projectiles.size(); i++) {
					projectiles[i].update();
					projectiles[i].draw();
					// if a projectile goes off the screen, remove it
					if (projectiles[i].getX() > 255 || projectiles[i].getY() > 191 || projectiles[i].getY() < 0) {
						projectiles.erase(projectiles.begin() + i);
					}

				}

				// update enemies
				for (int i = 0; i < enemies.size(); i++) {
					enemies[i].update();
					enemies[i].draw();

					// if an enemy goes off the screen, remove it
					if (enemies[i].getX() > 260) {
						enemies.erase(enemies.begin() + i);
						lives--;
						glBoxFilled(0, 0, 255, 191, RGB15(31, 0, 0));
						if (lives <= 0) {
							gameState = END;
						}
					}

					// if an enemy collides with a projectile, remove both
					for (int j = 0; j < projectiles.size(); j++) {
						if (projectiles[j].getX() > enemies[i].getX() - 8 && projectiles[j].getX() < enemies[i].getX() + 8 && projectiles[j].getY() > enemies[i].getY() - 8 && projectiles[j].getY() < enemies[i].getY() + 8) {
							enemies.erase(enemies.begin() + i);
							projectiles.erase(projectiles.begin() + j);
							score++;
						}
					}
				}

				// spawn enemies every couple frames
				if (score < 5) {
					if (frames % 200 == 0)
						enemies.push_back(Enemy(0, 130));
				} else if (score < 10) {
					if (frames % 150 == 0)
						enemies.push_back(Enemy(0, 130));
				} else {
					if (frames % 100 == 0)
						enemies.push_back(Enemy(0, 130));
				}

				// print score
				consoleClear();
				iprintf("\x1b[10;13HScore: %d", score);

				// draw hearts in the top right corner
				for (int i = 0; i < lives; i++) {
					drawHeart(240 - i * 30);
				}

				// Stop 2D mode
				glEnd2D();
				glFlush(0);
				swiWaitForVBlank();
				frames++;
				break;
			}

			case(END): {
				glBegin2D();
				drawBackground();
				glEnd2D();
				swiWaitForVBlank();
				scanKeys();
				if (keys & KEY_START) {
					gameState = PLAY;
					score = 0;
					lives = 3;
				}

				iprintf("\x1b[10;4HPress Start to Play Again");

				break;
			}
		}
    }

    return 0;
}

void drawBackground() {
	// draw grass
	glBoxFilled(0, 0, 255, 191, RGB15(0, 15, 3));

	// draw path
	glBoxFilled(0, 120, 220, 140, RGB15(15, 9, 3));
	glBoxFilled(200, 120, 220, 80, RGB15(15, 9, 3));
	glBoxFilled(200, 100, 40, 80, RGB15(15, 9, 3));
	glBoxFilled(40, 100, 60, 40, RGB15(15, 9, 3));
	glBoxFilled(60, 60, 255, 40, RGB15(15, 9, 3));
}

void drawHeart(int x) {
	glBoxFilled(x - 8, 12, x + 8, 16, RGB15(31, 0, 0));
	glBoxFilled(x - 6, 16, x + 6, 20, RGB15(31, 0, 0));
	glBoxFilled(x - 6, 8, x - 2, 12, RGB15(31, 0, 0));
	glBoxFilled(x + 2, 8, x + 6, 12, RGB15(31, 0, 0));
	glBoxFilled(x - 2, 20, x + 2, 24, RGB15(31, 0, 0));
}