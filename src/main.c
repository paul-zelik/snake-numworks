#include <eadk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define TILE_SIZE 10
#define GRID_WIDTH (EADK_SCREEN_WIDTH / TILE_SIZE)
#define GRID_HEIGHT (EADK_SCREEN_HEIGHT / TILE_SIZE)
#define MAX_SNAKE_LENGTH (GRID_WIDTH * GRID_HEIGHT)

typedef struct {
    uint16_t x;
    uint16_t y;
} SnakeSegment;

SnakeSegment snake[MAX_SNAKE_LENGTH];
int snake_length;
int head_x, head_y;
int food_x, food_y;
int dx, dy;
int score;
bool game_over;

const eadk_color_t COLOR_BACKGROUND = eadk_color_black;
const eadk_color_t COLOR_SNAKE = eadk_color_green;
const eadk_color_t COLOR_FOOD = eadk_color_red;
const eadk_color_t COLOR_TEXT = eadk_color_white;

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Snake";
const uint32_t eadk_api_level __attribute__((section(".rodata.eadk_api_level"))) = 0;

void game_init() {
    snake_length = 3;
    head_x = GRID_WIDTH / 2;
    head_y = GRID_HEIGHT / 2;

    for (int i = 0; i < snake_length; i++) {
        snake[i].x = head_x - i;
        snake[i].y = head_y;
    }

    dx = 1;
    dy = 0;
    score = 0;
    game_over = false;

    food_x = eadk_random() % GRID_WIDTH;
    food_y = eadk_random() % GRID_HEIGHT;

    eadk_display_push_rect_uniform(eadk_screen_rect, COLOR_BACKGROUND);
}

void draw_square(uint16_t x, uint16_t y, eadk_color_t color) {
    eadk_rect_t rect = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
    eadk_display_push_rect_uniform(rect, color);
}

void draw_snake() {
    for (int i = 0; i < snake_length; i++) {
        draw_square(snake[i].x, snake[i].y, COLOR_SNAKE);
    }
}

void clear_tail() {
    draw_square(snake[snake_length].x, snake[snake_length].y, COLOR_BACKGROUND);
}

void draw_food() {
    draw_square(food_x, food_y, COLOR_FOOD);
}

void place_food() {
    bool collision;
    do {
        collision = false;
        food_x = eadk_random() % GRID_WIDTH;
        food_y = eadk_random() % GRID_HEIGHT;

        for (int i = 0; i < snake_length; i++) {
            if (snake[i].x == food_x && snake[i].y == food_y) {
                collision = true;
                break;
            }
        }
    } while (collision);
    draw_food();
}

void game_update() {
    if (game_over) {
        return;
    }

    draw_square(snake[snake_length - 1].x, snake[snake_length - 1].y, COLOR_BACKGROUND);

    for (int i = snake_length - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }

    head_x += dx;
    head_y += dy;

    snake[0].x = head_x;
    snake[0].y = head_y;

    if (head_x < 0 || head_x >= GRID_WIDTH || head_y < 0 || head_y >= GRID_HEIGHT) {
        game_over = true;
        return;
    }

    for (int i = 1; i < snake_length; i++) {
        if (head_x == snake[i].x && head_y == snake[i].y) {
            game_over = true;
            return;
        }
    }

    if (head_x == food_x && head_y == food_y) {
        score += 10;
        snake_length++;
        if (snake_length > MAX_SNAKE_LENGTH) {
            snake_length = MAX_SNAKE_LENGTH;
        }
        place_food();
    }
}

void display_score() {
    char score_str[20];
    sprintf(score_str, "Score: %d", score);
    eadk_display_push_rect_uniform((eadk_rect_t){0, 0, EADK_SCREEN_WIDTH, TILE_SIZE}, COLOR_BACKGROUND);
    eadk_display_draw_string(score_str, (eadk_point_t){5, 2}, false, COLOR_TEXT, COLOR_BACKGROUND);
}

int main(int argc, char * argv[]) {
    game_init();

    eadk_timing_msleep(1000);

    while (!game_over) {
        eadk_keyboard_state_t keyboard = eadk_keyboard_scan();

        if (eadk_keyboard_key_down(keyboard, eadk_key_left) && dx == 0) {
            dx = -1;
            dy = 0;
        } else if (eadk_keyboard_key_down(keyboard, eadk_key_right) && dx == 0) {
            dx = 1;
            dy = 0;
        } else if (eadk_keyboard_key_down(keyboard, eadk_key_up) && dy == 0) {
            dx = 0;
            dy = -1;
        } else if (eadk_keyboard_key_down(keyboard, eadk_key_down) && dy == 0) {
            dx = 0;
            dy = 1;
        } else if (eadk_keyboard_key_down(keyboard, eadk_key_back)) {
            game_over = true;
        }

        game_update();
        draw_snake();
        draw_food();
        display_score();

        eadk_timing_msleep(150 - (score / 2));
        if (eadk_timing_msleep < 50) {
            eadk_timing_msleep(50);
        }
    }

    eadk_display_push_rect_uniform(eadk_screen_rect, eadk_color_black);
    eadk_display_draw_string("GAME OVER!", (eadk_point_t){(EADK_SCREEN_WIDTH - strlen("GAME OVER!")*6)/2, EADK_SCREEN_HEIGHT/2 - 10}, true, eadk_color_red, eadk_color_black);
    char final_score_str[30];
    sprintf(final_score_str, "Final Score: %d", score);
    eadk_display_draw_string(final_score_str, (eadk_point_t){(EADK_SCREEN_WIDTH - strlen(final_score_str)*6)/2, EADK_SCREEN_HEIGHT/2 + 10}, false, eadk_color_white, eadk_color_black);
    
    while(true) {
        eadk_keyboard_state_t keyboard = eadk_keyboard_scan();
        if (eadk_keyboard_key_down(keyboard, eadk_key_back)) {
            break;
        }
        eadk_timing_msleep(100);
    }

    return 0;
}
