/*	pong.c
	Includes functions for running the game

	Written by Rasmus Sundbom, 2024-02-28
	
	For copyright and licensing, see file COPYING	*/

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declarations for these labs */
#include <math.h>	/* Necessary for absolute value function */
#include "data.h"
#include "functions.h"

/* Interrupt Service Routine */
void user_isr ()
{
	if ((IFS(0) & 0x100) == 0x100)	// Interrupt is from Timer 2
	{
		// Clear the interrupt flag
		IFS(0) = (IFS(0) & 0xeff);	// Set bit 8 of IFS0 to 0, which clears T2IF
		
		delay_count++;
		if (delay_count > 100)
		{
			delay_count = 0;
		}
		
		t_mod += 0.0005f;	// Increase speed of ball over time
		
		// Timer-controlled polling of input
		btns = getbtns();
		
		// Update display if in-game
		if (in_game)
			display_image (0, screen);
	}
	
	return;
}

/*	multi_game
	Function for playing the game in multiplayer mode.
	Checks for input from both paddles.
*/
void multi_game ()
{
	game_init ();
	
	while (in_game)
	{
		move_ball (&b, &p1, &p2);
		move_paddle (&p1);
		move_paddle (&p2);
		draw_score (p1.score, p2.score);
		
		// Check for input
		if (btns & 1 << 3)	// Pushbutton 4 is pressed
			p1.v_y = -PADDLE_SPEED;
		else if (btns & 1 << 2)	// Pushbutton 3 is pressed
			p1.v_y = PADDLE_SPEED;
		else
			p1.v_y = 0.0f;
		
		if (btns & 1 << 1)	// Pushbutton 2 is pressed
			p2.v_y = -PADDLE_SPEED;
		else if (btns & 1)	// Pushbutton 1 is pressed
			p2.v_y = PADDLE_SPEED;
		else
			p2.v_y = 0.0f;
		
		draw_screen();
		
		// Check for end of game
		if (p1.score == SCORE_WIN || p2.score == SCORE_WIN)
		{
			in_game = 0;
			write_high_score ();
			window = 0;
		}
	}
}

/*	single_game
	Function for playing the game in single-player mode.
	Checks for input from paddle 1.
	Paddle 2 is AI controlled.
*/
void single_game (int diff)
{
	game_init ();
	
	float ai_speed;
	(diff == 0) ? (ai_speed = EASY_SPEED) : (ai_speed = HARD_SPEED);	// Set the speed of AI paddle according to difficulty chosen
	
	while (in_game)
	{
		move_ball (&b, &p1, &p2);
		move_paddle (&p1);
		move_paddle (&p2);
		draw_score (p1.score, p2.score);
		
		// Check for input
		if (btns & 1 << 3)	// Pushbutton 4 is pressed
			p1.v_y = -PADDLE_SPEED;
		else if (btns & 1 << 2)	// Pushbutton 3 is pressed
			p1.v_y = PADDLE_SPEED;
		else
			p1.v_y = 0.0f;
		
		// AI
		if (b.y < p2.y + p2.height / 2)		// Ball is above paddle
			p2.v_y = -ai_speed;
		else if (b.y > p2.y + p2.height / 2)	// Ball is below paddle
			p2.v_y = ai_speed;
		else
			p2.v_y = 0.0f;
		
		draw_screen();
		
		// Check for end of game
		if (p1.score == SCORE_WIN)
		{
			in_game = 0;
			write_high_score ();
			window = 0;
		}
		else if (p2.score == SCORE_WIN)
		{
			in_game = 0;
			window = 0;
		}
	}
}

/*	game
	Decides which window is displayed and which game mode is chosen
*/
void game ()
{
	int i;
	int diff;	// 0 is easy, 1 is hard
	
	while (1)
	{
		switch (window)
		{
			case 0:		// Main menu
				display_string (0, "PONG");
				display_string (1, "4 Single-player");
				display_string (2, "3 Multiplayer");
				display_string (3, "2 High-score");
				display_update ();
				
				delay ();
				
				while (1)
				{
					if (btns & 1 << 3)	// Pushbutton 4 is pressed
					{
						window = 1;
						break;
					}
					else if (btns & 1 << 2)	// Pushbutton 3 is pressed
					{
						window = 3;
						break;
					}
					else if (btns & 1 << 1)	// Pushbutton 2 is pressed
					{
						window = 4;
						break;
					}
				}
				break;
			case 1:		// Choose difficulty
				display_string (0, "DIFFICULTY");
				display_string (1, "1 Easy");
				display_string (2, "2 Hard");
				display_string (3, "3 Return");
				display_update ();
				
				delay ();
				
				while (1)
				{
					if (btns & 1)	// Pushbutton 1 is pressed
					{
						diff = 0;
						window = 2;
						break;
					}
					else if (btns & 1 << 1)	// Pushbutton 2 is pressed
					{
						diff = 1;
						window = 2;
						break;
					}
					else if (btns & 1 << 2)	// Pushbutton 3 is pressed
					{
						window = 0;
						break;
					}
				}
				break;
			case 2:		// Single-player
				in_game = 1;
				single_game (diff);
				break;
			case 3:		// Multiplayer
				in_game = 1;
				multi_game ();
				break;
			case 4:		// High-score menu
				sort_high_score ();
				for (i = 0; i < 3; i++)
				{
					create_score_entry (i);
					display_string (i, h_entry);
				}
				display_string (3, "3 Return");
				display_update ();
				while (1)
				{
					if (btns & 1 << 2)	// Pushbutton 3 is pressed
					{
						window = 0;
						break;
					}
				}
				break;
		}
	}
}