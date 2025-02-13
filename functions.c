/*	functions.c
	Includes functions related to the game and menus
	
	Written by Rasmus Sundbom, 2024-02-28
	
	For copyright and licensing, see file COPYING	*/

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declarations for these labs */
#include <math.h>	/* Necessary for absolute value function */
#include "data.h"
#include "functions.h"

// Draw functions, used for showing things on the display

/*	draw_object
	Draws an object on the screen based on its width, height, x-coordinate and y-coordinate
*/
void draw_object (object * o)
{
	uint8_t * obj;
	obj = o->data;
	int page_off;	// Page Offset, determines which page the pixel is on
	
	int i, j;
	for (i = 0; i < 512; i++)	// Clear all pixels
		obj[i] = 255;
	
	for (j = 0; j < o->width; j++)
	{
		for (i = 0; i < o->height; i++)
		{
			page_off = (o->y + i) / 8;
			obj[page_off * 128 + (int) o->x + j] -= 1 << ((int) o->y + i - page_off * 8);
		}
	}
	draw_screen ();
}

/*	draw_screen
	Combines paddle 1, paddle 2, ball and score, then draws them on the screen
*/
void draw_screen ()
{
	int i;
	for (i = 0; i < 512; i++)
		screen[i] = p1.data[i] & p2.data[i] & b.data[i] & score[i];
}

/*	draw_score
	Draws the scores of player 1 and player 2 on the screen, using font found in mipslabdata.c
*/
void draw_score (int s1, int s2)
{
	int i;
	char c = s1 + '0';
	for (i = 0; i < 512; i++)
	{
		score[i] = 255;
	}
	for (i = 0; i < 8; i++)	// Draw score 1
	{
		score[50 + i] = ~font[c * 8 + i];
	}
	
	c = s2 + '0';	// Draw score 2
	for (i = 0; i < 8; i++)
	{
		score[76 + i] = ~font[c * 8 + i];
	}
}

// Initialization functions

/*	init_btns
	Initializes pushbuttons 1-4 to input
*/
void init_btns ()
{
	// Pushbutton 1
	TRISFSET = 0x2;		// Initialize bit 1 to 1 (input)
	// Pushbuttons 2-4
	TRISDSET = 0x0e0;	// Initialize bits 7 through 5 to 1 (input)
}

/*	start_init
	Initialization to be done only once at program start.
	Initializes the timer and the pushbuttons, as well as setting some initial values of global variables.
*/
void start_init ()
{
	score_pos = 0;
	delay_count = 0;
	start_dir = 1;
	t_mod = 1;
	window = 0;
	in_game = 0;
	
	// Initialize Timer 2 to update 100 times per second
	TMR2 = 0;	// Reset the timer
	T2CONSET = 0x8070;	// Set the prescaling to 1:256. Prescaling TCKPS is stored in bits 6-4 of T2CON. Bit 15 enables the timer
	PR2 = 3125;	// Timer is set to update every 10 ms, that is 100 times per second
	// Configure the interrupt priority and subpriority levels, and set the interrupt enable bit
	IPC(2) = (IPC(2) | 0x1f);	// Set bits 2 to 4 of IPC2 to 7 and bits 0 to 1 of IPC2 to 3, which sets the interrupt to highest priority and subpriority levels (T2IP is stored at bits 2 - 4 of IPC2, T2IS is stored at bits 0 - 1 of IPC2)
	IEC(0) = (IEC(0) | 0x100);	// Set bit 8 of IEC0 to 1, which enables the interrupt for Timer 2 (T2IE is stored at bit 8 of IEC0)
	asm volatile ("ei");	// Enable interrupts globally
	
	init_btns();
}

/*	game_init
	Initialization done each time either single-player or multiplayer mode is loaded.
	This includes initialization for the paddles, the ball and the score.
*/
void game_init ()
{	
	p1.x = 5;
	p1.y = 15;
	p1.width = PADDLE_WIDTH;
	p1.height = PADDLE_HEIGHT;
	p1.score = 0;
	draw_object (&p1);
	
	p2.x = 122;
	p2.y = 15;
	p2.width = PADDLE_WIDTH;
	p2.height = PADDLE_HEIGHT;
	p2.score = 0;
	draw_object (&p2);
	
	b.x = BALL_STARTX;
	b.y = BALL_STARTY;
	b.width = BALL_SIZE;
	b.height = BALL_SIZE;
	b.v_x = BALL_SPEED / 2;
	b.v_y = BALL_SPEED / 2;
	draw_object (&b);
	t_mod = 1;
	
	draw_score (p1.score, p2.score);
	draw_screen();
}

// Game functions, used for input, moving objects and collision

/*	getbtns
	Returns the current status of the pushbuttons
*/
int getbtns ()
{
	return (((PORTD & 0x0e0) >> 4) | ((PORTF & 0x2) >> 1));	// Return bits 7 through 5 of PORTD together with bit 1 of PORTF, which are connected to the push buttons
}

/*	move_paddle
	Read paddles velocity and update its coordinates
*/
void move_paddle (object * p)
{
	if ((p->y > 0 && p->v_y < 0) || (p->y < (31 - p->height) && p->v_y > 0))
	{
		p->y += p->v_y;
	}
	draw_object (p);
}

/*	in_object
	Checks if object 1 is inside of object 2.
	Returns 1 if true, 0 if false.
*/
int in_object (object * o1, object * o2)
{
	int i, j;
	int col = 0;
	for (i = 0; i < o1->width; i++)
	{
		for (j = 0; j < o1->height; j++)
		{
			if (o1->x + i >= o2->x && o1->x + i <= o2->x + o2->width && o1->y + j <= o2->y + o2->height && o1->y + j >= o2->y)
			{
				col = 1;
				break;
			}
		}
		
		if (col)
			break;
	}
	return col;
}

/*	move_ball
	Updates the balls position according to its velocity.
	The speed of the ball is increased over time using the variable t_mod.
	Checks for vertical collision, horizontal collision, and collision with either paddle.
*/
void move_ball (object * b, object * p1, object * p2)
{
	float t;	// Variable used for swapping the values of v_x and v_y
	
	b->x += b->v_x * t_mod;
	b->y += b->v_y * t_mod;
	
	// Vertical collision
	if (b->y <= 0 || b->y >= 31 - b->height)
	{
		if ((b->v_x > 0 && b->v_y > 0) || (b->v_x < 0 && b->v_y < 0))	// Both velocities have same sign
		{
			t = b->v_y;
			b->v_y = b->v_x * -1;
		}
		else
		{
			t = b->v_y * -1;
			b->v_y = b->v_x;
		}
		b->v_x = t;
		while (b->y <= 0 || b->y >= 31 - b->height)	// Do nothing until ball is back on playing field
		{
			b->x += b->v_x;
			b->y += b->v_y;
		}
	}
	
	// Horizontal collision
	if (b->x <= 0 || b->x >= 127 - b->width)
	{
		// Give score
		(b->x >= 0) ? (p1->score++) : (p2->score++);
		
		// Reset balls position
		b->x = BALL_STARTX;
		b->y = BALL_STARTY;
		start_dir *= -1;	// Reverse the start direction
		t_mod = 1;	// Reset time modifier
		// Reset balls velocity
		b->v_x = start_dir * BALL_SPEED / 2;
		b->v_y = start_dir * BALL_SPEED / 2;
	}
	
	// Paddle collision
	if (in_object (b, p1))
		bounce_paddle (b, p1);
	else if (in_object (b, p2))
		bounce_paddle (b, p2);
	
	draw_object (b);
}

/*	bounce_paddle
	Modifies the angle of reflection of the ball according to where on the paddle the ball hits.
	Hitting the paddle closer to the edge or from the y-direction results in a greater angle of reflection.
*/
void bounce_paddle (object * b, object * p)
{
	float mid_ball = b->y + b->height / 2;	// y-position of middle of ball
	float dy = fabs ((p->y + p->height) / 2 - mid_ball);	// Distance between where ball hits and center of paddle
	float mod = dy * BASE_MODIFIER;	// Set bounce modifier according to how close the edge of the paddle the ball hits
	
	b->v_x *= -1;
	// Decide direction of ball and set mod accordingly
	(b->v_x > 0) ? (b->v_x -= mod) : (b->v_x += mod);	// Check if moving right
	(b->v_y < 0) ? (b->v_y -= mod) : (b->v_y += mod);	// Check if moving up
	
	// Decide which part of the paddle the ball hits
	if (b->y + b->height > p->y && mid_ball < p->y + (p->height / 2.0f))	// Upper part of paddle
	{
		if (b->y + b->height >= p->y && b->y + b->height < p->y + 0.2f)	// Check if ball hits paddle from y-direction, if so then increase bounce angle
			b->v_y *= 1.02f;
		
		if (b->v_y > 0)	// Moving down
			b->v_y *= -1;
	}
	else if (mid_ball > p->y + (p->height / 2.0f) && b->y < p->y + p->height)	// Lower part of paddle
	{
		if (b->y <= p->y + p->height && b->y > p->y + p->height - 0.2f)	// Check if ball hits paddle from y-direction, if so then increase bounce angle
			b->v_y *= 1.02f;
			
		if (b->v_y < 0)	// Moving up
			b->v_y *= -1;
	}
	else	// Center of paddle
		b->v_y *= -1;
	
	while (in_object (b, p))	// Do nothing until ball is outside paddle
	{
		b->x += b->v_x;
		b->y += b->v_y;
	}
}

// High-score functions, used for writing and displaying the high-score

/*	sort_high_score
	Sorts the top three high-scores using a selection sort algorithm
*/
void sort_high_score ()
{
	int i, j;
	struct high_score h_temp;
	int max;
	for (i = 0; i < 3; i++)	// Sort the top three high-scores
	{
		int max = i;
		for (j = i; j < NUM_HIGH_SCRS; j++)
		{
			if (h_scrs[j].score > h_scrs[max].score)
			max = j;
		}
	
		if (max != i)	// Swap places of the scores
		{
			h_temp = h_scrs[i];
			h_scrs[i] = h_scrs[max];
			h_scrs[max] = h_temp;
		}
	}
}

/*	create_score_entry
	Creates a high-score entry to display in the high-score menu.
	The entry can show three digits, meaning the maximum high-score is 999.
*/
void create_score_entry (int index)
{
	int i;
	for (i = 0; i < 3; i++)
	{
		h_entry[i] = h_scrs[index].initials[i];
	}
	h_entry[3] = ':';
	h_entry[4] = ' ';
	(h_scrs[index].score >= 100) ? (h_entry[5] = h_scrs[index].score / 100 + '0') : (h_entry[5] = ' ');	// Third digit
	(h_scrs[index].score >= 10) ? (h_entry[6] = (h_scrs[index].score % 100) / 10 + '0') : (h_entry[6] = ' ');	// Second digit
	h_entry[7] = h_scrs[index].score % 10 + '0';	// First digit
	h_entry[8] = ' ';
}

/*	write_high_score
	Allows the user to enter three initials to save their high-score.
	If the initials exists in the high-score list, update their high-score.
	If the initials do not exist, create a new high-score entry.
*/
void write_high_score ()
{
	int i, j, k;
	int exists;
	char in[] = {' ', ' ', ' '};
	char c[] = {'4', '<', 'A', '>', '3', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
	
	display_string (0, "Enter 3 initials");
	display_string (1, "to save score");
	display_string (2, "");
	display_string (3, c);
	display_update ();
	i = 0;
	while (1)
	{
		if (btns & 1 << 2 && c[2] < 'Z')	// Pushbutton 3 is pressed, move to next char
		{
			c[2] += 1;
			display_string (3, c);
			display_update ();
			delay ();
		}
		else if (btns & 1 << 3 && c[2] > 'A')	// Pushbutton 4 is pressed, move to previous char
		{
			c[2] -= 1;
			display_string (3, c);
			display_update ();
			delay ();
		}
		else if (btns & 1 << 1)	// Pushbutton 2 is pressed, add char to initials
		{
			in[i] = c[2];
			c[7 + i] = in[i];
			i++;
			display_string (3, c);
			display_update ();
			delay ();
		}
		else if (btns & 1 && i > 0)	// Pushbutton 1 is pressed, remove char from initials
		{
			i--;
			in[i] = ' ';
			c[7 + i] = in[i];
			display_string (3, c);
			display_update ();
			delay ();
		}
		
		if (i == 3)	// Check if initials exist, if not create new entry for the initials
		{
			for (j = 0; j <= score_pos; j++)	// Go through all existing high-scores
			{
				for (k = 0; k < 3; k++)	// Go through all initals
				{
					if (h_scrs[j].initials[k] == in[k])
						exists = 1;
					else
					{
						exists = 0;
						break;
					}
				}
				
				if (exists)	// If initials are found in high-score list, break out of loop
					break;
			}
			
			if (exists)
				h_scrs[j].score++;
			else
			{
				for (k = 0; k < 3; k++)	// Write score
				{
					h_scrs[score_pos].initials[k] = in[k];
				}
				h_scrs[score_pos].score++;
				score_pos++;
			}
			
			break;
		}
	}
}

/*	delay
	Simple delay function, used in menus to wait between reading input from buttons
*/
void delay ()
{
	delay_count = 0;
	while (delay_count != 50);
}