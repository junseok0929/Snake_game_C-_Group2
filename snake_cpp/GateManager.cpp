#include "GateManager.h"
#include "Stage.h"

GateManager::GateManager() : gate1(nullptr), gate2(nullptr) {}

GateManager::~GateManager() {
    // [분석/수정] 기존 코드에 없던 메모리 누수 방지 코드 추가
    if (gate1) delete gate1;
    if (gate2) delete gate2;
}

// 두 개의 게이트를 무작위 위치의 벽에 생성하는 메서드
void GateManager::appearGate(int** map, int level) {
    // n: 벽의 위치를 결정할 난수를 담을 변수 (좌, 우, 상, 하, 가운데 중 하나를 고르는 변수)
    // x: 게이트를 생성할 벽의 열 좌표 나타낼 변수 /  y: 게이트를 생성할 벽의 행 좌표 나타낼 변수
    int n, y, x;
    for (int i = 0; i < 2; i++) {  // 게이트 2개를 만들어야 하므로, for문을 2번 돌린다.
        while (1) {                // 조건에 맞는 벽 위치를 찾을 때까지 난수 생성
            n = rand() % (!level ? 4 : 5);                          // 0레벨 스테이지면 n에 4가 안나오도록. 0레벨 스테이지에는 가운데에 벽이 없으므로 (0~3: 좌/우/상/하에 gate가 위치, 4: 가운데에 gate가 위치)
            y = rand() % (MAP_ROW - (i ? 3 : 2)) + (i ? 2 : 1);     // 두 게이트가 가장자리에 너무 가까이 붙어서, 억울하게 GameOver되는 상황을 방지하기 위해
            x = rand() % (MAP_COL - (i ? 3 : 2)) + (i ? 2 : 1);     // 첫 번째 게이트와 두 번째 게이트의 좌표 생성 범위를 다르게 설정.

            switch (n) {
            case 0: y = 0; break;           // 상단 벽에 gate 위치
            case 1: x = 0; break;           // 좌측 벽에 gate 위치
            case 2: x = COL_END; break;     // 우측 벽에 gate 위치
            case 3: y = ROW_END; break;     // 하단 벽에 gate 위치
            case 4:                         // 가운데 벽에 gate 위치
                while (1) {
                    // 가운데에 맵이 존재하는 맵들에서, 벽이 존재할 가능성이 있는 범위를 랜덤으로 지정하고, 그곳에 정말 벽이 있을 때까지 while문 반복.
                    x = rand() % 30 + 10;           // x: 10 ~ 39
                    y = rand() % 15 + 5;            // y: 5 ~ 19
                    if (map[y][x] == WALL) break;
                }
            }
            if (map[y][x] == WALL) {        // 정말 벽이 존재하는지 최종 확인
                map[y][x] = GATE;                           // 벽이 존재하면 gate 지정
                gatePos.push_back(std::make_pair(y, x));    // 게이트 생성 후, 일정 시간이 지난 뒤, disappearGate()에서 게이트를 삭제하기 위해 좌표 저장.
                break;
            }
        }
        if (i == 0) gate1 = new Something(y, x, GATE);      // Something.h에서 정의한 Something 인스턴스 생성
        if (i == 1) gate2 = new Something(y, x, GATE);
    }
}

// 게이트를 삭제하는 메서드
void GateManager::disappearGate(int** map) {
    for (auto gate : gatePos) {                     // gatePos에 추가했던 gate의 좌표를 꺼내서 for문. (pair로 저장됨)
        if (map[gate.first][gate.second] == GATE)   // 저장되었던 gate를 wall로 바꿈.
            map[gate.first][gate.second] = WALL;
    }
    gatePos.clear();

    // 게이트가 사라질 때, 메모리 누수 방지를 위해 동적 할당된 메모리 해제.
    if (gate1) { delete gate1; gate1 = nullptr; }
    if (gate2) { delete gate2; gate2 = nullptr; }
}

// 게이트에 들어가서 나왔을 때, snake의 진행방향과 head 방향을 처리하는 함수
void GateManager::enterGate(Something* head, int& dir, int** map, bool& chkEnter, ItemAndMission& itemMission) {
    chkEnter = true;    // 뱀이 게이트에 진입했음을 알리는 상태 플래그 (이 플래그는 play() 함수에서 뱀이 게이트를 통과한 후 일정 턴이 지나면 게이트를 소멸시키고 새로 생성하는 기준점 역할)
    if (gate1->x == head->x && gate1->y == head->y) {
        if (gate2->x == 0) {                                        // 연결된 gate가 좌측벽에 붙어있으면
            head->x = 1; head->y = gate2->y; dir = RIGHT;           // head를 연결된 gate의 왼쪽으로 이동하고, head의 방향은 오른쪽.
        }
        else if (gate2->x == COL_END) {                             // 연결된 gate가 우측벽에 붙어있으면
            head->x = COL_END - 1; head->y = gate2->y; dir = LEFT;  // head를 연결된 gate의 오른쪽으로 이동, head의 방향은 왼쪽
        }
        else if (gate2->y == 0) {                                   // 연결된 gate가 상단벽에 붙어있으면
            head->x = gate2->x; head->y = 1; dir = DOWN;            // head를 연결된 gate의 아래쪽으로 이동, head의 방향은 아래쪽
        }
        else if (gate2->y == ROW_END) {                             // 연결된 gate가 하단벽에 붙어있으면
            head->x = gate2->x; head->y = ROW_END - 1; dir = UP;    // head를 gate 위쪽으로 이동, head의 방향은 위쪽
        }
        else {                                                      // 연결된 gate가 맵 내부에 있다면
            dir = findRoot(gate2, dir, map);                        // findRoot()함수로 유효한 방향을 판단하고, (x,y) 설정
            if (dir == LEFT) { head->x = gate2->x - 1; head->y = gate2->y; }
            else if (dir == UP) { head->x = gate2->x; head->y = gate2->y - 1; }
            else if (dir == RIGHT) { head->x = gate2->x + 1; head->y = gate2->y; }
            else if (dir == DOWN) { head->x = gate2->x; head->y = gate2->y + 1; }
        }
    }
    else if (gate2->x == head->x && gate2->y == head->y) {
        if (gate1->x == 0) {
            head->x = 1; head->y = gate1->y; dir = RIGHT;
        }
        else if (gate1->x == COL_END) {

            head->x = COL_END - 1; head->y = gate1->y; dir = LEFT;
        }
        else if (gate1->y == 0) {
            head->x = gate1->x; head->y = 1; dir = DOWN;
        }
        else if (gate1->y == ROW_END) {
            head->x = gate1->x; head->y = ROW_END - 1; dir = UP;
        }
        else {
            dir = findRoot(gate1, dir, map);
            if (dir == LEFT) { head->x = gate1->x - 1; head->y = gate1->y; }
            else if (dir == UP) { head->x = gate1->x; head->y = gate1->y - 1; }
            else if (dir == RIGHT) { head->x = gate1->x + 1; head->y = gate1->y; }
            else if (dir == DOWN) { head->x = gate1->x; head->y = gate1->y + 1; }
        }
    }
    itemMission.incrementGateStat();    // 게이트 통과 횟수 증가
}


// 맵 내부에 위치한 벽에 게이트를 형성할 때, 어느 방향으로 빠져나와야 하는지 결정하는 함수
int GateManager::findRoot(Something* gate, int currentDir, int** map) {
    for (int i = 0; i < 4; i++) {                                           // 4방향(상,하,좌,우)을 탐색하기 위한 for문.
        if (currentDir == LEFT) {                                           // 게이트를 진입하는 지렁이의 방향이 왼쪽일 때
            if (map[gate->y][gate->x - 1] == EMPTY) return currentDir;      // 연결된 게이트의 왼쪽이 빈공간이면, 왼쪽 방향 그대로 유지
            else currentDir = KEY_UP;                                       // 아닐 경우, 위쪽 방향으로 변경
        }
        else if (currentDir == KEY_UP || currentDir == UP) {
            if (map[gate->y - 1][gate->x] == EMPTY) return currentDir;
            else currentDir = RIGHT;
        }
        else if (currentDir == RIGHT) {
            if (map[gate->y][gate->x + 1] == EMPTY) return currentDir;
            else currentDir = DOWN;
        }
        else if (currentDir == DOWN) {
            if (map[gate->y + 1][gate->x] == EMPTY) return currentDir;
            else currentDir = LEFT;
        }
    }
    return currentDir;
}