#ifndef ITEM_AND_MISSION_H
#define ITEM_AND_MISSION_H

#include <ncurses.h>
#include <vector>
#include <utility>
#include <string>

// 기존 상수를 사용하기 위한 선언 (프로젝트 헤더에 맞춰 조절)
#define STAGE_NUM 4
#define MAP_ROW 40
#define MAP_COL 120
#define NUMBER_OF_ITEMS 3

// 아이템 및 게임 오브젝트 타입 정의
enum OBJ_TYPE {
    EMPTY = 0, WALL, IMMUNE_WALL, SNAKE_HEAD, SNAKE_BODY,
    GROWTH_ITEM, POISON_ITEM, SPEED_SLOW, GATE, PLUS_GATE, PLUS_EXIT
};

class ItemAndMission {
private:
    // 1. 아이템 관련 데이터
    std::vector<std::pair<int, int>> itemPos;
    char itemIndex[15] = {' ', '#', 'W', 'H', 'B', 'G', 'P', 'S', 'G', 'X', 'E'}; // 렌더링용 기호

    // 2. 스코어 및 미션 데이터 (기존 stat, statMission, chkMission)
    int stat[5];             // 0:Length, 1:Growth, 2:Poison, 3:Gate, 4:Slow
    int statMission[4];      // 각 항목별 목표치
    char chkMission[4];      // 미션 달성 여부 표시 (' ' 또는 'v')

public:
    ItemAndMission();
    ~ItemAndMission();

    // [1단계] 아이템 생성 알고리즘
    void appearItem(int** map, int level);
    void disappearItem(int** map);

    // [2단계] 아이템 획득 및 뱀과의 상호작용 (뱀 객체나 좌표 구조체를 가리키는 포인터 필요)
    // 여기서는 질문자님의 원본 구조를 유지하기 위해 구조체 포인터나 상태 변경용 참조를 받습니다.
    void eatItem(int itemType, int& snakeLength, int& timeoutMs);

    // [3단계] UI 및 데이터 반영
    void setMission(); // 초기화
    bool isMissionClear(); // 클리어 조건 검사
    void drawScoreAndMission(WINDOW* scoreWin, WINDOW* missionWin, int snakeMaxLength); // UI 갱신

    // Getter / Setter (Stage나 다른 클래스에서 데이터 접근용)
    int* getStat() { return stat; }
    void updateSnakeLengthStat(int length) { stat[0] = length; }
    void incrementGateStat() { stat[3]++; }
};

#endif