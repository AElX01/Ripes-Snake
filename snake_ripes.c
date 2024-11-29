#include "ripes_system.h"

#define SNAKE_COLOR 0xff0000 // RED
#define APPLE_COLOR 0x00ff00 // GREEN
#define MAX_SNAKE_LENGTH 50 
#define PIXEL_SIZE 2
#define GAME_SPEED 3000 
#define SNAKE_START_X 10
#define SNAKE_START_Y 10


typedef enum {GAME_OVER, PLAYING} Status;

typedef enum {UP, DOWN, LEFT, RIGHT} Keys;

typedef enum {FALSE, TRUE} Boolean;

typedef struct {unsigned x, y;} Apple;

typedef struct {unsigned x, y; Keys direction;} SnakeSegment;

typedef struct {SnakeSegment segments[MAX_SNAKE_LENGTH]; int length; Apple apple;} Snake;

unsigned *led_base = (unsigned*) LED_MATRIX_0_BASE;
unsigned *switch_base = (unsigned*) SWITCHES_0_BASE;
unsigned *up = (unsigned*) D_PAD_0_UP;
unsigned *down = (unsigned*) D_PAD_0_DOWN;
unsigned *left = (unsigned*) D_PAD_0_LEFT;
unsigned *right = (unsigned*) D_PAD_0_RIGHT;
unsigned seed = 12345; // SEED USED TO GENERATE RANDOM NUMBERS

unsigned rand_ripes(); // CUSTOM RAND FUNCTION
unsigned rand_mod(unsigned max); // PART OF CUSTOM RAND FUNCTION
void create_snake(Snake *snake);
void clear_screen();
void delay();
void init_game(Snake *snake);
void init_apple_coordinates(Snake *snake, int seed);
void move_snake(Snake *snake);
Boolean wall_collision(Snake *snake);
Boolean snake_collision(Snake *snake);
Boolean snake_collision_with_apple(Snake *snake);
void change_head_direction(Snake *snake);
void move_snake_head(Snake *snake);


void main(){
    Snake snake;
    Status gameStatus = PLAYING;
    Boolean restart_flag;
    unsigned cnt = 0;

    create_snake(&snake);
    clear_screen();
    while (TRUE) {
        while (gameStatus == PLAYING) {
            clear_screen();
            if (!(snake.apple.x && snake.apple.y)) init_apple_coordinates(&snake, cnt);

            init_game(&snake);
            move_snake(&snake);
            change_head_direction(&snake);
            move_snake_head(&snake);

            gameStatus = (wall_collision(&snake) || snake_collision(&snake)) ? GAME_OVER : PLAYING;

            if (snake_collision_with_apple(&snake)) {
                ++snake.length;
                snake.apple.x = 0;
                snake.apple.y = 0;
            }

            delay();
            ++cnt;
        }

        restart_flag = (*(switch_base) & 0x01 && gameStatus == GAME_OVER) ? TRUE : FALSE;
        
        if (restart_flag) {
            create_snake(&snake);
            gameStatus = PLAYING;
        }
    }
}


unsigned rand_ripes() {
    seed = (1103515245 * seed + 12345) % 0x7FFFFFFF;
    return seed;
}

unsigned rand_mod(unsigned max) {return rand_ripes() % max;}

void create_snake(Snake *snake) {
    snake->length = 1;
    snake->segments[0].x = SNAKE_START_X;
    snake->segments[0].y = SNAKE_START_Y;
    snake->segments[0].direction = RIGHT;
    snake->apple.x = 0;
    snake->apple.y = 0;
}

void clear_screen() {
    for (int y = PIXEL_SIZE; y < LED_MATRIX_0_HEIGHT; ++y) {
        for (int x = PIXEL_SIZE; x < LED_MATRIX_0_WIDTH; ++x) 
            *(led_base + y * LED_MATRIX_0_WIDTH + x) = 0x000000;
    }

    for (int i = 0; i < LED_MATRIX_0_WIDTH; ++i) {
        for (int j = 0; j < PIXEL_SIZE; ++j) 
            *(led_base + j * LED_MATRIX_0_WIDTH + i) = 0x000000;   
    }
}

void delay() {
    volatile unsigned delay_var = 0;
    for (zint i = 0; i < GAME_SPEED; ++i, ++delay_var);
}

void init_game(Snake *snake) {
    for (int i = 0; i < snake->length; ++i) {
        for (int x = 0; x < PIXEL_SIZE; ++x) {
            for (int y = 0; y < PIXEL_SIZE; ++y) {
                if (!i) *(led_base + (snake->apple.y + y) * LED_MATRIX_0_WIDTH + (snake->apple.x + x)) = APPLE_COLOR;
                
                *(led_base + (snake->segments[i].y + y) * LED_MATRIX_0_WIDTH + (snake->segments[i].x + x)) = SNAKE_COLOR;
            }
        }
    }
}

void init_apple_coordinates(Snake *snake, int seed){
    int x = rand_mod(LED_MATRIX_0_WIDTH - PIXEL_SIZE);
    int y = rand_mod(LED_MATRIX_0_HEIGHT - PIXEL_SIZE);
    
    if (x % 2) 
        --x;
    if (y % 2) 
        --y;

    if (x < PIXEL_SIZE) 
        x = PIXEL_SIZE;
    if (y < PIXEL_SIZE) 
        y = PIXEL_SIZE;

    snake->apple.x = x;
    snake->apple.y = y;
}

void move_snake(Snake *snake){
    for (int i = snake->length - 1; i > 0; --i) {
        snake->segments[i].x = snake->segments[i - 1].x;
        snake->segments[i].y = snake->segments[i - 1].y;
    }
}

Boolean wall_collision(Snake *snake){
    return (snake->segments[0].x < PIXEL_SIZE || 
           snake->segments[0].x >= LED_MATRIX_0_WIDTH - PIXEL_SIZE || 
           snake->segments[0].y < PIXEL_SIZE || 
           snake->segments[0].y >= LED_MATRIX_0_HEIGHT - PIXEL_SIZE) ? TRUE : FALSE;
}

Boolean snake_collision(Snake *snake){
    for (int i = 1; i < snake->length; ++i) {
        if (snake->segments[0].x == snake->segments[i].x && snake->segments[0].y == snake->segments[i].y) 
            return TRUE;
    }
    return FALSE;
}

Boolean snake_collision_with_apple(Snake *snake){
    return (snake->segments[0].x == snake->apple.x && snake->segments[0].y == snake->apple.y) ? TRUE : FALSE;
}

void change_head_direction(Snake *snake){
    if (*up && snake->segments[0].direction != DOWN) 
        snake->segments[0].direction = UP;

    else if (*down && snake->segments[0].direction != UP) 
        snake->segments[0].direction = DOWN;

    else if (*left && snake->segments[0].direction != RIGHT) 
        snake->segments[0].direction = LEFT;

    else if (*right && snake->segments[0].direction != LEFT) 
        snake->segments[0].direction = RIGHT;
}

void move_snake_head(Snake *snake){
    if (snake->segments[0].direction == UP) 
        snake->segments[0].y -= PIXEL_SIZE;

    else if (snake->segments[0].direction == DOWN) 
        snake->segments[0].y += PIXEL_SIZE;

    else if (snake->segments[0].direction == LEFT) 
        snake->segments[0].x -= PIXEL_SIZE;

    else 
        snake->segments[0].x += PIXEL_SIZE;
}