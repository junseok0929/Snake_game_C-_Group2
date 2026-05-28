//Stage.h
#pragma once

#ifndef __STAGE__
#define __STAGE__
#include <ncurses.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <vector>

#include "Something.h"
#include "ItemAndMission.h"
#include "GateManager.h"


#define UP KEY_UP
#define DOWN KEY_DOWN
#define RIGHT KEY_RIGHT
#define LEFT KEY_LEFT
#define ENTER 10
#define ESC 27
#define PAUSE 112
#define RESUME 114


#define EMPTY 0
#define WALL 1
#define IMMUNE_WALL 2
#define SNAKE_HEAD 3
#define SNAKE_BODY 4
#define GROWTH_ITEM 5
#define POISON_ITEM 6
#define GATE 8
#define SPEED_SLOW 7
#define NUMBER_OF_ITEMS 3
#define PLUS_EXIT 13

using namespace std;

class Stage
{
public:
    Stage();
    ~Stage();

    void play();

    int getStageNum() const { return STAGE_NUM; }
    int getMapRow() const { return MAP_ROW; }
    int getMapCol() const { return MAP_COL; }

    void setMap();
    void copyMap(int nStage);
    void drawMap();

    void appearItem();
    void disappearItem();

    void makeSnake();
    void moveSnake();
    void eatItem(int item);

    void setMission();
    bool isMissionClear();
    bool checkGameOver() { return finish; }
    void gameOver();
    void alert(int posY, int posX, const string msg, bool stopFlag);

private:
    WINDOW* game, * score, * mission, * info;

    const int SNAKE_MAX_LENGTH = 10;
    Something* Snail;

    const int STAGE_NUM = 4,
        MAP_ROW = 30,
        MAP_COL = 50,
        ROW_END = MAP_ROW - 1,
        COL_END = MAP_COL - 1;
    const string itemIndex = "  X0O+-                ";

    int*** stage, ** map;

    int dir, y, x, level, speed, tcount, timeoutMs, msTime;
    const int speedMs[5] = { 500,250,200,100,70 };
    const int msDiv[5] = { 2,4,5,10,20 };

    vector<pair<int, int>> itemPos;
    bool chkEnter, finish;
    ItemAndMission itemMission;
    GateManager gateManager;
};
#endif