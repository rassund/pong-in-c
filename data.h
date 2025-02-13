/*  data.h
	Includes all defines as well as struct declarations and global variables
	
	Written by Rasmus Sundbom, 2024-02-28
	
	For copyright and licensing, see file COPYING	*/

#define PADDLE_HEIGHT 8
#define PADDLE_WIDTH 2
#define PADDLE_SPEED 0.15f

#define BALL_STARTX 63
#define BALL_STARTY 15
#define BALL_SIZE 2
#define BALL_SPEED 0.25f

#define BASE_MODIFIER 0.004f

#define SCORE_WIN 8

#define EASY_SPEED 0.085f
#define HARD_SPEED 0.12f

#define NUM_HIGH_SCRS 50

typedef struct object
{
	uint8_t data[512];
	float x;
	float y;
	float v_x;	// Velocity in x direction
	float v_y;	// Velocity in y direction
	int width;
	int height;
	int score;
} object;

struct high_score
{
	char initials[3];
	int score;
};

// Global variables
object p1;	// Paddle 1
object p2;	// Paddle 2
object b;	// Ball

uint8_t screen[512];
uint8_t score[512];
int btns;

struct high_score h_scrs[NUM_HIGH_SCRS];
char h_entry[9];	// High-score entry
int score_pos;	// Used to track amount of high-score entries

int delay_count;	// Used in delay function

int start_dir;	// Variable for alternating starting direction of the ball
float t_mod;	// Modifier for increasing balls speed over time

int window;
int in_game;