#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <stdio.h>
#include <thread>
#include <string>
#include <iostream>

int main()
{
    al_init();
    al_install_keyboard();
    al_install_mouse();

    int window_x = 320, window_y = 200;

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 6.0);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    ALLEGRO_DISPLAY* disp = al_create_display(window_x, window_y);
    ALLEGRO_FONT* font = al_create_builtin_font();

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    bool redraw = true;
    ALLEGRO_EVENT event;

    //variables
    int points = 0;





    al_start_timer(timer);
    while (1)
    {
            std::string display_points_str = std::to_string(points); //crude conversion to make it printable on screen
            const char* display_points = display_points_str.c_str();


            al_wait_for_event(queue, &event);
            if (event.type == ALLEGRO_EVENT_TIMER) redraw = true;
            else
                if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) break;
                else
            if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
                {
                    al_clear_to_color(al_map_rgb(0, 0, 0));
                    al_draw_text(font, al_map_rgb(255, 255, 255), window_x / 2, window_y / 2, 0, display_points);
                    al_flip_display();
                    redraw = true;
                    //printf("%s\n",display_points);
                    points++;

                }

        if (redraw && al_is_event_queue_empty(queue))
        {
            //al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_text(font, al_map_rgb(255, 255, 255), window_x / 2 - 90, window_y / 2 + 15, 0, "Press S to save the game");
            al_flip_display();
            redraw = false;
        }
    }





    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    return 0;
}