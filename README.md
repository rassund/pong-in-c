# Pong in C

## Description
This project is a recreation of the classic Pong game, made for a course in hardware engineering at the Royal Institute of Technology, Sweden. It is developed mainly in the C language and is designed to be run on a chipKIT Uno32 board together with a Basic I/O shield. Most of the game logic is included in the files pong.c, functions.c, and data.h. The file pong.c is the main file, controlling the running of the game, while functions.c defines various functions used for both the game and the menus. The header file data.h defines most of the constants, struct declarations, and global variables.

## Things I learned
This project greatly increased my understanding of the C language and I/O operations.

## How it works
When the game is started, the player is presented with a menu consisting of three alternatives: single-player, multiplayer, and high-score.
The single-player mode allows the player to choose between an easy and a hard AI opponent, while the multiplayer mode allows two players to face each other on the same board.
Once the game has started, players control their paddles using push buttons on the Basic I/O shield. The game ends when either player reaches 8 points.
