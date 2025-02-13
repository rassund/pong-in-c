/*	functions.h
	Header file for functions
	For copyright and licensing, see file COPYING	*/

// Draw functions, used for showing things on the display
void draw_object (object * o);
void draw_screen ();
void draw_score (int s1, int s2);

// Initialization functions
void init_btns ();
void start_init ();
void game_init ();

// Game functions, used for input, moving objects and collision
int getbtns ();
void move_paddle (object * p);
int in_object (object * o1, object * o2);
void move_ball (object * b, object * p1, object * p2);
void bounce_paddle (object * b, object * p);

// High-score functions, used for writing and displaying the high-score
void sort_high_score ();
void create_score_entry (int index);
void write_high_score ();

// Simple delay function, used in menus to wait between reading input from buttons
void delay ();