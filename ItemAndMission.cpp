#include "ItemAndMission.h"
#include <cstdlib>
#include <ctime>
#include <cstring>

ItemAndMission::ItemAndMission() {
    setMission();
}

ItemAndMission::~ItemAndMission() {}

// ==========================================
// [1단계] 아이템 생성 및 소멸 알고리즘
// ==========================================
void ItemAndMission::appearItem(int** map, int level) {
    int appearNum = rand() % 3 + 1; // 1~3개 무작위 생성
    for (int i = 0; i < appearNum; i++) {
        int itemType = rand() % NUMBER_OF_ITEMS + 5; // GROWTH, POISON, SPEED_SLOW 중 하나
        
        while (1) {
            int rY = rand() % (MAP_ROW - 2) + 1;
            int rX = rand() % (MAP_COL - 2) + 1;
            
            // 빈 공간(EMPTY)이면서 주변에 게이트가 없는 안전한 위치 확보
            if (map[rY][rX] == EMPTY &&
                map[rY][rX - 1] != GATE && map[rY][rX + 1] != GATE &&
                map[rY + 1][rX] != GATE && map[rY - 1][rX] != GATE &&
                map[rY][rX - 1] != PLUS_GATE && map[rY][rX + 1] != PLUS_GATE &&
                map[rY + 1][rX] != PLUS_GATE && map[rY - 1][rX] != PLUS_GATE) 
            {
                map[rY][rX] = itemType;
                itemPos.push_back(std::make_pair(rY, rX));
                break;
            }
        }
    }
}

void ItemAndMission::disappearItem(int** map) {
    for (auto item : itemPos) {
        if (map[item.first][item.second] == GROWTH_ITEM || 
            map[item.first][item.second] == POISON_ITEM || 
            map[item.first][item.second] == SPEED_SLOW) {
            map[item.first][item.second] = EMPTY;
        }
    }
    itemPos.clear();
}

// ==========================================
// [2단계] 아이템 상호작용 처리 방식
// ==========================================
void ItemAndMission::eatItem(int item, int& snakeLength, int& timeoutMs) {
    if (item == GROWTH_ITEM) {
        if (stat[0] == 10) return; // 최대 길이 제한
        
        // ※ 실제 뱀 몸통 늘리는 로직은 Snake 객체 혹은 Stage 메인 루프에서 처리하도록 호출 유도
        // 여기서는 질문자님 파트의 데이터(stat) 변경에 집중합니다.
        snakeLength++; 
        stat[0]++; // 현재 스네이크 길이 증가
        stat[1]++; // 먹은 Growth 아이템 개수 증가
    }
    else if (item == POISON_ITEM) {
        snakeLength--;
        stat[0]--; // 현재 스네이크 길이 감소
        stat[2]++; // 먹은 Poison 아이템 개수 증가
    }
    else if (item == SPEED_SLOW) {
        timeoutMs += 100; // 게임 속도 지연 (느려짐)
        stat[4]++; // 먹은 Slow 아이템 개수 증가
    }
}

// ==========================================
// [3단계] UI 반영 및 미션 클리어 조건 검사
// ==========================================
void ItemAndMission::setMission() {
    std::memset(stat, 0, sizeof(stat));
    std::memset(statMission, 0, sizeof(statMission));
    std::memset(chkMission, ' ', sizeof(chkMission));

    stat[0] = 3; // 기본 시작 길이 초기화
    
    // 스테이지 기본 미션 목표 설정 (테스트용 예시 값)
    statMission[0] = 5; // 목표 길이
    statMission[1] = 2; // 목표 Growth 개수
    statMission[2] = 2; // 목표 Poison 개수
    statMission[3] = 2; // 목표 Gate 통과 횟수
}

bool ItemAndMission::isMissionClear() {
    int count = 0;
    for (int i = 0; i < 4; i++) {
        if (stat[i] >= statMission[i]) {
            chkMission[i] = 'v'; // 달성 시 체크 마크 표시
            count++;
        } else if (!i) {
            chkMission[i] = ' ';
        }
    }
    return (count == 4); // 4개 미션 모두 달성 시 true 반환
}

void ItemAndMission::drawScoreAndMission(WINDOW* scoreWin, WINDOW* missionWin, int snakeMaxLength) {
    // Score 윈도우 갱신 및 UI 출력
    wclear(scoreWin);
    wattron(scoreWin, COLOR_PAIR(10));
    box(scoreWin, 0, 0);
    mvwprintw(scoreWin, 0, 10, "[ SCORE ]");
    wattroff(scoreWin, COLOR_PAIR(10));
    
    mvwprintw(scoreWin, 3, 5, "Snake Length: %d / %d", stat[0], snakeMaxLength);
    mvwprintw(scoreWin, 6, 5, "Growth Items: %d", stat[1]);
    mvwprintw(scoreWin, 9, 5, "Poison Items: %d", stat[2]);
    mvwprintw(scoreWin, 12, 5, "Slow Items: %d", stat[4]);
    mvwprintw(scoreWin, 15, 5, "Gate: %d", stat[3]);
    wrefresh(scoreWin);

    // Mission 윈도우 갱신 및 UI 출력
    wclear(missionWin);
    wattron(missionWin, COLOR_PAIR(10));
    box(missionWin, 0, 0);
    mvwprintw(missionWin, 0, 9, "[ MISSION ]");
    wattroff(missionWin, COLOR_PAIR(10));
    
    mvwprintw(missionWin, 3, 5, "Snake Length: %d ( %c )", statMission[0], chkMission[0]);
    mvwprintw(missionWin, 6, 5, "Growth Items: %d ( %c )", statMission[1], chkMission[1]);
    mvwprintw(missionWin, 9, 5, "Poison Items: %d ( %c )", statMission[2], chkMission[2]);
    mvwprintw(missionWin, 12, 5, "Gate: %d ( %c )", statMission[3], chkMission[3]);
    wrefresh(missionWin);
}