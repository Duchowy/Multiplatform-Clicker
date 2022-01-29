#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
//#include <thread>
//#include <chrono>
#include <fstream>
#include <string>
#include <iostream>
#include <time.h>

void idler(int*, int*,int*);
void start_screen(int*, int*,int*,int*, int*, bool*, bool*, ALLEGRO_TIMER*, ALLEGRO_FONT*,int*,int*,double*,ALLEGRO_EVENT_QUEUE*);
void shop(int*, int*,int*, int*, int*, bool*, bool*, ALLEGRO_TIMER*, ALLEGRO_FONT*, int*, int*, double*, ALLEGRO_EVENT_QUEUE*);

int main()
{
    //INITIALIZATION
    srand(time(NULL));
    al_init();
    al_init_primitives_addon();
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
    int r_idle = 10;
    bool debug = 0;

    bool savestate = 0;  //savestate switch
    int saveTrigger = 0; //as savestate is triggered, it will persist for 3 seconds; this variable is to count that time

    int delay_counter = 0; //used to count the delay
    int delay_randomised = rand()%3+1; //used for random delay before next point is added
    al_start_timer(timer);

    start_screen(&points, &r,&r_idle, &effort, &level, &debug,&end, timer, font, &window_x, &window_y,&fps,queue); //trigger the start sequence


    while (!end)
    {
            std::string display_points_str = std::to_string(points/10) + "," + std::to_string(points % 10); //crude conversion to make points printable on screen


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
                        idler(&points, &r_idle, &delay_randomised);
                        delay_counter = 0;
                    }
                }
                break;
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                end = 1;
                break;
            case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
                effort++;
                points+=r;
                break;
            case ALLEGRO_EVENT_KEY_DOWN:
            {
                switch (event.keyboard.keycode)
                {
                case ALLEGRO_KEY_S:
                {
                    std::ofstream savefile("clicker-savedata.sav");
                    savefile << level << " " << r << " " << r_idle <<" " << points << " " << effort << " " << debug;
                    savefile.close();
                    savestate = 1;
                    saveTrigger = 0;
                    break;
                }
                case ALLEGRO_KEY_ESCAPE:
                    savestate = 0;
                    shop(&points, &r,&r_idle, &effort, &level, &debug, &end, timer, font, &window_x, &window_y, &fps, queue);
                    break;
                }
            }
            break;
            }


        if (redraw && al_is_event_queue_empty(queue))
        {
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_text(font, al_map_rgb(255, 255, 255), window_x / 2, window_y / 2, ALLEGRO_ALIGN_CENTRE, display_points_str.c_str());

            //switch mechanism for printing either "press S" or "game saved"
            if(savestate == 0)
            al_draw_text(font, al_map_rgb(255, 255, 255), window_x / 2, window_y - 25, ALLEGRO_ALIGN_CENTRE, "Press S to save the game");
            else
            {
                saveTrigger++;
                al_draw_text(font, al_map_rgb(255, 255, 255), window_x / 2, window_y - 25, ALLEGRO_ALIGN_CENTRE, "Game saved successfully!");
                if (saveTrigger == 3 * (int)fps) //release the switch
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


void idler(int*variable, int*idle_additive, int*random)
{
        *variable += *idle_additive;
        *random = rand() % 3 + 1;
}

void start_screen(int* variable, int* additive, int* idle_additive,int * effort,int * level,bool*debug, bool* kill, ALLEGRO_TIMER* timer, ALLEGRO_FONT* font, int*window_x, int*window_y,double*fps,ALLEGRO_EVENT_QUEUE* queue)
{
    bool end = 0;
    bool redraw = 1;

    bool loadfail = 0;
    int loadTrigger = 0;
    
    ALLEGRO_EVENT event;

    do
    {
        std::string display_points_str = std::to_string(*level); //crude conversion to make points printable on screen

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
                *additive += *level-1;
                *idle_additive = *additive;
                break;
            }
            case ALLEGRO_KEY_UP:
            {
                *level += 1;
                break;
            }
            case ALLEGRO_KEY_DOWN:
            {
                if (*level > 1) *level -= 1;
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
                        savefile >> *level >> *additive >> *idle_additive >> *variable >> *effort >> *debug;
                        savefile.close();
                        end = 1;
                    }
                }
                break; //break the L key event from key event switch
            }

            }
            break; //event key down break
        }



        if (redraw && al_is_event_queue_empty(queue))
        {
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_text(font, al_map_rgb(255, 255, 255), *window_x / 2, *window_y / 2, ALLEGRO_ALIGN_CENTRE, "Select level");
            al_draw_text(font, al_map_rgb(255, 255, 255), *window_x / 2, *window_y / 2 - 10, ALLEGRO_ALIGN_CENTRE, display_points_str.c_str());
            al_draw_text(font, al_map_rgb(255, 255, 255), *window_x / 2 , *window_y / 2 + 10, ALLEGRO_ALIGN_CENTRE, "Or press 'L' to load the game");
            if (loadfail)
            {
                loadTrigger++;
                al_draw_text(font, al_map_rgb(255, 255, 255), *window_x / 2, *window_y / 2 + 20, ALLEGRO_ALIGN_CENTRE, "Failed to load the game!");
                if (loadTrigger == (int)(3* *fps))
                {
                    loadTrigger = 0;
                    loadfail = 0;
                }
            }
            al_flip_display();
            redraw = false;
        }

    }while (!end);
}

void shop(int* variable, int* additive,int*idle_additive, int* effort, int* level, bool* debug, bool* kill, ALLEGRO_TIMER* timer, ALLEGRO_FONT* font, int* window_x, int* window_y, double* fps, ALLEGRO_EVENT_QUEUE* queue)
{
    bool end = false;
    bool redraw = 1;

    bool NotEnough = 0;
    short int NotEnough_Trigger = 0;

    short int select = 0;

    ALLEGRO_EVENT event;

    while (!end)
    {
        al_wait_for_event(queue, &event);
            switch (event.type)
            {
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                end = 1;
                *kill = 1;
                break;
            case ALLEGRO_EVENT_TIMER:
                redraw = true;
                break;
            case ALLEGRO_EVENT_KEY_DOWN:
            {
                switch (event.keyboard.keycode)
                {
                    case ALLEGRO_KEY_ESCAPE:
                    {
                        end = true;
                        break;
                    }
                    case ALLEGRO_KEY_RIGHT:
                        if (select < 2) select += 1;
                        break;
                    case ALLEGRO_KEY_LEFT:
                        if (select > 0) select -= 1;
                        break;
                    case ALLEGRO_KEY_ENTER:
                        switch (select)
                        {
                        case 0: //upgrade the r
                            if (*effort < 300) NotEnough = 1;
                            else
                            {
                                *effort -= 300;
                                *additive += 1;
                            }
                            break;
                        case 1: //upgrade the r_idle
                            if (*effort < 500) NotEnough = 1;
                            else
                            {
                                *effort -= 500;
                                *idle_additive += 1;
                            }
                            break;
                        case 2:
                            break;
                        
                        }
                        break; //break the enter statement


                }
                break; //break the key down statement
            }




            }

        if (redraw && al_is_event_queue_empty(queue))
        {
            std::string desc;
            std::string geld = std::to_string(*effort);
            std::string r_power = std::to_string(*additive/10) + "," + std::to_string(*additive%10);
            std::string r_idle_power = std::to_string(*idle_additive/10) + "," + std::to_string(*idle_additive%10);

            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_text(font, al_map_rgb(255, 255, 255), *window_x / 2, *window_y / 2-40, ALLEGRO_ALIGN_CENTRE, "Choose an upgrade:");
            al_draw_rectangle(*window_x / 2 - 100 + 75*select, *window_y / 2-20, *window_x / 2 - 50 + 75 * select, *window_y / 2 + 30, al_map_rgb(255, 255, 255),3); //draw selection rectangle
            //section for printing statistics
            al_draw_text(font, al_map_rgb(255, 255, 255), 1, 1, 0,("Money: " + geld).c_str());
            al_draw_text(font, al_map_rgb(255, 255, 255), 1, 11, 0, ("Click power: " + r_power).c_str());
            al_draw_text(font, al_map_rgb(255, 255, 255), 1, 21, 0, ("Idle power: " + r_idle_power).c_str());
            //draw the description
            switch (select)
            {
            case 0:
                desc = "Upgrade the clicker by 0.1";
                break;
            case 1:
                desc = "Upgrade the idler by 0.1";
                break;
            case 2:
                desc = "Yet to be implemented!";
                break;
            }
            al_draw_text(font, al_map_rgb(255, 255, 255), *window_x/2, *window_y/2 + 40, ALLEGRO_ALIGN_CENTRE, desc.c_str());
            
            al_flip_display();
            redraw = false;
        }




    }
}