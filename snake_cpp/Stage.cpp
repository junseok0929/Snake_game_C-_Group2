// Stage.cpp
#include "Stage.h"
#include <chrono>

Stage::Stage()
{
    // 게임 진행에 필요한 색상만 초기화
    init_pair(EMPTY, COLOR_WHITE, COLOR_BLACK);
    init_pair(WALL, COLOR_BLACK, COLOR_WHITE);
    init_pair(IMMUNE_WALL, COLOR_BLACK, COLOR_WHITE);
    init_pair(SNAKE_HEAD, COLOR_YELLOW, COLOR_BLACK);
    init_pair(SNAKE_BODY, COLOR_GREEN, COLOR_BLACK);
    init_pair(GROWTH_ITEM, COLOR_WHITE, COLOR_BLUE);
    init_pair(POISON_ITEM, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(SPEED_SLOW, COLOR_BLACK, COLOR_RED);
    init_pair(GATE, COLOR_CYAN, COLOR_CYAN);

    speed = 2;
    tcount = 0;
    level = 0;
    y = 40; x = 120; // 맵 렌더링에 사용되는 기본 해상도
}

Stage::~Stage()
{
    if(info) delwin(info);
    if(mission) delwin(mission);
    if(score) delwin(score);
    if(game) delwin(game);
}

/*void Stage::screenLock()
{
    cout << "\e[3;240;120t";
    cout << "\e[8;40;120t";
    system("resize -s 40 120");
    y = 40, x = 120;
    mvprintw(y - 1, 0, "SnakeGame");
    sizeY = y / 1.5,
    sizeX = x / 1.5,
    startY = y / 2 - sizeY / 2,
    startX = x / 2 - sizeX / 2,
    desSizeY = sizeY - 6,
    desSizeX = sizeX - 6,
    desStartY = startY + 3,
    desStartX = startX + 3,
    txtLines = 26,
    hidTxtLen = txtLines - desSizeY > 0 ? txtLines - desSizeY : 0,
    scrollBarLen = desSizeY - hidTxtLen;
}


string Stage::menu()
{
    clear();
    screenLock();
    curs_set(0);
    string txt[4];
    txt[0] = "[ SNAKE GAME ]";
    int focus = menuLastFocus;
    level = 0;
    while (1)
    {
        if (!focus)
            focus = 300;
        txt[1] = "PLAY";
        txt[2] = "HELP";
        txt[3] = "EXIT";
        attron(COLOR_PAIR(10));
        mvprintw(y / 2 - 2, x / 2 - txt[0].length() / 2, txt[0].c_str());
        attroff(COLOR_PAIR(10));
        for (int i = 1; i < sizeof(txt) / sizeof(txt[0]); i++)
        {
            if (i == abs(focus % 4 + 1))
            {
                attron(COLOR_PAIR(11));
                mvprintw(y / 2 + i, x / 2 - (txt[i].length() / 2), txt[i].c_str());
                attroff(COLOR_PAIR(11));
            }
            else
                mvprintw(y / 2 + i, x / 2 - (txt[i].length() / 2), txt[i].c_str());
        }
        switch (getch())
        {
        case UP:
            focus--;
            break;
        case DOWN:
            focus++;
            break;
        case ENTER:
            menuLastFocus = focus;
            return txt[abs(focus % 4 + 1)];
        }
    }
    return NULL;
}

*/
void Stage::play()
{
    setMap();
    int n;
    for (int i = 0; i < STAGE_NUM; i++)
    {
       timeoutMs = speedMs[speed - 1];
        msTime = n = 0;
        dir = LEFT;
        copyMap(i);
        setMission();
        makeSnake();
        gateManager.appearGate(map, level);
        drawMap();

        // 키 입력을 실시간으로 감지하기 위해 아주 짧은 타임아웃 설정
        timeout(10);
        auto lastTick = std::chrono::steady_clock::now();
       
        while (1)
        {
            int key = getch();
            if (key != ERR)
            {
                if (key == LEFT || key == RIGHT || key == UP || key == DOWN)
                {
                    dir = key; // 방향 업데이트 (반대 방향 필터링 없음 -> 누르면 기존처럼 죽음)
                }
                else if (key == PAUSE)
                {
                    alert(y / 2 - 4, x / 2 - 34, "Press 'r' to play!", TRUE);
                    while (1)
                    {
                        if (getch() == RESUME)
                            break;
                    }
                    lastTick = std::chrono::steady_clock::now(); // 일시정지 후 타이머 리셋
                }
                else if (key == ESC)
                {
                    endwin();
                    return;
                }
                
            }

            // 실제 게임 로직 실행: 원래 설정된 timeoutMs 시간이 지났을 때만 실행
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTick).count();

            if (elapsed >= timeoutMs)
            {
                lastTick = now; // 타이머 초기화

                moveSnake();
                if (chkEnter)
                {
                    int *stat = itemMission.getStat();
                    if (++n >= stat[0])
                    {
                        gateManager.disappearGate(map);
                        gateManager.appearGate(map, level);
                        n = 0;
                        chkEnter = FALSE;
                    }
                }
                if (++msTime % (msDiv[speed - 1] * 20) == 0)
                {
                    disappearItem();
                    appearItem();
                }

                bool hasItem = false;
                for (int r = 0; r < MAP_ROW && !hasItem; r++)
                {
                    for (int c = 0; c < MAP_COL; c++)
                    {
                        if (map[r][c] == GROWTH_ITEM || map[r][c] == POISON_ITEM || map[r][c] == SPEED_SLOW)
                        {
                            hasItem = true;
                            break;
                        }
                    }
                }
                if (!hasItem)
                    appearItem();

                if (itemMission.getStat()[0] < 3)
                    gameOver();
                if (isMissionClear())
                {
                    alert(y / 2 - 4, x / 2 - 27, "Stage Clear!", FALSE);
                    speed++;
                    break;
                }
                if (checkGameOver())
                {
                    alert(y / 2 - 4, x / 2 - 25, "Game Over!", FALSE);
                    return;
                }
                drawMap();
            }
        }
        level++;
    }
}

/*
void Stage::help()
{
    screenLock();
    int ySize = 0, yScroll = 0;
    while (1)
    {
        manual = newwin(sizeY, sizeX, startY, startX);
        description = newwin(desSizeY, desSizeX, desStartY, desStartX);
        scrollBar = newwin(scrollBarLen, 2, desStartY + yScroll, startX + sizeX - 6);
        wattron(manual, COLOR_PAIR(10));
        box(manual, 0, 0);
        mvwprintw(manual, 0, sizeX / 2 - manualTitle.length() / 2, "%s", manualTitle.c_str());
        wattroff(manual, COLOR_PAIR(10));

        mvwprintw(description, 0 + ySize,
                  sizeX / 2 - menuTitle.length() / 2 - 3, "%s", menuTitle.c_str());
        for (int i = 0; i < sizeof(menuTxt) / sizeof(menuTxt[0]); i++)
            mvwprintw(description, 2 + (i * 2) + ySize, sizeX / 2 - menuTxt[2].length() / 2 - 3, "%s", menuTxt[i].c_str());

        mvwprintw(description, 11 + ySize,
                  sizeX / 2 - shorTitle.length() / 2 - 3, "%s", shorTitle.c_str());
        for (int i = 0; i < sizeof(shorTxt) / sizeof(shorTxt[0]); i++)
            mvwprintw(description, 13 + (i * 2) + ySize, sizeX / 2 - shorTxt[6].length() / 2 - 3, "%s", shorTxt[i].c_str());

        if (txtLines >= desSizeY)
        {
            wattron(scrollBar, COLOR_PAIR(10));
            box(scrollBar, 0, 0);
            wattroff(scrollBar, COLOR_PAIR(10));
        }
        refresh();
        wrefresh(manual);
        wrefresh(description);
        wrefresh(scrollBar);
    RE:
        switch (getch())
        {
        case UP:
            if (yScroll)
                yScroll--;
            else
                goto RE;
            if (ySize)
                ySize++;
            break;
        case DOWN:
            if (yScroll < desSizeY - scrollBarLen)
                yScroll++;
            else
                goto RE;
            if (ySize > desSizeY - txtLines && txtLines > desSizeY)
                ySize--;
            break;
        case ESC:
            return;
        }
    }
}
*/
// 맵 생성 함수
// 각 스테이지의 맵 데이터를 생성
// 3차원 배열 stage[stage][row][col] 사용
void Stage::setMap()
{
    int i, j, k;

    // 스테이지 개수만큼 동적 할당
    stage = new int **[STAGE_NUM];

    for (i = 0; i < STAGE_NUM; i++)
    {
        // 행(row) 생성
        stage[i] = new int *[MAP_ROW];

        for (j = 0; j < MAP_ROW; j++)
        {
            // 열(col) 생성
            stage[i][j] = new int[MAP_COL];
        }
    }

    // 각 스테이지 맵 초기화
    for (i = 0; i < STAGE_NUM; i++)
    {
        for (j = 0; j < MAP_ROW; j++)
        {
            for (k = 0; k < MAP_COL; k++)
            {
                // 가장자리는 WALL로 설정
                if (!j || !k || j == ROW_END || k == COL_END)
                    stage[i][j][k] = WALL;

                // 내부는 EMPTY로 설정
                else
                    stage[i][j][k] = EMPTY;
            }
        }

        // 모서리는 IMMUNE_WALL 설정
        /* 반 벽(WALL)과는 다른 특수벽 ex) 맵 모서리, 벽이 겹치는 지점, 절대 없어지면 안 되는 벽 */
        stage[i][0][0] = IMMUNE_WALL;
        stage[i][0][COL_END] = IMMUNE_WALL;
        stage[i][ROW_END][0] = IMMUNE_WALL;
        stage[i][ROW_END][COL_END] = IMMUNE_WALL;

        // Stage 2 내부 벽 생성 (장애물)
        if (i == 1)
        {
            for (int z = 10; z < 40; z++)
                stage[i][7][z] = WALL;
            for (int z = 10; z < 40; z++)
                stage[i][MAP_ROW - 7][z] = WALL;
        }

        // Stage 3 세로 벽 생성
        if (i == 2)
        {
            for (int z = 5; z < 20; z++)
                stage[i][z][MAP_COL - 15] = WALL;
            for (int z = 5; z < 20; z++)
                stage[i][z][15] = WALL;
        }

        // Stage 4 복합 벽 생성
        if (i == 3)
        {
            for (int z = 10; z < 40; z++)
            {
                // 가운데 통로 생성
                if (z > 22 && z < 27)
                    continue;
                stage[i][7][z] = WALL;
            }
            for (int z = 10; z < 40; z++)
            {
                if (z > 22 && z < 27)
                    continue;
                stage[i][MAP_ROW - 7][z] = WALL;
            }
            // 세로 벽 생성
            for (int z = 5; z < 20; z++)
            {
                if (z > 10 && z < 14)
                    continue;
                if (stage[i][z][MAP_COL - 15] == WALL)
                    stage[i][z][MAP_COL - 15] = IMMUNE_WALL;
                else
                    stage[i][z][MAP_COL - 15] = WALL;
            }
            for (int z = 5; z < 20; z++)
            {
                if (z > 10 && z < 14)
                    continue;
                if (stage[i][z][15] == WALL)
                    stage[i][z][15] = IMMUNE_WALL;
                else
                    stage[i][z][15] = WALL;
            }
        }
    }
}

// 스테이지 맵 복사 함수
// stage 배열의 특정 스테이지를
// 현재 게임용 map 배열로 복사
void Stage::copyMap(int nStage)
{
    // 현재 플레이할 맵 공간 동적 할당
    map = new int *[MAP_ROW];

    // 각 행(row)에 열(col) 공간 생성
    for (int i = 0; i < MAP_COL; i++)
        map[i] = new int[MAP_COL];

    // 선택한 스테이지의 맵 데이터를
    // 현재 게임 맵(map)으로 복사
    for (int i = 0; i < MAP_ROW; i++)
    {
        for (int j = 0; j < MAP_COL; j++)
            map[i][j] = stage[nStage][i][j];
    }
}

// 맵 출력 함수
// ncurses를 이용해 게임 화면 출력
void Stage::drawMap()
{
    // 게임 맵 출력용 윈도우 생성
    game = newwin(MAP_ROW, MAP_COL, y / 2 - MAP_ROW / 2, x / 2 - (MAP_COL / 2 + 16));

    // 맵 전체 출력
    for (int i = 0; i < MAP_ROW; i++)
    {
        for (int j = 0; j < MAP_COL; j++)
        {
            // 현재 좌표의 오브젝트 종류 저장
            int index = map[i][j];

            // 해당 오브젝트 색상 적용
            wattron(game, COLOR_PAIR(index));

            // 해당 위치에 문자 출력
            mvwaddch(game, i, j, itemIndex[index]);

            // 색상 해제
            wattroff(game, COLOR_PAIR(index));
        }

        printw("\n");
    }

    // SCORE 창 생성
    score = newwin(19, 30, y / 2 - (MAP_ROW / 2 + 4), x / 2 + MAP_COL / 2 - 7.4);

    wattron(score, COLOR_PAIR(10));

    // 테두리 생성
    box(score, 0, 0);

    // 제목 출력
    mvwprintw(score, 0, 10, "[ SCORE ]");

    wattroff(score, COLOR_PAIR(10));

    // MISSION 창 생성
    mission = newwin(16, 30, y / 2 - (MAP_ROW / 2 + 4) + 19, x / 2 + MAP_COL / 2 - 7.4);
    itemMission.drawScoreAndMission(score, mission, SNAKE_MAX_LENGTH);

    // 현재 스테이지 및 시간 출력
    info = newwin(4, 15, y / 2 - (MAP_ROW / 2 + 4), x / 2 + MAP_COL / 2 - 47.4);

    // 현재 스테이지 번호 출력
    mvwprintw(info, 0, 1, "[ STAGE %d/%d ]", level + 1, STAGE_NUM);

    // 플레이 시간 출력
    mvwprintw(info, 2, 3, "< %02d:%02d >", msTime / (msDiv[speed - 1] * 60), (msTime / msDiv[speed - 1]) % 60);

    // 화면 갱신
    refresh();

    // 각 윈도우 출력
    wrefresh(info);
    wrefresh(game);
    wrefresh(score);
    wrefresh(mission);
}

void Stage::appearItem()
{
    itemMission.appearItem(map, level);
}


void Stage::disappearItem()
{
    itemMission.disappearItem(map);
}



// 스네이크 생성 함수
// 연결 리스트 형태로 Snake 생성
void Stage::makeSnake()
{
    // 초기 길이 3
    itemMission.updateSnakeLengthStat(3);

    int row = 13;
    int col = 26;

    // 첫 번째 몸통 생성 (Tail)
    Snail = new Something(row, col--, SNAKE_BODY);

    // 두 번째 몸통 생성
    Something *p = new Something(row, col--, SNAKE_BODY);

    // 첫 번째 몸통과 연결
    Snail->link = p;

    // Head 생성
    p = new Something(row, col--, SNAKE_HEAD);

    // 두 번째 몸통과 Head 연결
    Snail->link->link = p;

    // 맵에 Snake 출력
    map[Snail->y][Snail->x] = Snail->who;

    p = Snail->link;
    map[p->y][p->x] = p->who;

    p = p->link;
    map[p->y][p->x] = p->who;
}

// 스네이크 이동 함수
// 몸통 따라오기 + Head 이동 + 충돌 판정
void Stage::moveSnake()
{
    // Tail 위치를 EMPTY로 변경
    if (map[Snail->y][Snail->x] != WALL) // 본체 Head의 위치가 Wall이 아닌 경우
        map[Snail->y][Snail->x] = EMPTY; // 해당 위치 -> 빈 공간 설정

    // q : 이전 노드
    // p : 현재 노드
    Something *q = Snail;
    Something *p = q->link;

    // 몸통 이동
    // 앞 노드 좌표를 뒤 노드가 따라감
    while (p->link != NULL) // 본체 body를 따라 노드를 이동하며 각 노드의 좌표를 이전 좌표로 업데이트
    {
        q->x = p->x;
        q->y = p->y;

        q = p;
        p = p->link;
    }

    // Head 이동
    // 방향(dir)에 따라 좌표 변경
    if (dir == LEFT) // 현재 방향에 따라 본체의 Head와 Head 다음 노드의 좌표 업데이트
    {
        map[p->y][p->x] = q->who;

        q->x = p->x;
        q->y = p->y;

        p->x--;
    }
    else if (dir == UP)
    {
        map[p->y][p->x] = q->who;

        q->x = p->x;
        q->y = p->y;

        p->y--;
    }
    else if (dir == RIGHT)
    {
        map[p->y][p->x] = q->who;

        q->x = p->x;
        q->y = p->y;

        p->x++;
    }
    else if (dir == DOWN)
    {
        map[p->y][p->x] = q->who;

        q->x = p->x;
        q->y = p->y;

        p->y++;
    }

    // 충돌 판정
    // 벽 또는 몸통과 충돌 시 게임 종료
    if (map[p->y][p->x] == WALL || map[p->y][p->x] == SNAKE_BODY) // 본체가 벽이나 다른 본체의 몸통
    {
        map[p->y][p->x] = IMMUNE_WALL;
        gameOver();
    }

    // Gate 진입
    if (map[p->y][p->x] == GATE)
    {
        gateManager.enterGate(p, dir, map, chkEnter, itemMission);
    }

    // Plus Gate 진입
    // if (map[p->y][p->x] == PLUS_GATE)
    //{
    //    enterPlusGate(p);
    //}
    // if (map[p->y][p->x] == PLUS_EXIT)
    //{
    //    enterPlusGate(p);
    //}

    // 아이템 먹기
    if (map[p->y][p->x] == GROWTH_ITEM)
    {
        eatItem(GROWTH_ITEM);
    }

    if (map[p->y][p->x] == POISON_ITEM)
    {
        eatItem(POISON_ITEM);
    }

    if (map[p->y][p->x] == SPEED_SLOW)
    {
        eatItem(SPEED_SLOW);
    }

    // Head 위치를 맵에 반영
    map[p->y][p->x] = p->who;
}


void Stage::eatItem(int item)
{
    int *stat = itemMission.getStat();
    if (item == GROWTH_ITEM)
    {
        if (stat[0] == 10)
            return;
        Something *p = new Something(Snail->y, Snail->x, SNAKE_BODY);
        if (Snail->x - Snail->link->x == 1)
            p->x++;
        else if (Snail->y - Snail->link->y == 1)
            p->y++;
        else if (Snail->x - Snail->link->x == -1)
            p->x--;
        else if (Snail->y - Snail->link->y == -1)
            p->y--;
        p->link = Snail;
        Snail = p;
        if (map[Snail->y][Snail->x] != WALL)
            map[Snail->y][Snail->x] = Snail->who;
        int snakeLength = stat[0];
        itemMission.eatItem(GROWTH_ITEM, snakeLength, timeoutMs);
        itemMission.updateSnakeLengthStat(snakeLength);
    }
    else if (item == POISON_ITEM)
    {
        map[Snail->y][Snail->x] = EMPTY;
        Snail = Snail->link;
        int snakeLength = stat[0];
        itemMission.eatItem(POISON_ITEM, snakeLength, timeoutMs);
        itemMission.updateSnakeLengthStat(snakeLength);
    }
    else if (item == SPEED_SLOW)
    {
        int snakeLength = stat[0];
        itemMission.eatItem(SPEED_SLOW, snakeLength, timeoutMs);
        itemMission.updateSnakeLengthStat(snakeLength);
    }
}

void Stage::setMission()
{
    finish = chkEnter = FALSE;
    itemMission.setMission();
}

bool Stage::isMissionClear()
{
    return itemMission.isMissionClear();
}

// 게임 오버 함수
// finish 값을 true로 변경
void Stage::gameOver()
{
    finish = true;
}

void Stage::alert(int posY, int posX, const string msg, bool stopFlag)
{
    WINDOW *alert = newwin(7, msg.length() * 2, posY, posX);
    box(alert, 0, 0);
    wattron(alert, COLOR_PAIR(0));
    wbkgd(alert, COLOR_PAIR(2));
    mvwprintw(alert, 3, msg.length() / 2, "%s", msg.c_str());
    wrefresh(alert);
    if (!stopFlag)
        usleep(1750000);
}