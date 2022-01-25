#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <stdio.h>
//#include <thread>
//#include <chrono>
#include <fstream>
#include <string>
#include <iostream>
#include <time.h>

void idler(int*, int*,int*);
void start_screen(int*, int*,int*, int*, bool*, bool*, ALLEGRO_TIMER*, ALLEGRO_FONT*,int*,int*,double*,ALLEGRO_EVENT_QUEUE*);

int main()
{
    //INITIALIZATION
    srand(time(NULL));
    al_init();
    al_install_keyboard();
    al_install_mouse();

    int window_x = 320, window_y = 200;
    double fps = 30.0;

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / fps);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    ALLEGRO_DISPLAY* disp = al_create_display(window_x, window_y);
    ALLEGRO_FONT* font = al_create_builtin_font();

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    bool end = false;
    bool redraw = true;
    ALLEGRO_EVENT event;
    //END OF INITIALIZATION

    //variables
    int points = 0;
    int r = 10;
    int level = 1;
    int effort = 0;
    bool debug = 0;

    bool savestate = 0;
    int saveTrigger = 0;

    int delay_counter = 0; //used to count the delay
    int delay_randomised = rand()%3+1; //used for random delay before next point is added
    al_start_timer(timer);

    start_screen(&points, &r, &effort, &level, &debug,&end, timer, font, &window_x, &window_y,&fps,queue);


    while (end == 0)
    {
            std::string display_points_str = std::to_string(points/10) + "," + std::to_string(points % 10); //crude conversion to make points printable on screen
            const char* display_points = display_points_str.c_str();


            al_wait_for_event(queue, &event);
            switch (event.type)
            {
            case ALLEGRO_EVENT_TIMER:
                if (event.timer.source == timer)
                {
                    delay_counter += 1;
                    redraw = true;
                    if (delay_counter == (int)fps * delay_randomised) //idler mechanism
                    {
                        idler(&points, &r, &delay_randomised);
                        delay_counter = 0;
                    }
                }
                break;
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                end = 1;
                break;
            case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
                redraw = true;
                effort++;
                points+=r;
                break;
            case ALLEGRO_EVENT_KEY_DOWN:
            {
                switch (event.keyboard.keycode)
                {
                case ALLEGRO_KEY_S:
                    std::ofstream savefile("clicker-savedata.sav");
                    savefile << level << " " << r << " " << points << " " << effort << " " << debug;
                    savefile.close();
                    savestate = 1;
                    saveTrigger = 0;
                    redraw = true;
                    break;
                }
            }
            break;
            }


        if (redraw && al_is_event_queue_empty(queue))
        {
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_text(font, al_map_rgb(255, 255, 255), window_x / 2, window_y / 2, 0, display_points);
            if(savestate == 0)
            al_draw_text(font, al_map_rgb(255, 255, 255), window_x / 2 - 90, window_y / 2 + 15, 0, "Press S to save the game");
            else
            {
                saveTrigger++;
                al_draw_text(font, al_map_rgb(255, 255, 255), window_x / 2 - 90, window_y / 2 + 15, 0, "Game saved successfully!");
                if (saveTrigger == 3 * (int)fps)
                {
                    saveTrigger = 0;
                    savestate = 0;
                }
            }
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


void idler(int*variable, int*additive, int*random)
{
        *variable += *additive;
        *random = rand() % 3 + 1;
}

void start_screen(int* variable, int* additive,int * effort,int * level,bool*debug, bool* kill, ALLEGRO_TIMER* timer, ALLEGRO_FONT* font, int*window_x, int*window_y,double*fps,ALLEGRO_EVENT_QUEUE* queue)
{
    //al_register_event_source(queue, al_get_keyboard_event_source());
    bool end = 0;
    bool redraw = 1;

    bool loadfail = 0;
    int loadTrigger = 0;
    
    ALLEGRO_EVENT event;

    do
    {
        std::string display_points_str = std::to_string(*level); //crude conversion to make points printable on screen
        const char* display_points = display_points_str.c_str();

        al_wait_for_event(queue, &event);
        switch (event.type)
        {
        case ALLEGRO_EVENT_TIMER:
            redraw = 1;
            break;
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            end = 1;
            *kill = 1;
            break;
        case ALLEGRO_EVENT_KEY_DOWN:
            switch (event.keyboard.keycode)
            {
            case ALLEGRO_KEY_ENTER:
            {
                end = 1;
                *additive += *level;
                break;
            }
            case ALLEGRO_KEY_UP:
            {
                *level += 1;
                redraw = 1;
                break;
            }
            case ALLEGRO_KEY_DOWN:
            {
                if (*level > 1)
                {
                    *level -= 1;
                    redraw = 1;
                }
                break;
            }
            case ALLEGRO_KEY_L:
            {
                std::ifstream savefile("clicker-savedata.sav");
                {
                    if (savefile.fail())
                    {
                        loadfail = 1;
                        loadTrigger = 0;
                    }
                    else
                    {
                        savefile >> *level >> *additive >> *variable >> *effort >> *debug;
                        savefile.close();
                        end = 1;
                    }
                }
                break; //break the L key event from key event switch
            }

            }
            break;
        }



        if (redraw && al_is_event_queue_empty(queue))
        {
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_text(font, al_map_rgb(255, 255, 255), *window_x / 2-40, *window_y / 2, 0, "Select level");
            al_draw_text(font, al_map_rgb(255, 255, 255), *window_x / 2, *window_y / 2 - 10, 0, display_points);
            al_draw_text(font, al_map_rgb(255, 255, 255), *window_x / 2 - 120, *window_y / 2 + 10, 0, "Or press 'L' to load the game");
            if (loadfail)
            {
                loadTrigger++;
                al_draw_text(font, al_map_rgb(255, 255, 255), *window_x / 2 - 100, *window_y / 2 + 20, 0, "Failed to load the game!");
                if (loadTrigger == (int)(3* *fps))
                {
                    loadTrigger = 0;
                    loadfail = 0;
                }
            }
            al_flip_display();
            redraw = false;
        }

    }while (end == 0);
}