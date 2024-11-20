#include "raylib.h"
#include "resource_dir.h"
// for trig functions, etc.
#include <math.h>
// for absolute value
#include <stdlib.h>

/*
 * Browser support
 * See https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)
 */
#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

// separate functions for update and draw
static void UpdateFrame(void);
static void DrawingFrame(void);

static const int screenWidth = 800;
static const int screenHeight = 450;

static int p1Score;
static int p2Score;

static Image puckImg;
static Texture2D p;

static Image rpiLogo;
static Texture2D logo;

static Image p1Img;
static Texture2D p1;

static Image p2Img;
static Texture2D p2;

static Sound goalSound;

static bool gamePaused;

static Rectangle p1GoalBox;
static Rectangle p2GoalBox;

/* size of circle trail array
static const int num_circles = 50;
*/

// TODO add variables as they come

// structs for puck or player pos/vel/accel values
typedef struct IceObj {
	Vector2 position;
	Vector2 speed;
	Vector2 accel;
} IceObj;

/* circle trail to follow puck left commented
typedef struct CircleTrail {
	Vector2 position;
} CircleTrail;
*/

static struct IceObj puck;
static struct IceObj play1;
static struct IceObj play2;
// static struct CircleTrail cT[num_circles];

int main() {

	// support for vsync and high DPI displays (unnecessary)
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	SearchAndSetResourceDir("resources");


	/* Load images, sounds and other resources here */

	rpiLogo = LoadImage("rpi-logo.png"); 
	ImageResize(&rpiLogo, (int)(rpiLogo.width / 16), (int)(rpiLogo.height / 16));
	logo = LoadTextureFromImage(rpiLogo); 

	puckImg = LoadImage("puck.png");
	ImageResize(&puckImg, (int)(puckImg.width / 16), (int)(puckImg.height / 16));
	p = LoadTextureFromImage(puckImg);

	p1Img = LoadImage("puckman.png");
	ImageResize(&p1Img, (int)(p1Img.width / 1.75), (int)(p1Img.height / 1.75));
	p1 = LoadTextureFromImage(p1Img);

	p2Img = LoadImage("greenpuckman.png");
	ImageResize(&p2Img, (int)(p2Img.width / 1.75), (int)(p2Img.height / 1.75));
	p2 = LoadTextureFromImage(p2Img);

	goalSound = LoadSound("rpigoalsfx.mp3");

	// goal box rectangles
	p2GoalBox = (Rectangle){screenWidth-15, screenHeight / 3 + screenHeight/27, 4, screenHeight/3 - screenHeight/15};
	p1GoalBox = (Rectangle){20,screenHeight / 3 + screenHeight/27, 4, screenHeight/3 - screenHeight/14};

	// Initial puck and player-related values
	puck.position = (Vector2){ 400, 225 };
	puck.speed = (Vector2){ 1, 1 };
	// we will multiply speed by accel value so it acts as friction
	puck.accel = (Vector2){ -0.02, -0.02 };

	play1.position = (Vector2){ 100, 225 };
	play1.speed = (Vector2){ 0.05, 0.05 };
	play1.accel = (Vector2){ -0.02, -0.02 };

	play2.position = (Vector2){ 700, 225 };
    play2.speed = (Vector2){ 0.05, 0.05 };
    play2.accel = (Vector2){ -0.02, -0.02 };

	// set score to 0
	p1Score = 0;
	p2Score = 0;

	gamePaused = false;

	#if defined(PLATFORM_WEB)
    	emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
	#else
    	SetTargetFPS(30);
	#endif

	// Create the window and OpenGL context
	InitWindow(screenWidth, screenHeight, "Puckman Ball");
	
	// game loop runs until ESC key is pressed
	while (!WindowShouldClose()) { UpdateFrame(); DrawingFrame(); }

	/* UNLOAD all sounds and images here */
	UnloadSound(goalSound);
	UnloadTexture(logo);
	UnloadImage(rpiLogo);

	UnloadTexture(p);
	UnloadImage(puckImg);

	UnloadTexture(p1);
	UnloadImage(p1Img);

	UnloadTexture(p2);
	UnloadImage(p2Img);

	// close the window and exit cleanly
	CloseWindow();
	return 0;
}

// Changes variables (puck position, score, etc.) prior to drawing
static void UpdateFrame(void) {
	
	// handle pausing
	if ( IsKeyPressed('P') ) { gamePaused = !gamePaused; }

	if ( !gamePaused ) {
		// Handle user input (WASD for play1, arrows for play2)
		if ( IsKeyDown(KEY_W) ) { play1.accel.y = -0.25; }
		if ( IsKeyDown(KEY_S) ) { play1.accel.y = 0.25; }
		if ( IsKeyDown(KEY_A) ) { play1.accel.x = -0.25; }
		if ( IsKeyDown(KEY_D) ) { play1.accel.x = 0.25; }
		if ( IsKeyDown(KEY_UP) ) { play2.accel.y = -0.25; }
		if ( IsKeyDown(KEY_DOWN) ) { play2.accel.y = 0.25; }
		if ( IsKeyDown(KEY_LEFT) ) { play2.accel.x = -0.25; }
		if ( IsKeyDown(KEY_RIGHT) ) { play2.accel.x = 0.25; }
		// reset accel if neither direction is pressed
		if ( !IsKeyDown(KEY_LEFT) && !IsKeyDown(KEY_RIGHT) ) {
			play2.accel.x = -0.02;
		}
		if ( !IsKeyDown(KEY_UP) && !IsKeyDown(KEY_DOWN) ) {
            play2.accel.y = -0.02;
        }
		if ( !IsKeyDown(KEY_A) && !IsKeyDown(KEY_D) ) {
            play1.accel.x = -0.02;
        }
		if ( !IsKeyDown(KEY_W) && !IsKeyDown(KEY_S) ) {
            play1.accel.x = -0.02;
        }

		// update speed and position for puck and players
		play1.speed.x += play1.accel.x;
		play1.speed.y += play1.accel.y;
		play1.position.x += play1.speed.x;
		play1.position.y += play1.speed.y;

		play2.speed.x += play2.accel.x;
        play2.speed.y += play2.accel.y;
        play2.position.x += play2.speed.x;
        play2.position.y += play2.speed.y;

		puck.speed.x += puck.accel.x;
        puck.speed.y += puck.accel.y;
        puck.position.x += puck.speed.x;
        puck.position.y += puck.speed.y;

		// basic puck wall collision
		/* TODO rounded corner collision
		 * use float Vector2DotProduct(Vector2 v1, Vector2 v2);
		 */
		if (puck.position.x <= 20) {
			puck.speed.x *= -0.75;
			puck.accel.x *= -0.25;
			puck.position.x = 22;
		}
		if (puck.position.x >= 780) {
			puck.speed.x *= -0.75;
            puck.accel.x *= -0.25;
			puck.position.x = 778;
		}
		if (puck.position.y <= 20) {
			puck.speed.y *= -0.75;
            puck.accel.y *= -0.25;
			puck.position.y = 22;
		}
		if (puck.position.y >= 430) {
			puck.speed.y *= -0.75;
            puck.accel.y *= -0.25;
			puck.position.y = 428;
		}

		// identical bounds logic for both players
		if (play1.position.x <= 20) {
            play1.speed.x *= -0.75;
            play1.accel.x *= -0.25;
            play1.position.x = 22;
        }
        if (play1.position.x >= 780) {
            play1.speed.x *= -0.75;
            play1.accel.x *= -0.25;
            play1.position.x = 778;
        }
    	if (play1.position.y <= 20) {
            play1.speed.y *= -0.75;
            play1.accel.y *= -0.25;
            play1.position.y = 22;
        }
        if (play1.position.y >= 430) {
            play1.speed.y *= -0.75;
			play1.accel.y *= -0.25;
            play1.position.y = 428;
        }

		// more for player 2
		if (play2.position.x <= 20) {
            play2.speed.x *= -0.75;
            play2.accel.x *= -0.25;
            play2.position.x = 22;
        }
        if (play2.position.x >= 780) {
            play2.speed.x *= -0.75;
            play2.accel.x *= -0.25;
            play2.position.x = 778;
        }
        if (play2.position.y <= 20) {
            play2.speed.y *= -0.75;
            play2.accel.y *= -0.25;
            play2.position.y = 22;
        }
        if (play2.position.y >= 430) {
            play2.speed.y *= -0.75;
            play2.accel.y *= -0.25;
            play2.position.y = 428;
        }

		// HITBOXES
		Rectangle puckBox = (Rectangle){puck.position.x, puck.position.y, p.width, p.height};
		// Note: It would be better if these were circles for hitting and scoring purposes. Puckman Feet >:( .

		Rectangle p1Box = (Rectangle){play1.position.x, play1.position.y, p1.width, p1.height};
		Rectangle p2Box = (Rectangle){play2.position.x, play2.position.y, p2.width, p2.height};

		// Check for collision
		if (CheckCollisionRecs(p1Box, puckBox)) {
			float deltaX = puck.position.x - play1.position.x;
			float deltaY = puck.position.x - play1.position.x;
			float angle = atan2(deltaY, deltaX);

			// Adjust puck velocity based on the collision angle and player's velocity
			puck.speed.x = (float)(cos(angle) * abs(puck.speed.x) + play1.speed.x);
			puck.speed.y = (float)(sin(angle) * abs(puck.speed.y) + play1.speed.y);
		}

		if (CheckCollisionRecs(p2Box, puckBox)) {
			float deltaX = puck.position.x - play2.position.x;
			float deltaY = puck.position.x - play2.position.x;
			float angle = atan2(deltaY, deltaX);

			// Adjust puck velocity based on the collision angle and player's velocity
			puck.speed.x = (float)(cos(angle) * abs(puck.speed.x) + play2.speed.x);
			puck.speed.y = (float)(sin(angle) * abs(puck.speed.y) + play2.speed.y);
		}

		if (CheckCollisionRecs(p2GoalBox, puckBox)) {
            p1Score++;
            PlaySound(goalSound);
            // Position reset
            puck.position = (Vector2){ 400, 225 };
            puck.speed = (Vector2){ 1, 1 };
            puck.accel = (Vector2){ -0.02, -0.02 };

            play1.position = (Vector2){ 100, 225 };
            play1.speed = (Vector2){ 0.05, 0.05 };
            play1.accel = (Vector2){ -0.02, -0.02 };

            play2.position = (Vector2){ 700, 225 };
            play2.speed = (Vector2){ 0.05, 0.05 };
            play2.accel = (Vector2){ -0.02, -0.02 };
        }

		if (CheckCollisionRecs(p1GoalBox, puckBox)) {
            p2Score++;
            PlaySound(goalSound);
            // Position reset
            puck.position = (Vector2){ 400, 225 };
            puck.speed = (Vector2){ 1, 1 };
            puck.accel = (Vector2){ -0.02, -0.02 };

            play1.position = (Vector2){ 100, 225 };
            play1.speed = (Vector2){ 0.05, 0.05 };
            play1.accel = (Vector2){ -0.02, -0.02 };

            play2.position = (Vector2){ 700, 225 };
            play2.speed = (Vector2){ 0.05, 0.05 };
            play2.accel = (Vector2){ -0.02, -0.02 };
        }

		/* for circle trail, use var to count frames, then
		 * cT[framesCounter % NUM_CIRCLES].position = puck.position;
		 */

		// TODO: puck/player circles and reimplemented collisions
		// use above DotProduct function along with Angle
	} // gamePaused if statement

}

// Draw stadium, score, players etc. to window
static void DrawingFrame(void) {
	BeginDrawing();

	ClearBackground(LIGHTGRAY);

	// Draw the stadium to screen

	DrawRectangleRounded( (Rectangle){5, 5, screenWidth - 10, screenHeight - 10}, 0.3f, 1, DARKGRAY);
	DrawRectangleRounded( (Rectangle){10, 10, screenWidth - 20, screenHeight - 20}, 0.3f, 1, WHITE);

	DrawRectangle(screenWidth/2 - 2, 10, 4, screenHeight - 20, RED);
	DrawRectangle((int)(screenWidth * 0.15) - 2, 10, 4, screenHeight- 20, RED);
	DrawRectangle((int)(screenWidth * 0.85) - 2, 10, 4, screenHeight- 20, RED);

	DrawText(TextFormat("%i", p1Score), screenWidth * 1 / 4 - 10, screenHeight/10, 70, BLACK);
	DrawText(TextFormat("%i", p2Score), screenWidth * 3 / 4 - 10, screenHeight/10, 70, BLACK);

	DrawCircle(screenWidth/2, screenHeight/2, 40, RED);
	DrawCircle(screenWidth/2, screenHeight/2, 37, WHITE);
	DrawTexture(logo, screenWidth/2 - logo.width/2, screenHeight/2 - logo.height/2, WHITE);
	
	// Right/Player 2 Goal Box
	DrawRectangle(screenWidth - screenWidth/16, screenHeight / 3, screenWidth/16 - 10, screenHeight/3, ColorFromHSV(180, 1, 1));
	DrawRectangle(screenWidth - screenWidth/16 + 3, screenHeight / 3 + 3, screenWidth/16 - 13, screenHeight/3 - 6, WHITE);

	DrawRectangle(screenWidth-15,screenHeight / 3 + screenHeight/27, 4, screenHeight/3 - screenHeight/14,BLACK);

	// Left/Player 1 Goal Box
	DrawRectangle(10, screenHeight / 3, screenWidth/16 - 10, screenHeight/3, RED);
	DrawRectangle(10, screenHeight / 3 + 3, screenWidth/16 - 13, screenHeight/3 - 6, WHITE);

	DrawRectangle(15,screenHeight / 3 + screenHeight/27, 4, screenHeight/3 - screenHeight/14,BLACK);

	// Draw puck and player
	DrawTexture(p1, (int)(play1.position.x - p1.width / 2), (int)(play1.position.y - p1.height / 2), WHITE);
	DrawTexture(p2, (int)(play2.position.x - p1.width / 2), (int)(play2.position.y - p2.height / 2), WHITE);
	DrawTexture(p, (int)(puck.position.x - 25), (int)(puck.position.y - 25), WHITE);

	if (p1Score == 7) {
		DrawText("Player 1 Wins", screenWidth/4, screenHeight/2, 50, BLACK);
	}

	if (p2Score == 7) {
		DrawText("Player 2 Wins", screenWidth/4, screenHeight/2, 50, BLACK);
	}

	EndDrawing();
}
