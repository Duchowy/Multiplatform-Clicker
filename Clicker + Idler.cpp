#include<iostream>
#include<cstdlib>
#include<stdlib.h>
#include<time.h>
#include<windows.h>
#include<cstdio>
#include<thread>
#include<fstream>
using namespace std;


int debug = 0;
int level;
int r;
int kliki = 0;
int a = 0;
int nagroda = 1;

int load()
{
        char decision;
        cout<<"Would you like to load the save file? y/n" << endl << "To save the game press 's' while in the game" << endl;
        do{
        cin >> decision;
        switch (decision)
        {
            case 'y':
                {
                ifstream savefile("clicker-savedata.sav");
                    if(savefile.fail())
                    {
                    cout << "No save file found" << endl; //no break, continues to the next operation which is 'n'
                    }
                    else
                    {
                        savefile >> level >> r >> a >> kliki >> debug;
                        savefile.close();
                        break;
                    }
                }

            case 'n': //when no save file is found
            do{ //select level
                cout<<"Select level: ";
                cin >> level;
                if((level<=0 || cin.fail())&& level != -1)
                {
                    cout<< "Error!" << endl;
                    cin.clear();
                    cin.sync();
                }
                if(level==-1)
                {
                    cout << "Debug mode turned on" << endl;
                    debug=1;
                }
                if(level>=1) r = 10+level;
            }while(level<=0);
        break;
            default:
                cout << "Error!"<<endl;
                cin.clear();
                cin.sync();
        }
        }while((decision != 'y') && (decision !='n'));
    cout << "Selected level: " << level << endl;
    if(debug == 1) cout << "r = " << r/10 << "." << r%10 << endl;
}


void naliczanie()
{
    system("CLS"); //erase everything on the console
    a += r;
    cout << a/10 << "." << a%10 << endl << "r = " << r/10 << "." << r%10 <<  endl << "Level " << level << endl;
    if(debug==1) cout << "Clicks from the beginning: " << kliki << endl;

    if (a >= nagroda*10000) //nabijacz r
    {
    r = r+rand()%5+1;
    cout << "Great job! New r - " << r/10 << "." << r%10 << "\a" << endl;
    nagroda++;
    Sleep(3000);
    }

    if ((kliki == (6000*level+r*100))    || (a > 60000*level+r)) //true progress
    {
    a = 0;
    level = level+1+rand()%2;
    r = 10+level;
    nagroda = 1;
    cout << "You got to a higher level " << level << endl;
    Sleep(3000);
    }



}

void clicker()
{
     bool mysz, klik = 0;
        while(true)
{
    if(HIBYTE(GetKeyState(VK_LBUTTON))) mysz=1;   //mouse click detect
    else mysz =0;
    if(mysz == 1) klik = 1;
    if(mysz == 0)
    {
    if(klik) // increment a,       end of switch mechanism
    {
    naliczanie();
    kliki++;
    klik = 0;
    Sleep(5);
    }
    }



}
}

void idler()
{
 while(true)
 {
 Sleep( (rand()%30+11)*100    );
 naliczanie();
 }

}

void savedata()
{
    bool save = 0, klik = 0;
    while(true)
    {
        if(GetAsyncKeyState('S')& 0x8000) save = 1;
        else save = 0;
        if(save == 1) klik = 1;
        if(save == 0)
        {
            if(klik)
            {
                ofstream savefile("clicker-savedata.sav");
                savefile << level << " " << r << " " << a << " " << kliki << " " << debug;

                system("CLS");
                cout<< "Saved data";
                savefile.close();
                klik = 0;
            }

        }


    }



}



int main()
{
      {
          HANDLE hInput;
          hInput = GetStdHandle(STD_INPUT_HANDLE);                                                      //change console mode
          DWORD prev_mode;
          GetConsoleMode(hInput, &prev_mode);
          SetConsoleMode(hInput, ENABLE_EXTENDED_FLAGS | (prev_mode & ~ENABLE_QUICK_EDIT_MODE));
      }

      srand(time(NULL));
      load();
      thread idle(idler);
      thread saveprogress(savedata);
      clicker();



}
