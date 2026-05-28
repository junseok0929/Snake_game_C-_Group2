// Stage.h

#ifndef __STAGE__
#define __STAGE__
#include <ncurses.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <fstream>

#include "Something.h"
#include "ItemAndMission.h" // ★ 1. 새로 만든 헤더 파일 추가


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
#define PLUS_GATE 12
#define PLUS_EXIT 13

using namespace std;

class Stage
{
public:
    
    Stage();
    ~Stage();

    
    void screenLock();

    
    string menu();
    void play();
    void help();

    
    int getStageNum() const { return STAGE_NUM; }
    int getMapRow() const { return MAP_ROW; }
    int getMapCol() const { return MAP_COL; }

    
    void setMap();
    void copyMap(int nStage);
    void drawMap();

    // ★ 2. 아이템 관련 함수 선언들 전부 삭제됨 (ItemAndMission으로 이동)
    void appearGate();
    void appearPlusGate();
    void disappearGate();
    void disappearPlusGate();

    
    void makeSnake();
    void moveSnake();
    void enterGate(Something *head);
    void enterPlusGate(Something *head);
    int findRoot(Something *gate);
    // ★ 3. eatItem(int item) 삭제됨 (ItemAndMission으로 이동)

    
    // ★ 4. setMission(), isMissionClear() 선언 삭제됨 (ItemAndMission으로 이동)
    bool checkGameOver() { return finish; }
    void gameOver();
    void alert(int posY, int posX, const string msg, bool stopFlag);

private:
    
    WINDOW *game, *score, *mission, *info;
    WINDOW *manual, *description, *scrollBar;

    // ★ 5. 본인 파트 데이터 멤버(stat) 삭제 및 새 매니저 객체 생성
    ItemAndMission itemManager; 
    int stat[1]; // Stage 내부에서 미세하게 쓰는 용도로 최소화하거나 지워도 무방 (구조 유지를 위해 보존 가능)

    // ★ 6. 미션 관련 변수들 전부 삭제됨 (ItemAndMission으로 이동)
    // char chkMission[4]; 
    // int statMission[4]; 

    
    const int SNAKE_MAX_LENGTH = 10;
    Something *Snail;
    Something *gate1;
    Something *gate2;
    Something *plusGate1;
    Something *plusGate2;

    
    const int STAGE_NUM = 4,
              MAP_ROW = 30,
              MAP_COL = 50,
              ROW_END = MAP_ROW - 1,
              COL_END = MAP_COL - 1;
    const string itemIndex = "  X0O+-                 ";

    
    int ***stage, **map;

    
    int dir, y, x,
        menuLastFocus, optLastFocus,
        level, speed, tcount,
        timeoutMs, msTime;
    const int speedMs[5] = {500,250,200,100,70} ;
    const int msDiv[5] = {2,4,5,10,20}; 

    
    string manualTitle, menuTitle, shorTitle;
    string menuTxt[4], shorTxt[7];
    int sizeY, sizeX,
        startY, startX;
    int desSizeY, desSizeX,
        desStartY, desStartX;
    int txtLines, hidTxtLen, scrollBarLen;

    vector<string> manualTxt;

    // ★ 7. itemPos 벡터 삭제됨 (ItemAndMission으로 이동)
    vector<pair<int, int>> gatePos, plusGatePos;
    bool chkEnter, chkPlusEnter, finish;
};
#endif