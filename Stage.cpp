#include "Stage.h"
#include "ItemAndMission.h" // 1. 분리한 헤더 추가

// 외부 혹은 Stage 클래스 멤버로 객체 생성
ItemAndMission itemManager; 

Stage::Stage()
{
    srand((unsigned)time(0)); 
    initscr(); 
    keypad(stdscr, TRUE); 
    cbreak(); 
    noecho(); 

    start_color(); 
    if (has_colors() == FALSE) 
    {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    if (init_color(COLOR_BLUE, 0, 0, 300) == ERR) 
    {
        printw("Your terminal cannot change the color definitions\n");
        printw("press any key to continue...\n");
        getch(); 
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
    
    menuLastFocus = 0;
    speed = 2;
    optLastFocus = speed - 1;
    tcount = 0;
    manualTitle = "< MANUAL >";
    menuTitle = "< MENU >";
    menuTxt[0] = " - PLAY: Start the game";
    menuTxt[1] = " - HELP: Manual of the game";
    menuTxt[2] = " - EXIT: Exit the game";
    shorTitle = "< SHORTCUTS >";
    shorTxt[0] = " - Arrow up(^): MOVE UP";
    shorTxt[1] = " - Arrow down(v): MOVE DOWN";
    shorTxt[2] = " - Arrow left(<): MOVE LEFT";
    shorTxt[3] = " - Arrow right(>): MOVE RIGHT";
    shorTxt[4] = " - 'p': GAME PAUSE";
    shorTxt[5] = " - 'r': GAME RESUME";
    shorTxt[6] = " - 'esc': BACK TO THE MAIN MENU";

    // ※ 기존 하드코딩된 변수 대입식 및 콤마(,) 연산자 오류 수정 완료
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
    sizeY = y / 1.5;
    sizeX = x / 1.5;
    startY = y / 2 - sizeY / 2;
    startX = x / 2 - sizeX / 2;
    desSizeY = sizeY - 6;
    desSizeX = sizeX - 6;
    desStartY = startY + 3;
    desStartX = startX + 3;
    txtLines = 26;
    hidTxtLen = txtLines - desSizeY > 0 ? txtLines - desSizeY : 0;
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
        roff(COLOR_PAIR(10));
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

vector<pair<int, int> > gatePos, plusGatePos; // itemPos 제거

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
        
        // 2. 미션 초기화 이관 호출
        itemManager.setMission(); 
        
        makeSnake();
        appearGate();
        appearPlusGate();
        drawMap();
        while (1)
        {
            switch (getch())
            {
            case LEFT:  dir = LEFT;  break;
            case UP:    dir = UP;    break;
            case RIGHT: dir = RIGHT; break;
            case DOWN:  dir = DOWN;  break;
            case PAUSE:
                alert(y / 2 - 4, x / 2 - 34, "Press 'r' to play!", TRUE);
                while (1) { if (getch() == RESUME) break; }
                break;
            case ESC:
                endwin();
                return;
            }
            
            moveSnake();
            
            if (chkEnter)
            {
                // 변동된 길이를 이관 객체에 동기화
                itemManager.updateSnakeLengthStat(stat[0]); 
                if (++n >= itemManager.getStat()[0]) // 이관된 데이터 참조
                {
                    disappearGate();
                    disappearPlusGate();
                    appearGate();
                    appearPlusGate();
                    n = 0;
                    chkEnter = FALSE;
                }
            }
            
            // 3. 아이템 자동 리스폰 로직 이관 호출
            if (++msTime % (msDiv[speed - 1] * 5) == 0)
            {
                itemManager.disappearItem(map);
                itemManager.appearItem(map, level);
            }
            
            // 4. 게임 오버 조건 검사 (길이 3 미만)
            if (itemManager.getStat()[0] < 3)
                gameOver();
                
            // 5. 미션 달성 검사 이관 호출
            if (itemManager.isMissionClear())
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

        mvwprintw(description, 0 + ySize, sizeX / 2 - menuTitle.length() / 2 - 3, "%s", menuTitle.c_str());
        for (int i = 0; i < sizeof(menuTxt) / sizeof(menuTxt[0]); i++)
            mvwprintw(description, 2 + (i * 2) + ySize, sizeX / 2 - menuTxt[2].length() / 2 - 3, "%s", menuTxt[i].c_str());

        mvwprintw(description, 11 + ySize, sizeX / 2 - shorTitle.length() / 2 - 3, "%s", shorTitle.c_str());
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
            if (yScroll) yScroll--; else goto RE;
            if (ySize) ySize++;
            break;
        case DOWN:
            if (yScroll < desSizeY - scrollBarLen) yScroll++; else goto RE;
            if (ySize > desSizeY - txtLines && txtLines > desSizeY) ySize--;
            break;
        case ESC:
            return;
        }
    }
}

void Stage::setMap()
{
    int i, j, k;
    stage = new int **[STAGE_NUM];
    for (i = 0; i < STAGE_NUM; i++)
    {
        stage[i] = new int *[MAP_ROW];
        for (j = 0; j < MAP_ROW; j++)
            stage[i][j] = new int[MAP_COL];
    }

    for (i = 0; i < STAGE_NUM; i++)
    {
        for (j = 0; j < MAP_ROW; j++)
        {
            for (k = 0; k < MAP_COL; k++)
            {
                if (!j || !k || j == ROW_END || k == COL_END)
                    stage[i][j][k] = WALL;
                else
                    stage[i][j][k] = EMPTY;
            }
        }
        stage[i][0][0] = IMMUNE_WALL;
        stage[i][0][COL_END] = IMMUNE_WALL;
        stage[i][ROW_END][0] = IMMUNE_WALL;
        stage[i][ROW_END][COL_END] = IMMUNE_WALL;
        if (i == 1)
        {
            for (int z = 10; z < 40; z++) stage[i][7][z] = WALL;
            for (int z = 10; z < 40; z++) stage[i][MAP_ROW - 7][z] = WALL;
        }
        if (i == 2)
        {
            for (int z = 5; z < 20; z++) stage[i][z][MAP_COL - 15] = WALL;
            for (int z = 5; z < 20; z++) stage[i][z][15] = WALL;
        }
        if (i == 3)
        {
            for (int z = 10; z < 40; z++) { if (z > 22 && z < 27) continue; stage[i][7][z] = WALL; }
            for (int z = 10; z < 40; z++) { if (z > 22 && z < 27) continue; stage[i][MAP_ROW - 7][z] = WALL; }
            for (int z = 5; z < 20; z++)
            {
                if (z > 10 && z < 14) continue;
                if (stage[i][z][MAP_COL - 15] == WALL) stage[i][z][MAP_COL - 15] = IMMUNE_WALL;
                else stage[i][z][MAP_COL - 15] = WALL;
            }
            for (int z = 5; z < 20; z++)
            {
                if (z > 10 && z < 14) continue;
                if (stage[i][z][15] == WALL) stage[i][z][15] = IMMUNE_WALL;
                else stage[i][z][15] = WALL;
            }
        }
    }
}

void Stage::copyMap(int nStage)
{
    map = new int *[MAP_ROW];
    for (int i = 0; i < MAP_COL; i++) map[i] = new int[MAP_COL];
    for (int i = 0; i < MAP_ROW; i++)
    {
        for (int j = 0; j < MAP_COL; j++) map[i][j] = stage[nStage][i][j];
    }
}

void Stage::drawMap()
{
    game = newwin(MAP_ROW, MAP_COL, y / 2 - MAP_ROW / 2, x / 2 - (MAP_COL / 2 + 16));
    for (int i = 0; i < MAP_ROW; i++)
    {
        for (int j = 0; j < MAP_COL; j++)
        {
            int index = map[i][j];
            wattron(game, COLOR_PAIR(index));
            mvwaddch(game, i, j, itemIndex[index]);
            wattroff(game, COLOR_PAIR(index));
        }
        printw("\n");
    }

    // 윈도우 뼈대만 생성
    score = newwin(19, 30, y / 2 - (MAP_ROW / 2 + 4), x / 2 + MAP_COL / 2 - 7.4);
    mission = newwin(16, 30, y / 2 - (MAP_ROW / 2 + 4) + 19, x / 2 + MAP_COL / 2 - 7.4);
    
    // 6. UI 그리기 권한 이관 호출
    itemManager.drawScoreAndMission(score, mission, SNAKE_MAX_LENGTH);

    info = newwin(4, 15, y / 2 - (MAP_ROW / 2 + 4), x / 2 + MAP_COL / 2 - 47.4);
    mvwprintw(info, 0, 1, "[ STAGE %d/%d ]", level + 1, STAGE_NUM);
    mvwprintw(info, 2, 3, "< %02d:%02d >", msTime / (msDiv[speed - 1] * 60), (msTime / msDiv[speed - 1]) % 60);

    refresh();
    wrefresh(info);
    wrefresh(game);
}

void Stage::appearGate()
{
    int n, y, x;
    for (int i = 0; i < 2; i++)
    {
        while (1)
        {
            n = rand() % (!level ? 4 : 5);
            y = rand() % (MAP_ROW - (i?3:2)) + (i?2:1);
            x = rand() % (MAP_COL - (i?3:2)) + (i?2:1);
            switch (n)
            {
            case 0: y = 0; break;
            case 1: x = 0; break;
            case 2: x = COL_END; break;
            case 3: y = ROW_END; break;
            case 4:
                while (1) { x = rand() % 30 + 10; y = rand() % 15 + 5; if (map[y][x] == WALL) break; }
            }
            if (map[y][x] == WALL)
            {
                map[y][x] = GATE;
                gatePos.push_back(make_pair(y, x));
                break;
            }
        }
        if (i == 0) gate1 = new Something(y, x, GATE);
        if (i == 1) gate2 = new Something(y, x, GATE);
    }
}

void Stage::appearPlusGate()
{
    int n, y, x;
    for (int i = 0; i < 2; i++)
    {
        while (1)
        {
            n = rand() % (!level ? 4 : 5);
            y = rand() % (MAP_ROW - (i?3:2)) + (i?2:1);
            x = rand() % (MAP_COL - (i?3:2)) + (i?2:1);
            switch (n)
            {
            case 0: y = 0; break;
            case 1: x = 0; break;
            case 2: x = COL_END; break;
            case 3: y = ROW_END; break;
            case 4:
                while (1) { x = rand() % 30 + 10; y = rand() % 15 + 5; if (map[y][x] == WALL && map[y][x] != GATE) break; }
            }
            if(i==0){
                if (map[y][x] == WALL && map[y][x] != GATE )
                {
                    map[y][x] = PLUS_GATE;
                    plusGatePos.push_back(make_pair(y, x));
                    break;
                }
            }
            if(i==1){
               if (map[y][x] == WALL && map[y][x] != GATE && map[y][x] != PLUS_GATE )
                {
                    map[y][x] = PLUS_EXIT;
                    plusGatePos.push_back(make_pair(y, x));
                    break;
                }   
            }
        }
        if (i == 0) plusGate1 = new Something(y, x, PLUS_GATE);
        if (i == 1) plusGate2 = new Something(y, x, PLUS_EXIT);
    }
}

void Stage::disappearGate()
{
    for (auto gate : gatePos)
    {
        if(map[gate.first][gate.second] == GATE) map[gate.first][gate.second] = WALL;
    }
    gatePos.clear();
}

void Stage::disappearPlusGate()
{
    for (auto gate : plusGatePos)
    {
        if(map[gate.first][gate.second] == PLUS_GATE) map[gate.first][gate.second] = WALL;
        if(map[gate.first][gate.second] == PLUS_EXIT) map[gate.first][gate.second] = WALL;
    }
    plusGatePos.clear();
}

void Stage::makeSnake()
{
    stat[0] = 3;
    int row = 13;
    int col = 26;
    Snail = new Something(row, col--, SNAKE_BODY);
    Something *p = new Something(row, col--, SNAKE_BODY);
    Snail->link = p;
    p = new Something(row, col--, SNAKE_HEAD);
    Snail->link->link = p;
    map[Snail->y][Snail->x] = Snail->who;
    p = Snail->link;
    map[p->y][p->x] = p->who;
    p = p->link;
    map[p->y][p->x] = p->who;
    
    itemManager.updateSnakeLengthStat(stat[0]); // 생성 시 초기 길이 주입
}

void Stage::moveSnake()
{
    if (map[Snail->y][Snail->x] != WALL) 
        map[Snail->y][Snail->x] = EMPTY; 
    Something *q = Snail;
    Something *p = q->link;
    while (p->link != NULL) 
    {
        q->x = p->x; q->y = p->y; q = p; p = p->link;
    }
    if (dir == LEFT)        { map[p->y][p->x] = q->who; q->x = p->x; q->y = p->y; p->x--; }
    else if (dir == UP)     { map[p->y][p->x] = q->who; q->x = p->x; q->y = p->y; p->y--; }
    else if (dir == RIGHT)  { map[p->y][p->x] = q->who; q->x = p->x; q->y = p->y; p->x++; }
    else if (dir == DOWN)   { map[p->y][p->x] = q->who; q->x = p->x; q->y = p->y; p->y++; }
    
    if (map[p->y][p->x] == WALL || map[p->y][p->x] == SNAKE_BODY) 
    {
        map[p->y][p->x] = IMMUNE_WALL;
        gameOver();
    }
    if (map[p->y][p->x] == GATE)      enterGate(p);
    if (map[p->y][p->x] == PLUS_GATE)  enterPlusGate(p);
    if (map[p->y][p->x] == PLUS_EXIT)  enterPlusGate(p);
    
    // 7. 아이템 상호작용 이관 처리 영역
    if (map[p->y][p->x] == GROWTH_ITEM || map[p->y][p->x] == POISON_ITEM || map[p->y][p->x] == SPEED_SLOW)
    {
        int targetItem = map[p->y][p->x];
        itemManager.eatItem(targetItem, stat[0], timeoutMs); // 점수 및 속도 반영
        
        // 실제 물리적인 노드 증감 처리
        if (targetItem == GROWTH_ITEM && stat[0] < 10) {
            Something *newNode = new Something(Snail->y, Snail->x, SNAKE_BODY);
            if (Snail->x - Snail->link->x == 1)       newNode->x++;
            else if (Snail->y - Snail->link->y == 1)  newNode->y++;
            else if (Snail->x - Snail->link->x == -1) newNode->x--;
            else if (Snail->y - Snail->link->y == -1) newNode->y--;
            newNode->link = Snail; Snail = newNode;
            if (map[Snail->y][Snail->x] != WALL) map[Snail->y][Snail->x] = Snail->who;
        } 
        else if (targetItem == POISON_ITEM) {
            map[Snail->y][Snail->x] = EMPTY;
            Snail = Snail->link;
        }
    }
    map[p->y][p->x] = p->who;
}

void Stage::enterGate(Something *head)
{
    chkEnter = TRUE;
    // ... (기존 변함없는 게이트 워프 코드는 가독성을 위해 중략 유지) ...
    itemManager.incrementGateStat(); // 8. 게이트 누적 수치를 매니저 객체에 통보
}

void Stage::enterPlusGate(Something *head)
{
    chkPlusEnter = TRUE;
    // ... (기존 변함없는 플러스 게이트 워프 코드는 중략 유지) ...
    itemManager.incrementGateStat(); 
}

int Stage::findRoot(Something *gate)
{
    for (int i = 0; i < 4; i++)
    {
        if (dir == LEFT)        { if (map[gate->y][gate->x - 1] == EMPTY) return dir; else dir = KEY_UP; }
        else if (dir == KEY_UP) { if (map[gate->y - 1][gate->x] == EMPTY) return dir; else dir = RIGHT; }
        else if (dir == RIGHT)  { if (map[gate->y][gate->x + 1] == EMPTY) return dir; else dir = DOWN; }
        else if (dir == DOWN)   { if (map[gate->y + 1][gate->x] == EMPTY) return dir; else dir = LEFT; }
    }
    return dir;
}

void Stage::gameOver() { finish = true; }

void Stage::alert(int posY, int posX, const string msg, bool stopFlag)
{
    WINDOW *alert = newwin(7, msg.length() * 2, posY, posX);
    box(alert, 0, 0);
    wattron(alert, COLOR_PAIR(0));
    wbkgd(alert, COLOR_PAIR(2));
    mvwprintw(alert, 3, msg.length() / 2, msg.c_str());
    wrefresh(alert);
    if (!stopFlag) usleep(1750000);
}