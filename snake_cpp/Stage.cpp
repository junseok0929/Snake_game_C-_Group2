// Stage.cpp
#include "Stage.h"

Stage::Stage()
{
    srand((unsigned)time(0)); // 시드값 설정
    initscr();                // ncurses init
    keypad(stdscr, TRUE);     // 키보드 입력 모드
    cbreak();                 // 문자로 입력 받기
    noecho();                 // 문자가 화면에 출력되지 않게 하기

    start_color();             // 컬러모드
    if (has_colors() == FALSE) // 컬러를 지원하지 않는 경우
    {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    if (init_color(COLOR_BLUE, 0, 0, 300) == ERR) // COLOR_BLUE를 사용자 정의로 변경할 수 없는 경우
    {
        printw("Your terminal cannot change the color definitions\n");
        printw("press any key to continue...\n");
        getch(); // 사용자로부터 입력 대기, 사용자가 아무 키나 누를때까지 일시정지
        moveSnake();
    }

    init_pair(EMPTY, COLOR_WHITE, COLOR_BLACK);
    init_pair(WALL, COLOR_BLACK, COLOR_WHITE);
    init_pair(IMMUNE_WALL, COLOR_BLACK, COLOR_WHITE);
    init_pair(SNAKE_HEAD, COLOR_YELLOW, COLOR_BLACK);
    init_pair(SNAKE_BODY, COLOR_GREEN, COLOR_BLACK);
    init_pair(GROWTH_ITEM, COLOR_WHITE, COLOR_BLUE);
    init_pair(POISON_ITEM, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(SPEED_SLOW, COLOR_BLACK, COLOR_RED);
    init_pair(GATE, COLOR_CYAN, COLOR_CYAN);

    init_pair(10, COLOR_YELLOW, COLOR_BLACK);
    init_pair(11, COLOR_BLACK, COLOR_WHITE);
    init_pair(12, COLOR_MAGENTA, COLOR_YELLOW);
    init_pair(13, COLOR_WHITE, COLOR_WHITE);

    menuLastFocus = 0,
    speed = 2,
    optLastFocus = speed - 1,
    tcount = 0;
    manualTitle = "< MANUAL >",
    menuTitle = "< MENU >",
    menuTxt[0] = " - PLAY: Start the game",
    menuTxt[1] = " - HELP: Manual of the game",
    menuTxt[2] = " - EXIT: Exit the game";
    shorTitle = "< SHORTCUTS >";
    shorTxt[0] = " - Arrow up(^): MOVE UP",
    shorTxt[1] = " - Arrow down(v): MOVE DOWN",
    shorTxt[2] = " - Arrow left(<): MOVE LEFT",
    shorTxt[3] = " - Arrow right(>): MOVE RIGHT",
    shorTxt[4] = " - 'p': GAME PAUSE",
    shorTxt[5] = " - 'r': GAME RESUME",
    shorTxt[6] = " - 'esc': BACK TO THE MAIN MENU";
}

Stage::~Stage()
{
    delwin(scrollBar);
    delwin(description);
    delwin(manual);
    delwin(info);
    delwin(mission);
    delwin(score);
    delwin(game);
    endwin();
}

void Stage::screenLock()
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

void Stage::play()
{
    screenLock();
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
        appearGate();
        appearItem(); // Spawn initial items immediately at stage start.
        // appearPlusGate();
        drawMap();
        while (1)
        {
            switch (getch())
            {
            case LEFT:
                dir = LEFT;
                break;
            case UP:
                dir = UP;
                break;
            case RIGHT:
                dir = RIGHT;
                break;
            case DOWN:
                dir = DOWN;
                break;
            case PAUSE:
                alert(y / 2 - 4, x / 2 - 34, "Press 'r' to play!", TRUE);
                while (1)
                {
                    if (getch() == RESUME)
                        break;
                }
                break;
            case ESC:
                endwin();
                return;
            }
            moveSnake();
            if (chkEnter)
            {
                int *stat = itemMission.getStat();
                if (++n >= stat[0])
                {
                    disappearGate();
                    // disappearPlusGate();
                    appearGate();
                    // appearPlusGate();
                    n = 0;
                    chkEnter = FALSE;
                }
            }
            if (++msTime % (msDiv[speed - 1] * 10) == 0) // 일정 시간마다 아이템 생성
            {
                disappearItem();
                appearItem();
            }

            // 아이템을 모두 먹은 경우에는 주기를 기다리지 않고 즉시 재생성
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
            timeout(timeoutMs);
        }
        level++;
    }
    endwin();
}

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

// 두 개의 게이트를 무작위 위치의 벽에 생성하는 코드
void Stage::appearGate()
{
    // n: 벽의 위치를 결정할 난수를 담을 변수 (좌, 우, 상, 하, 가운데 중 하나를 고르는 변수)
    // x: 게이트를 생성할 벽의 열 좌표 나타낼 변수 /  y: 게이트를 생성할 벽의 행 좌표 나타낼 변수
    int n, y, x;
    for (int i = 0; i < 2; i++) // 게이트 2개를 만들어야 하므로, for문을 2번 돌린다.
    {
        while (1) // 조건에 맞는 벽 위치를 찾을 때까지 난수 생성
        {
            n = rand() % (!level ? 4 : 5);                      // 0레벨 스테이지면 n에 4가 안나오도록. 0레벨 스테이지에는 가운데에 벽이 없으므로 (0~3: 좌/우/상/하에 gate가 위치, 4: 가운데에 gate가 위치)
            y = rand() % (MAP_ROW - (i ? 3 : 2)) + (i ? 2 : 1); // 두 게이트가 가장자리에 너무 가까이 붙어서, 억울하게 GameOver되는 상황을 방지하기 위해
            x = rand() % (MAP_COL - (i ? 3 : 2)) + (i ? 2 : 1); // 첫 번째 게이트와 두 번째 게이트의 좌표 생성 범위를 다르게 설정.

            switch (n)
            {
            case 0:
                y = 0; // 상단 벽에 gate 위치
                break;
            case 1:
                x = 0; // 좌측 벽에 gate 위치
                break;
            case 2:
                x = COL_END; // 우측 벽에 gate 위치
                break;
            case 3:
                y = ROW_END; // 하단 벽에 gate 위치
                break;
            case 4: // 가운데 벽에 gate 위치
                while (1)
                {
                    // 가운데에 맵이 존재하는 맵들에서, 벽이 존재할 가능성이 있는 범위를 랜덤으로 지정하고, 그곳에 정말 벽이 있을 때까지 while문 반복.
                    x = rand() % 30 + 10; // x: 10 ~ 39
                    y = rand() % 15 + 5;  // y: 5 ~ 19
                    if (map[y][x] == WALL)
                        break;
                }
            }
            if (map[y][x] == WALL) // 정말 벽이 존재하는지 최종 확인
            {
                map[y][x] = GATE;                   // 벽이 존재하면 gate 지정
                gatePos.push_back(make_pair(y, x)); // 게이트 생성 후, 일정 시간이 지난 뒤, disappearGate()에서 게이트를 삭제하기 위해 좌표 저장.
                break;
            }
        }
        if (i == 0)
            gate1 = new Something(y, x, GATE); // Something.h에서 정의한 Something 인스턴스 생성
        if (i == 1)
            gate2 = new Something(y, x, GATE);
    }
}

// void Stage::appearPlusGate() // 요건 삭제해야 할지도
//{
//     int n, y, x;
//     for (int i = 0; i < 2; i++)
//     {
//         while (1)
//         {
//             n = rand() % (!level ? 4 : 5);
//             y = rand() % (MAP_ROW - (i?3:2)) + (i?2:1);
//             x = rand() % (MAP_COL - (i?3:2)) + (i?2:1);
//             switch (n)
//             {
//             case 0:
//                 y = 0;
//                 break;
//             case 1:
//                 x = 0;
//                 break;
//             case 2:
//                 x = COL_END;
//                 break;
//             case 3:
//                 y = ROW_END;
//                 break;
//             case 4:
//                 while (1)
//                 {
//                     // 가운데에 벽이 존재하는 맵에서의 벽이 존재하는 x,y 범위에서, 정말로 벽이 존재하는 부분을 찾을 때까지 while문
//                     x = rand() % 30 + 10; // 10 ~ 39
//                     y = rand() % 15 + 5;  // 5 ~ 19
//                     if (map[y][x] == WALL && map[y][x] != GATE)
//                         break;
//                 }
//             }
//             if(i==0){
//                 if (map[y][x] == WALL && map[y][x] != GATE )
//                 {
//                     map[y][x] = PLUS_GATE;
//                     plusGatePos.push_back(make_pair(y, x));
//                     break;
//                 }
//             }
//             if(i==1){
//                if (map[y][x] == WALL && map[y][x] != GATE && map[y][x] != PLUS_GATE )
//                 {
//                     map[y][x] = PLUS_EXIT;
//                     plusGatePos.push_back(make_pair(y, x));
//                     break;
//                 }
//             }
//         }
//         if (i == 0)
//             plusGate1 = new Something(y, x, PLUS_GATE);
//         if (i == 1)
//             plusGate2 = new Something(y, x, PLUS_EXIT);
//     }
// }

void Stage::disappearItem()
{
    itemMission.disappearItem(map);
}

void Stage::disappearGate()
{
    for (auto gate : gatePos) // gatePos에 추가했던 gate의 좌표를 꺼내서 for문. (pair로 저장됨)
    {
        if (map[gate.first][gate.second] == GATE) // 저장되었던 gate를 wall로 바꿈.
            map[gate.first][gate.second] = WALL;
    }
    gatePos.clear();
}

// void Stage::disappearPlusGate()
//{
//     for (auto gate : plusGatePos)
//     {
//         if(map[gate.first][gate.second] == PLUS_GATE)
//             map[gate.first][gate.second] = WALL;
//         if(map[gate.first][gate.second] == PLUS_EXIT)
//             map[gate.first][gate.second] = WALL;
//     }
//     plusGatePos.clear();
// }

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
        enterGate(p);
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

void Stage::enterGate(Something *head)
{
    chkEnter = TRUE;                                // 뱀이 게이트에 진입했음을 알리는 상태 플래그 (이 플래그는 play() 함수에서 뱀이 게이트를 통과한 후 일정 턴이 지나면 게이트를 소멸시키고 새로 생성하는 기준점 역할)
    if (gate1->x == head->x && gate1->y == head->y) // head의 좌표와 gate의 좌표와 일치할 때,
    {
        if (gate2->x == 0) // 연결된 gate가 좌측벽에 붙어있으면
        {
            head->x = 1; // head를 연결된 gate의 왼쪽으로 이동
            head->y = gate2->y;
            dir = RIGHT; // head의 방향은 오른쪽으로 향하도록
        }
        else if (gate2->x == COL_END) // 연결된 gate가 우측벽에 붙어있으면
        {
            head->x = COL_END - 1; // head를 연결된 gate의 오른쪽으로 이동
            head->y = gate2->y;
            dir = LEFT; // head의 방향은 왼쪽으로 향하도록
        }
        else if (gate2->y == 0) // 연결된 gate가 상단벽에 붙어있으면
        {
            head->x = gate2->x; // head를 gate의 아래쪽으로 이동
            head->y = 1;
            dir = DOWN; // head의 방향은 아래쪽으로 향하도록
        }
        else if (gate2->y == ROW_END) // 연결된 gate가 하단벽에 붙어있으면
        {
            head->x = gate2->x; // head를 gate 위쪽으로 이동
            head->y = ROW_END - 1;
            dir = UP; // head의 방향은 위쪽으로 향하도록
        }
        else
        {                    // 연결된 gate가 맵 내부에 있다면
            findRoot(gate2); // findRoot()함수로 유효한 방향을 판단하고, (x,y) 설정
            if (dir == LEFT)
            {
                head->x = gate2->x - 1;
                head->y = gate2->y;
            }
            else if (dir == UP)
            {
                head->x = gate2->x;
                head->y = gate2->y - 1;
            }
            else if (dir == RIGHT)
            {
                head->x = gate2->x + 1;
                head->y = gate2->y;
            }
            else if (dir == DOWN)
            {
                head->x = gate2->x;
                head->y = gate2->y + 1;
            }
        }
    }
    else if (gate2->x == head->x && gate2->y == head->y)
    {
        if (gate1->x == 0)
        {
            head->x = 1;
            head->y = gate1->y;
            dir = RIGHT;
        }
        else if (gate1->x == COL_END)
        {
            head->x = COL_END - 1;
            head->y = gate1->y;
            dir = LEFT;
        }
        else if (gate1->y == 0)
        {
            head->x = gate1->x;
            head->y = 1;
            dir = DOWN;
        }
        else if (gate1->y == ROW_END)
        {
            head->x = gate1->x;
            head->y = ROW_END - 1;
            dir = UP;
        }
        else
        {
            findRoot(gate1);
            if (dir == LEFT)
            {
                head->x = gate1->x - 1;
                head->y = gate1->y;
            }
            else if (dir == UP)
            {
                head->x = gate1->x;
                head->y = gate1->y - 1;
            }
            else if (dir == RIGHT)
            {
                head->x = gate1->x + 1;
                head->y = gate1->y;
            }
            else if (dir == DOWN)
            {
                head->x = gate1->x;
                head->y = gate1->y + 1;
            }
        }
    }
    itemMission.incrementGateStat(); // 게이트 통과 횟수 증가
}

// void Stage::enterPlusGate(Something *head)
//{
//     chkPlusEnter = TRUE;
//     if (plusGate1->x == head->x && plusGate1->y == head->y)
//     {
//         if (plusGate2->x == 0)
//         {
//             head->x = 1;
//             head->y = plusGate2->y;
//             dir = RIGHT;
//         }
//         else if (plusGate2->x == COL_END)
//         {
//             head->x = COL_END - 1;
//             head->y = plusGate2->y;
//             dir = LEFT;
//         }
//         else if (plusGate2->y == 0)
//         {
//             head->x = plusGate2->x;
//             head->y = 1;
//             dir = DOWN;
//         }
//         else if (plusGate2->y == ROW_END)
//         {
//             head->x = plusGate2->x;
//             head->y = ROW_END - 1;
//             dir = UP;
//         }
//         findRoot(plusGate2);
//         if (dir == LEFT)
//         {
//             head->x = plusGate2->x - 1;
//             head->y = plusGate2->y;
//         }
//         else if (dir == UP)
//         {
//             head->x = plusGate2->x;
//             head->y = plusGate2->y - 1;
//         }
//         else if (dir == RIGHT)
//         {
//             head->x = plusGate2->x + 1;
//             head->y = plusGate2->y;
//         }
//         else if (dir == DOWN)
//         {
//             head->x = plusGate2->x;
//             head->y = plusGate2->y + 1;
//         }
//     }
//     else if (plusGate2->x == head->x && plusGate2->y == head->y)
//     {
//         if (plusGate1->x == 0)
//         {
//             head->x = 1;
//             head->y = plusGate1->y;
//             dir = RIGHT;
//         }
//         else if (plusGate1->x == COL_END)
//         {
//             head->x = COL_END - 1;
//             head->y = plusGate1->y;
//             dir = LEFT;
//         }
//         else if (plusGate1->y == 0)
//         {
//             head->x = plusGate1->x;
//             head->y = 1;
//             dir = DOWN;
//         }
//         else if (plusGate1->y == ROW_END)
//         {
//             head->x = plusGate1->x;
//             head->y = ROW_END - 1;
//             dir = UP;
//         }
//         findRoot(plusGate1);
//         if (dir == LEFT)
//         {
//             head->x = plusGate1->x - 1;
//             head->y = plusGate1->y;
//         }
//         else if (dir == UP)
//         {
//             head->x = plusGate1->x;
//             head->y = plusGate1->y - 1;
//         }
//         else if (dir == RIGHT)
//         {
//             head->x = plusGate1->x + 1;
//             head->y = plusGate1->y;
//         }
//         else if (dir == DOWN)
//         {
//             head->x = plusGate1->x;
//             head->y = plusGate1->y + 1;
//         }
//     }
//     stat[3]++;
// }

// 맵 내부에 위치한 벽에 게이트를 형성할 때, 어느 방향으로 빠져나와야 하는지 결정하는 함수
int Stage::findRoot(Something *gate)
{
    for (int i = 0; i < 4; i++) // 4방향(상,하,좌,우)을 탐색하기 위한 for문.
    {
        if (dir == LEFT) // 게이트를 진입하는 지렁이의 방향이 왼쪽일 때
        {
            if (map[gate->y][gate->x - 1] == EMPTY) // 연결된 게이트의 왼쪽이 빈공간이면, 왼쪽 방향 그대로 유지
                return dir;
            else
                dir = KEY_UP; // 아닐 경우, 위쪽 방향으로 변경
        }
        else if (dir == KEY_UP)
        {
            if (map[gate->y - 1][gate->x] == EMPTY)
                return dir;
            else
                dir = RIGHT;
        }
        else if (dir == RIGHT)
        {
            if (map[gate->y][gate->x + 1] == EMPTY)
                return dir;
            else
                dir = DOWN;
        }
        else if (dir == DOWN)
        {
            if (map[gate->y + 1][gate->x] == EMPTY)
                return dir;
            else
                dir = LEFT;
        }
    }
    return dir;
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