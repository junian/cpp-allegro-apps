#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdlib.h>
#include <time.h>

#define SIZE_X       40
#define SIZE_Y       30
#define OBJECT_SIZE  20
#define SCREEN_W     (SIZE_X * OBJECT_SIZE)
#define SCREEN_H     (SIZE_Y * OBJECT_SIZE)

#define EAT          3
#define HEAD        -2
#define BODY        -3
#define WALL        -1
#define PATH         0

/* Ticks per move: snake moves every MOVE_TICKS timer events */
#define TICKS_PER_SEC  60
#define MOVE_TICKS     6

typedef struct stSNAKE {
    int x;
    int y;
    struct stSNAKE *next;
} SNAKE;

void add_body(SNAKE **snake, int x, int y, int map[SIZE_Y][SIZE_X]);
int  move_snake(SNAKE *snake, int moveX, int moveY, int map[SIZE_Y][SIZE_X]);

int main(int argc, char **argv)
{
    /* ------------------------------------------------------------------ */
    /*  Init                                                                */
    /* ------------------------------------------------------------------ */
    if (!al_init()) {
        fprintf(stderr, "Failed to initialize Allegro.\n");
        return -1;
    }
    if (!al_init_image_addon()) {
        fprintf(stderr, "Failed to initialize image addon.\n");
        return -1;
    }
    if (!al_init_primitives_addon()) {
        fprintf(stderr, "Failed to initialize primitives addon.\n");
        return -1;
    }
    if (!al_install_keyboard()) {
        fprintf(stderr, "Failed to install keyboard.\n");
        return -1;
    }

    ALLEGRO_DISPLAY *display = al_create_display(SCREEN_W, SCREEN_H);
    if (!display) {
        fprintf(stderr, "Failed to create display.\n");
        return -1;
    }
    al_set_window_title(display, "Team27, inc - Innocent Snake");

    ALLEGRO_TIMER *timer = al_create_timer(1.0 / TICKS_PER_SEC);
    if (!timer) {
        fprintf(stderr, "Failed to create timer.\n");
        al_destroy_display(display);
        return -1;
    }

    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    /* ------------------------------------------------------------------ */
    /*  Load / create bitmaps                                               */
    /* ------------------------------------------------------------------ */
    ALLEGRO_BITMAP *bmp_wall  = al_load_bitmap("wall.bmp");
    ALLEGRO_BITMAP *bmp_head  = al_load_bitmap("head.bmp");
    ALLEGRO_BITMAP *bmp_body  = al_load_bitmap("body.bmp");

    if (!bmp_wall || !bmp_head || !bmp_body) {
        fprintf(stderr, "Failed to load one or more bitmap assets.\n");
        al_destroy_display(display);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);
        return -1;
    }

    /* Path tile: plain white square */
    ALLEGRO_BITMAP *bmp_path = al_create_bitmap(OBJECT_SIZE, OBJECT_SIZE);
    al_set_target_bitmap(bmp_path);
    al_clear_to_color(al_map_rgb(255, 255, 255));

    /* Food: white background + red circle */
    ALLEGRO_BITMAP *bmp_eat = al_create_bitmap(OBJECT_SIZE, OBJECT_SIZE);
    al_set_target_bitmap(bmp_eat);
    al_clear_to_color(al_map_rgb(255, 255, 255));
    al_draw_filled_circle(OBJECT_SIZE / 2.0f, OBJECT_SIZE / 2.0f,
                          OBJECT_SIZE / 2.0f - 4, al_map_rgb(255, 0, 0));

    /* Pre-render the static map into a single bitmap */
    ALLEGRO_BITMAP *bmp_bg = al_create_bitmap(SCREEN_W, SCREEN_H);

    /* ------------------------------------------------------------------ */
    /*  Map definition                                                      */
    /* ------------------------------------------------------------------ */
    int map[SIZE_Y][SIZE_X] = {
        {-1,-1,-1,-1,-1,-1,-1, 0, 0, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0, 0, 0,-1,-1,-1,-1, 0, 0, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0,-1,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1},
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1, 0, 0, 0, 0},
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1,-1, 0, 0, 0,-1,-1,-1,-1,-1,-1, 0, 0,-1},
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0, 0, 0,-1,-1,-1,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1},
        {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1},
        {-1,-1,-1,-1,-1,-1,-1, 0, 0, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0, 0, 0,-1,-1,-1,-1, 0, 0, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1}
    };

    /* Pre-render static background */
    al_set_target_bitmap(bmp_bg);
    for (int i = 0; i < SIZE_Y; i++) {
        for (int j = 0; j < SIZE_X; j++) {
            switch (map[i][j]) {
                case PATH: al_draw_bitmap(bmp_path, j * OBJECT_SIZE, i * OBJECT_SIZE, 0); break;
                case WALL: al_draw_bitmap(bmp_wall, j * OBJECT_SIZE, i * OBJECT_SIZE, 0); break;
            }
        }
    }

    /* ------------------------------------------------------------------ */
    /*  Snake init                                                          */
    /* ------------------------------------------------------------------ */
    srand((unsigned) time(NULL));

    SNAKE *snake   = NULL;
    SNAKE *pointer = NULL;

    add_body(&snake, 3, 1, map);
    map[snake->y][snake->x] = HEAD;
    add_body(&snake, 2, 1, map);
    add_body(&snake, 1, 1, map);

    int moveX = 1, moveY = 0;
    int posX = 0, posY = 0;
    int loaded = 0;
    int counter = 0;

    /* ------------------------------------------------------------------ */
    /*  Game loop                                                           */
    /* ------------------------------------------------------------------ */
    al_start_timer(timer);
    bool running = true;

    while (running) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
            break;
        }

        if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (ev.keyboard.keycode) {
                case ALLEGRO_KEY_ESCAPE: running = false; break;
                case ALLEGRO_KEY_UP:    if (moveY == 0) { moveX =  0; moveY = -1; } break;
                case ALLEGRO_KEY_DOWN:  if (moveY == 0) { moveX =  0; moveY =  1; } break;
                case ALLEGRO_KEY_LEFT:  if (moveX == 0) { moveX = -1; moveY =  0; } break;
                case ALLEGRO_KEY_RIGHT: if (moveX == 0) { moveX =  1; moveY =  0; } break;
            }
        }

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            /* Spawn food if none on map */
            if (loaded == 0) {
                do {
                    posX = rand() % (SIZE_X - 1) + 1;
                    posY = rand() % (SIZE_Y - 1) + 1;
                } while (map[posY][posX] != PATH);
                map[posY][posX] = EAT;
                loaded = 1;
            }

            counter++;
            if (counter >= MOVE_TICKS) {
                counter = 0;
                int x_his = snake->x;
                int y_his = snake->y;

                if (!move_snake(snake, moveX, moveY, map)) {
                    running = false;
                    break;
                }

                /* Grew: food was eaten */
                if (map[posY][posX] != EAT) {
                    add_body(&snake, x_his, y_his, map);
                    loaded = 0;
                }
            }

            /* Render */
            al_set_target_backbuffer(display);
            al_draw_bitmap(bmp_bg, 0, 0, 0);
            al_draw_bitmap(bmp_eat, posX * OBJECT_SIZE, posY * OBJECT_SIZE, 0);

            for (pointer = snake; pointer != NULL; pointer = pointer->next) {
                switch (map[pointer->y][pointer->x]) {
                    case BODY: al_draw_bitmap(bmp_body, pointer->x * OBJECT_SIZE, pointer->y * OBJECT_SIZE, 0); break;
                    case HEAD: al_draw_bitmap(bmp_head, pointer->x * OBJECT_SIZE, pointer->y * OBJECT_SIZE, 0); break;
                }
            }

            al_flip_display();
        }
    }

    /* ------------------------------------------------------------------ */
    /*  Cleanup                                                             */
    /* ------------------------------------------------------------------ */
    /* Free snake linked list */
    while (snake) {
        SNAKE *tmp = snake;
        snake = snake->next;
        free(tmp);
    }

    al_destroy_bitmap(bmp_head);
    al_destroy_bitmap(bmp_body);
    al_destroy_bitmap(bmp_wall);
    al_destroy_bitmap(bmp_path);
    al_destroy_bitmap(bmp_eat);
    al_destroy_bitmap(bmp_bg);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);

    return 0;
}

/* ------------------------------------------------------------------ */
/*  Snake helpers                                                       */
/* ------------------------------------------------------------------ */

void add_body(SNAKE **snake, int x, int y, int map[SIZE_Y][SIZE_X])
{
    SNAKE *new_body = (SNAKE *) malloc(sizeof(SNAKE));
    if (new_body == NULL) return;
    new_body->next = *snake;
    *snake = new_body;
    new_body->x = x;
    new_body->y = y;
    map[y][x] = BODY;
}

int move_snake(SNAKE *snake, int moveX, int moveY, int map[SIZE_Y][SIZE_X])
{
    map[snake->y][snake->x] = PATH;
    while (true) {
        if (snake->next == NULL) {
            snake->x += moveX;
            if (snake->x >= SIZE_X) snake->x = 0;
            else if (snake->x < 0)  snake->x = SIZE_X - 1;

            snake->y += moveY;
            if (snake->y >= SIZE_Y) snake->y = 0;
            else if (snake->y < 0)  snake->y = SIZE_Y - 1;

            if (map[snake->y][snake->x] < 0) {
                fprintf(stderr, "Snake hit a wall!\n");
                return 0;
            }
            map[snake->y][snake->x] = HEAD;
            return 1;
        }
        snake->x = snake->next->x;
        snake->y = snake->next->y;
        map[snake->y][snake->x] = BODY;
        snake = snake->next;
    }
}
