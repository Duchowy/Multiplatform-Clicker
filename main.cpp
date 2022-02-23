#include "main.h"


void level_up(struct playerdata *,unsigned long long int*, bool*, ALLEGRO_EVENT_QUEUE*);
void idler(struct playerdata *,bool*,std::mt19937 *);
void start_screen(struct playerdata *, bool*, ALLEGRO_TIMER*,ALLEGRO_EVENT_QUEUE*);
void shop(struct playerdata *, bool*,std::mt19937 *, ALLEGRO_TIMER*, ALLEGRO_EVENT_QUEUE*,std::vector<std::thread> &);
void click(struct playerdata *);


//global variables
const int window_x = 320, window_y = 400;
const double fps = 30.0;

void init()
{
    al_init();
    al_init_image_addon();
    al_init_primitives_addon();
    al_install_keyboard();
    al_install_mouse();
}



int main()
{
    //INITIALIZATION
    init();

    //part responsible for functions of idler
    srand(time(NULL));
    std::random_device randomizer;
    std::mt19937 generator(randomizer());
    std::vector<std::thread> idlers;

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
    struct playerdata player;
    //variables
    player.points = 0;
    player.effort = 0;
    player.level = 1;
    player.r = 10;
    player.r_idle = 10;
    player.idlers = 0;
    player.debug = 0;

    bool savestate = 0;  //savestate switch
    int saveTrigger = 0; //as savestate is triggered, it will persist for 3 seconds; this variable is to count that time

    ALLEGRO_BITMAP* menu_bmp = al_load_bitmap("img/main_menu.bmp");
    if (!menu_bmp) return 0;

    al_start_timer(timer);

    start_screen(&player,&end, timer,queue); //trigger the start sequence
	for(int i = 0; i<player.idlers; i++)
	{
		idlers.push_back(std::thread(idler,&player,&end,&generator));
	}


    unsigned long long int cap = player.level*player.level*1000 + 6000;

    

    while (!end)
    {
            std::string display_points_str = std::to_string(player.points/10) + "," + std::to_string(player.points % 10); //crude conversion to make points printable on screen

            al_wait_for_event(queue, &event);
            switch (event.type)
            {
            case ALLEGRO_EVENT_TIMER:
                if (event.timer.source == timer)
                {
                    redraw = true; //rewrite to accomodate multithread
                    if (player.points >= cap) level_up(&player, &cap, &end, queue);
                }
                break;
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                end = 1;
                break;
            case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
                click(&player);
                if(player.points >= cap) level_up(&player, &cap, &end, queue);
                break;
            case ALLEGRO_EVENT_KEY_DOWN:
            {
                switch (event.keyboard.keycode)
                {
                case ALLEGRO_KEY_S:
                {
                    std::ofstream savefile("clicker-savedata.sav");
                    savefile << player.level << " " << player.r << " " << player.r_idle <<" " << player.points << " " << player.effort << " " << player.debug;
                    savefile.close();
                    savestate = 1;
                    saveTrigger = 0;
                    break;
                }
                case ALLEGRO_KEY_ESCAPE:
                    savestate = 0;
                    shop(&player, &end,&generator, timer, queue,std::ref(idlers));
                    break;
                }
            }
            break;
            }


        if (redraw && al_is_event_queue_empty(queue))
        {
            //al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_bitmap(menu_bmp, 0, 0, 0);
            if (player.debug == true) al_draw_text(font, al_map_rgb(255, 255, 255), window_x / 2, 30, ALLEGRO_ALIGN_CENTRE, "Debug mode on");
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
    al_destroy_bitmap(menu_bmp);
	for(auto &a : idlers)
	{
		a.join();
	}

    return 0;
}


void idler(struct playerdata * player, bool*kill,std::mt19937 * generator)
{
    std::uniform_int_distribution<unsigned short> range(2,7);
    time_t counter = time(NULL);
	unsigned short delay = range(*generator);
    while(!(*kill))
        {
            if(time(NULL)-counter >= delay)
            {
                delay = range(*generator);
                counter = time(NULL);
                player->points += player->r_idle;
            }
        }

}

void click(struct playerdata * player)
{
    bool flag = 0;
    player->effort += 1;
    static double tab[20];
    static int tracker = 0; //tracks the position where new time is put in
    static double old_Time = 0;
    static int entry_counter = 0;

    double new_Time = al_get_time();
    double current_Delay = new_Time - old_Time;

    //100x^2 +600

    for (int i = 0; i < entry_counter; i++)
    {
        if (current_Delay <= tab[i] + 0.1 && current_Delay >= tab[i] - 0.1)
        {
            flag = true;
            break;
        }
    }
    //exit sequence
    if(flag == 0) player->points += player->r*2.5;
    else player->points += player->r;

    //printf("%d\n", flag);

    tab[tracker] = current_Delay;
    tracker++;
    old_Time = new_Time;
    if (tracker == 20) tracker = 0;
    if (entry_counter < 20) entry_counter += 1;
}

void start_screen(struct playerdata * player, bool* kill, ALLEGRO_TIMER* timer,ALLEGRO_EVENT_QUEUE* queue)
{
    bool end = 0;
    bool redraw = 1;

    bool loadfail = 0;
    int loadTrigger = 0;
    
    ALLEGRO_FONT* font = al_create_builtin_font();
    ALLEGRO_EVENT event;

    while (!end)
    {
        std::string display_points_str = std::to_string(player->level); //crude conversion to make points printable on screen

        al_wait_for_event(queue, &event);
        switch (event.type)
        {
        case ALLEGRO_EVENT_TIMER:
            if (event.timer.source == timer)
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
                player->r += player->level-1;
                player->r_idle = player->r;
                break;
            }
            case ALLEGRO_KEY_UP:
            {
                player->level += 1;
                break;
            }
            case ALLEGRO_KEY_DOWN:
            {
                if (player->level > 1) player->level -= 1;
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
                        savefile >> player->level >> player->r >> player->r_idle >> player->points >> player->effort >> player->debug;
                        savefile.close();
                        end = 1;
                    }
                }
                break; //break the L key event from key event switch
            }
            /*
            case ALLEGRO_KEY_F9:
            {
                if (*debug == 0) *debug = true;
                else *debug = false;
                break;
            }
            */


            }
            break; //event key down break
        }



        if (redraw && al_is_event_queue_empty(queue))
        {
            al_clear_to_color(al_map_rgb(0, 0, 0));


            al_draw_text(font, al_map_rgb(255, 255, 255), window_x / 2, window_y / 2, ALLEGRO_ALIGN_CENTRE, "Select level");
            al_draw_text(font, al_map_rgb(255, 255, 255), window_x / 2, window_y / 2 - 10, ALLEGRO_ALIGN_CENTRE, display_points_str.c_str());
            al_draw_text(font, al_map_rgb(255, 255, 255), window_x / 2, window_y / 2 + 10, ALLEGRO_ALIGN_CENTRE, "Or press 'L' to load the game");
            if (loadfail)
            {
                loadTrigger++;
                al_draw_text(font, al_map_rgb(255, 255, 255), window_x / 2, window_y / 2 + 20, ALLEGRO_ALIGN_CENTRE, "Failed to load the game!");
                if (loadTrigger == (int)(3* fps))
                {
                    loadTrigger = 0;
                    loadfail = 0;
                }
            }
            al_flip_display();
            redraw = false;
        }

    }
    al_destroy_font(font);
}

void shop(struct playerdata * player, bool* kill,std::mt19937 * generator, ALLEGRO_TIMER* timer, ALLEGRO_EVENT_QUEUE* queue,std::vector<std::thread> & idlers)
{
    bool end = false;
    bool redraw = 1;

    bool NotEnough = 0;
    short int NotEnough_Trigger = 0;

    short int select = 0;

    ALLEGRO_FONT* font = al_create_builtin_font();
    ALLEGRO_BITMAP* sel1_bmp = al_load_bitmap("img/sel1.bmp");
    if (!sel1_bmp) return;

    ALLEGRO_EVENT event;

    while (!end && !(*kill))
    {
        al_wait_for_event(queue, &event);
            switch (event.type)
            {
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                end = 1;
                *kill = 1;
                break;
            case ALLEGRO_EVENT_TIMER:
                if (event.timer.source == timer)
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
                            if (player->effort < 300) NotEnough = 1;
                            else
                            {
                                NotEnough = 0;
                                player->effort -= 300;
                                player->r += 1;
                            }
                            break;
                        case 1: //upgrade the r_idle
                            if (player->effort < 500) NotEnough = 1;
                            else
                            {
                                NotEnough = 0;
                                player->effort -= 500;
                                player->r_idle += 1;
                            }
                            break;
                        case 2:
                            if(player->effort < 800) NotEnough = 1;
                            else
                            {
                                NotEnough = 0;
                                if(player->idlers < 5)
                                {
                                    player->effort -= 800;
                                    player->idlers +=1;
                                    idlers.push_back(std::thread(idler,player,kill,generator));
                                }
                            }
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
            std::string price;
            std::string geld = std::to_string(player->effort);
            std::string r_power = std::to_string(player->r/10) + "," + std::to_string(player->r%10);
            std::string r_idle_power = std::to_string(player->r_idle/10) + "," + std::to_string(player->r_idle%10);

            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_text(font, al_map_rgb(255, 255, 255), window_x / 2, window_y / 2-40, ALLEGRO_ALIGN_CENTRE, "Choose an upgrade:");
            
            //draw option pictures
            al_draw_bitmap(sel1_bmp, window_x / 2 - 100+3, window_y / 2 - 20+3, 0);

            //draw selection rectangle
            al_draw_rectangle(window_x / 2 - 100 + 75*select, window_y / 2-20, window_x / 2 - 50 + 75 * select, window_y / 2 + 30, al_map_rgb(255, 255, 255),3); //draw selection rectangle
            //section for printing statistics
            al_draw_text(font, al_map_rgb(255, 255, 255), 1, 1, 0,("Money: " + geld).c_str());
            al_draw_text(font, al_map_rgb(255, 255, 255), 1, 11, 0, ("Click power: " + r_power).c_str());
            al_draw_text(font, al_map_rgb(255, 255, 255), 1, 21, 0, ("Idle power: " + r_idle_power).c_str());
            //draw the description
            switch (select)
            {
            case 0:
                desc = "Upgrade the clicker by 0.1";
                price = "Price: 300";
                break;
            case 1:
                desc = "Upgrade the idler by 0.1";
                price = "Price: 500";
                break;
            case 2:
                desc = "Buy +1 idler. Owned " + std::to_string(player->idlers) + "/5";
                price = "Price: 800";
                break;
            }
            al_draw_text(font, al_map_rgb(255, 255, 255), window_x / 2, window_y / 2 + 50, ALLEGRO_ALIGN_CENTRE, price.c_str());
            al_draw_text(font, al_map_rgb(255, 255, 255), window_x/2, window_y/2 + 40, ALLEGRO_ALIGN_CENTRE, desc.c_str());
            
            if (NotEnough) //handle the not enough money error
            {
                NotEnough_Trigger++;
                al_draw_text(font, al_map_rgb(255, 255, 255), window_x / 2, window_y - 25, ALLEGRO_ALIGN_CENTRE, "Not enough money!");
                if (NotEnough_Trigger == (int)(fps * 3))
                {
                    NotEnough_Trigger = 0;
                    NotEnough = false;
                }
            }

            al_flip_display();
            redraw = false;
        }




    }
    al_destroy_bitmap(sel1_bmp);
    al_destroy_font(font);
}

void level_up(struct playerdata * player,unsigned long long int*cap, bool* kill, ALLEGRO_EVENT_QUEUE* queue)
{
    bool end = false;
    bool redraw = true;

    ALLEGRO_BITMAP* level_up_bmp = al_load_bitmap("img/level_up.bmp");
    if (!level_up_bmp) return;

    ALLEGRO_FONT* font = al_create_builtin_font();
    ALLEGRO_EVENT event;
    ALLEGRO_TIMER* timer = al_create_timer(1.0);
    al_start_timer(timer);
    al_register_event_source(queue, al_get_timer_event_source(timer));
    int state = 0;

    player->level += 1;
    player->r += 1 + rand() % 2;
    player->r_idle += 1 + rand() % 2;
    *cap += player->level * player->level * 1000 + 6000;

    std::string level_str = "You were promoted to rank " + std::to_string(player->level);
    std::string additive_str = "New click power: " + std::to_string(player->r / 10) + "," + std::to_string(player->r % 10);
    std::string idle_additive_str = "New idle power: " + std::to_string(player->r_idle / 10) + "," + std::to_string(player->r_idle % 10);


    while (!end)
    {
        al_wait_for_event(queue, &event);
        switch (event.type)
        {
             case ALLEGRO_EVENT_DISPLAY_CLOSE:
              {
                  end = true;
                  *kill = true;
                  break;
               }
              case ALLEGRO_EVENT_TIMER:
              if (event.timer.source == timer)
              {
                  if (state < 7)state++;
                  else end = true;
                  redraw = true;
              }
              break;
        }
        //printf("%d\n",state);

        if (redraw && al_is_event_queue_empty(queue))
        {
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_bitmap(level_up_bmp, window_x/2- al_get_bitmap_width(level_up_bmp)/2, 20, 0);
            if (state > 1) al_draw_text(font, al_map_rgb(255, 255, 255),window_x/2, window_y/2-10, ALLEGRO_ALIGN_CENTRE,level_str.c_str());
                
            if (state > 3)
            {
                al_draw_text(font, al_map_rgb(255, 255, 255), window_x / 2, window_y/2+10, ALLEGRO_ALIGN_CENTRE, additive_str.c_str());
                al_draw_text(font, al_map_rgb(255, 255, 255), window_x / 2, window_y/2+20, ALLEGRO_ALIGN_CENTRE, idle_additive_str.c_str());
            }


            al_flip_display();
            redraw = false;
        }
    }
    al_destroy_timer(timer);
    al_destroy_font(font);
    al_destroy_bitmap(level_up_bmp);
}