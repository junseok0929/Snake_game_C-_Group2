#ifndef GATEMANAGER_H
#define GATEMANAGER_H

#pragma once

#include <vector>
#include <utility>
#include <cstdlib>
#include "Something.h"
#include "ItemAndMission.h" // itemAndMission 조작을 위해 필요

const int STAGE_NUM = 4,
MAP_ROW = 30,
MAP_COL = 50,
ROW_END = MAP_ROW - 1,
COL_END = MAP_COL - 1;

// 게임 공통 상수가 정의된 헤더를 포함해야 합니다 (예: Stage.h 또는 Constants.h)
// #include "Constants.h" 

class GateManager {
private:
    Something* gate1;
    Something* gate2;
    std::vector<std::pair<int, int>> gatePos;

    int findRoot(Something* gate, int currentDir, int** map);

public:
    GateManager();
    ~GateManager();

    void appearGate(int** map, int level);
    void disappearGate(int** map);
    // 상태 변경이 필요한 변수들은 참조자(&)로 넘겨받습니다.
    void enterGate(Something* head, int& dir, int** map, bool& chkEnter, ItemAndMission& itemAndMission);
};

#endif