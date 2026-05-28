#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <ncurses.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

using namespace std;

class GameManager {
private:
    WINDOW* manual;
    WINDOW* description;
    WINDOW* scrollBar;

    int y, x;
    int menuLastFocus;
    string manualTitle, menuTitle, shorTitle;
    string menuTxt[3];
    string shorTxt[7];

    int sizeY, sizeX, startY, startX;
    int desSizeY, desSizeX, desStartY, desStartX;
    int txtLines, hidTxtLen, scrollBarLen;

public:
    GameManager();
    ~GameManager();

    void screenLock();
    string menu();
    void help();
};

#endif