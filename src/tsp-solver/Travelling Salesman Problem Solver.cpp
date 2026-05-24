#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAKS    500
#define RADIUS  3
#define EMPTY   -1

#define SCREEN_W 1280
#define SCREEN_H 720

typedef struct {
    double length;
    int next_city;
} ROUTE;

typedef struct {
    double x;
    double y;
    ROUTE *city_route;
} KOORD;

double panjang(KOORD *dot1, KOORD *dot2);
int *generate_shortest_route(KOORD *city, int n, int *start, double *total_length);
void quicksort(ROUTE *data, int left, int right);
void generate_city_connection(KOORD *city, int n);
void draw_line_bmp(ALLEGRO_BITMAP *bmp, KOORD *dot1, KOORD *dot2);
void make_circle(ALLEGRO_BITMAP *dest, KOORD *city);

int main(int argc, char **argv)
{
    /* --- Init core --- */
    if (!al_init()) {
        fprintf(stderr, "Failed to initialize Allegro.\n");
        return -1;
    }

    if (!al_init_primitives_addon()) {
        fprintf(stderr, "Failed to initialize primitives addon.\n");
        return -1;
    }

    al_init_font_addon();
    al_init_ttf_addon();

    if (!al_install_keyboard()) {
        fprintf(stderr, "Failed to install keyboard.\n");
        return -1;
    }

    /* --- Display --- */
    ALLEGRO_DISPLAY *display = al_create_display(SCREEN_W, SCREEN_H);
    if (!display) {
        fprintf(stderr, "Failed to create display.\n");
        return -1;
    }
    al_set_window_title(display, "Travelling Salesman Problem by Junian.dev");

    /* --- Font: fall back to built-in if no TTF available --- */
    ALLEGRO_FONT *font = al_create_builtin_font();
    if (!font) {
        fprintf(stderr, "Failed to load font.\n");
        al_destroy_display(display);
        return -1;
    }

    /* --- Event queue --- */
    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));

    /* --- Load city data --- */
    FILE *finput = fopen("salesman_data.txt", "r");
    if (!finput) {
        fprintf(stderr, "Cannot open salesman_data.txt\n");
        al_destroy_font(font);
        al_destroy_display(display);
        al_destroy_event_queue(queue);
        return -1;
    }

    int n;
    fscanf(finput, "%d", &n);

    KOORD *city = (KOORD *) malloc(sizeof(KOORD) * n);

    /* --- Off-screen bitmap for the city map --- */
    ALLEGRO_BITMAP *city_box = al_create_bitmap(MAKS + 4, MAKS + 4 + 16);
    al_set_target_bitmap(city_box);
    al_clear_to_color(al_map_rgb(255, 255, 255));

    for (int i = 0; i < n; i++) {
        fscanf(finput, "%lf %lf", &city[i].x, &city[i].y);
        make_circle(city_box, &city[i]);
        city[i].city_route = (ROUTE *) malloc(sizeof(ROUTE) * (n - 1));
    }
    fclose(finput);

    /* --- Solve TSP --- */
    generate_city_connection(city, n);

    double length = 0.0;
    int start = 0;
    int *shortest_path = generate_shortest_route(city, n, &start, &length);

    /* --- Draw route onto city_box --- */
    al_set_target_bitmap(city_box);
    int g = start;
    for (int i = 0; i < n; i++) {
        draw_line_bmp(city_box, &city[g], &city[city[g].city_route[shortest_path[i]].next_city]);
        g = city[g].city_route[shortest_path[i]].next_city;
    }

    /* --- Main render loop --- */
    bool running = true;
    while (running) {
        /* Draw */
        al_set_target_backbuffer(display);
        al_clear_to_color(al_map_rgb(255, 255, 255));

        int bmp_x = (SCREEN_W - al_get_bitmap_width(city_box)) / 2;
        int bmp_y = (SCREEN_H - al_get_bitmap_height(city_box)) / 2;
        al_draw_bitmap(city_box, bmp_x, bmp_y, 0);

        al_draw_textf(font, al_map_rgb(0, 0, 0), 10, 10, 0,
                      "Shortest path: %.2f px  (Press ESC to quit)", length);

        al_flip_display();

        /* Events */
        ALLEGRO_EVENT ev;
        al_wait_for_event(queue, &ev);

        if (ev.type == ALLEGRO_EVENT_KEY_DOWN &&
            ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            running = false;
        } else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        }
    }

    /* --- Cleanup --- */
    for (int i = 0; i < n; i++)
        free(city[i].city_route);
    free(city);
    free(shortest_path);

    al_destroy_bitmap(city_box);
    al_destroy_font(font);
    al_destroy_event_queue(queue);
    al_destroy_display(display);

    return 0;
}

/* ------------------------------------------------------------------ */
/*  Drawing helpers                                                     */
/* ------------------------------------------------------------------ */

void make_circle(ALLEGRO_BITMAP *dest, KOORD *city)
{
    al_set_target_bitmap(dest);
    al_draw_filled_circle((float)(city->x + 2), (float)(city->y + 2),
                          RADIUS, al_map_rgb(0, 200, 0));
}

void draw_line_bmp(ALLEGRO_BITMAP *bmp, KOORD *dot1, KOORD *dot2)
{
    al_set_target_bitmap(bmp);
    al_draw_line((float)(dot1->x + 2), (float)(dot1->y + 2),
                 (float)(dot2->x + 2), (float)(dot2->y + 2),
                 al_map_rgb(220, 0, 0), 1.0f);
}

/* ------------------------------------------------------------------ */
/*  TSP algorithm (unchanged logic)                                     */
/* ------------------------------------------------------------------ */

double panjang(KOORD *dot1, KOORD *dot2)
{
    return sqrt(pow(dot2->x - dot1->x, 2) + pow(dot2->y - dot1->y, 2));
}

void quicksort(ROUTE *data, int left, int right)
{
    if (left >= right) return;
    int pivot = left, pointer = right;
    int pointerMove = -1;
    ROUTE rtemp;
    while (pivot != pointer) {
        if ((data[pivot].length > data[pointer].length && pointerMove == -1) ||
            (data[pivot].length < data[pointer].length && pointerMove == +1)) {
            rtemp = data[pivot];
            data[pivot] = data[pointer];
            data[pointer] = rtemp;

            int tmp = pivot;
            pivot = pointer;
            pointer = tmp;
            pointerMove *= -1;
        }
        pointer += pointerMove;
    }
    quicksort(data, left, pivot - 1);
    quicksort(data, pivot + 1, right);
}

void generate_city_connection(KOORD *city, int n)
{
    for (int i = 0; i < n; i++) {
        int k = 0;
        for (int j = 0; j < n; j++) {
            if (j != i) {
                city[i].city_route[k].next_city = j;
                city[i].city_route[k].length = panjang(&city[i], &city[j]);
                k++;
            }
        }
        quicksort(city[i].city_route, 0, n - 2);
    }
}

int *generate_shortest_route(KOORD *city, int n, int *start, double *total_length_out)
{
    bool *city_list = (bool *) malloc(sizeof(bool) * n);
    int *list = (int *) malloc(sizeof(int) * n);
    int *shortest_path = (int *) malloc(sizeof(int) * n);

    double total_length = EMPTY;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            city_list[j] = true;

        double last_length = 0.0;
        int go_next_city = i;

        for (int j = 0; j < n - 1; j++) {
            int k = 0;
            city_list[go_next_city] = false;
            while (!city_list[city[go_next_city].city_route[k].next_city])
                k++;

            list[j] = k;
            last_length += city[go_next_city].city_route[k].length;
            go_next_city = city[go_next_city].city_route[k].next_city;
        }

        int k = 0;
        while (city[go_next_city].city_route[k].next_city != i)
            k++;
        list[n - 1] = k;
        last_length += city[go_next_city].city_route[k].length;

        if (last_length < total_length || total_length == EMPTY) {
            total_length = last_length;
            *total_length_out = total_length;
            *start = i;

            int *tmp = list;
            list = shortest_path;
            shortest_path = tmp;
        }
    }

    free(list);
    free(city_list);
    return shortest_path;
}
