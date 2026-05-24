#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>

#define SCREEN_W  960
#define SCREEN_H  600

static ALLEGRO_COLOR BLACK;
static ALLEGRO_COLOR WHITE;

void lineForward(int *x, int *y, int lengthX, int lengthY);
void drawHilbert(int *x, int *y, int lengthX, int lengthY, int lvl);

int main(int argc, char **argv)
{
    /* ------------------------------------------------------------------ */
    /*  Init                                                                */
    /* ------------------------------------------------------------------ */
    if (!al_init()) {
        fprintf(stderr, "Failed to initialize Allegro.\n");
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

    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));

    /* Colors must be initialised after al_init() */
    BLACK = al_map_rgb(0,   0,   0);
    WHITE = al_map_rgb(255, 255, 255);

    /* ------------------------------------------------------------------ */
    /*  Game loop                                                           */
    /* ------------------------------------------------------------------ */
    int lvl = 1;
    bool running = true;
    bool redraw  = true;   /* draw on first frame and after level change */

    while (running) {
        if (redraw) {
            /* Build title string */
            char title[32];
            snprintf(title, sizeof(title), "Hilbert Curve Level %d  (UP/DOWN to change, ESC to quit)", lvl);
            al_set_window_title(display, title);

            /* Compute starting position and segment length */
            int x = 10, y = 590, length = 512;
            for (int i = 1; i <= lvl; i++)
                if (length >= 8) length /= 2;

            /* Draw curve onto the backbuffer */
            al_set_target_backbuffer(display);
            al_clear_to_color(WHITE);
            drawHilbert(&x, &y, 0, length, lvl);
            al_flip_display();

            redraw = false;
        }

        /* Block until the next event */
        ALLEGRO_EVENT ev;
        al_wait_for_event(queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        } else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (ev.keyboard.keycode) {
                case ALLEGRO_KEY_ESCAPE:
                    running = false;
                    break;
                case ALLEGRO_KEY_UP:
                    if (lvl < 9) { lvl++; redraw = true; }
                    break;
                case ALLEGRO_KEY_DOWN:
                    if (lvl > 1) { lvl--; redraw = true; }
                    break;
            }
        }
    }

    /* ------------------------------------------------------------------ */
    /*  Cleanup                                                             */
    /* ------------------------------------------------------------------ */
    al_destroy_event_queue(queue);
    al_destroy_display(display);
    return 0;
}

/* Draw a line segment from (*x, *y) by (lengthX, lengthY) and advance */
void lineForward(int *x, int *y, int lengthX, int lengthY)
{
    al_draw_line((float)*x,           (float)*y,
                 (float)(*x + lengthX), (float)(*y + lengthY),
                 BLACK, 1.0f);
    *x += lengthX;
    *y += lengthY;
}

/* Recursive Hilbert curve */
void drawHilbert(int *x, int *y, int lengthX, int lengthY, int lvl)
{
    if (lvl > 0) {
        drawHilbert (x, y,  lengthY,  lengthX, lvl - 1);
        lineForward (x, y,  lengthX, -lengthY);
        drawHilbert (x, y,  lengthX,  lengthY, lvl - 1);
        lineForward (x, y,  lengthY, -lengthX);
        drawHilbert (x, y,  lengthX,  lengthY, lvl - 1);
        lineForward (x, y, -lengthX,  lengthY);
        drawHilbert (x, y, -lengthY, -lengthX, lvl - 1);
    }
}
