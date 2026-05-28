#include "GameManager.h"
#include <unistd.h>

GameManager::GameManager() {
    srand((unsigned)time(0)); 
    initscr();                
    keypad(stdscr, TRUE);     
    cbreak();                 
    noecho();                 

    start_color();             
    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    if (init_color(COLOR_BLUE, 0, 0, 300) == ERR) {
        printw("Your terminal cannot change the color definitions\n");
        printw("press any key to continue...\n");
        getch(); 
    }

    // 메뉴와 도움말 등에서 사용할 기본 색상
    init_pair(0, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    init_pair(10, COLOR_YELLOW, COLOR_BLACK);
    init_pair(11, COLOR_BLACK, COLOR_WHITE);

    menuLastFocus = 0;
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
}

GameManager::~GameManager() {
    endwin();
}

void GameManager::screenLock() {
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

string GameManager::menu() {
    clear();
    screenLock();
    curs_set(0);
    string txt[4];
    txt[0] = "[ SNAKE GAME ]";
    int focus = menuLastFocus;
    while (1) {
        if (!focus) focus = 300;
        txt[1] = "PLAY";
        txt[2] = "HELP";
        txt[3] = "EXIT";
        
        attron(COLOR_PAIR(10));
        mvprintw(y / 2 - 2, x / 2 - txt[0].length() / 2, txt[0].c_str());
        attroff(COLOR_PAIR(10));
        
        for (int i = 1; i < 4; i++) {
            if (i == abs(focus % 4 + 1)) {
                attron(COLOR_PAIR(11));
                mvprintw(y / 2 + i, x / 2 - (txt[i].length() / 2), txt[i].c_str());
                attroff(COLOR_PAIR(11));
            }
            else {
                mvprintw(y / 2 + i, x / 2 - (txt[i].length() / 2), txt[i].c_str());
            }
        }
        
        switch (getch()) {
        case KEY_UP: focus--; break;
        case KEY_DOWN: focus++; break;
        case 10: // ENTER
            menuLastFocus = focus;
            return txt[abs(focus % 4 + 1)];
        }
    }
    return "";
}

void GameManager::help() {
    screenLock();
    int ySize = 0, yScroll = 0;
    while (1) {
        manual = newwin(sizeY, sizeX, startY, startX);
        description = newwin(desSizeY, desSizeX, desStartY, desStartX);
        scrollBar = newwin(scrollBarLen, 2, desStartY + yScroll, startX + sizeX - 6);
        
        wattron(manual, COLOR_PAIR(10));
        box(manual, 0, 0);
        mvwprintw(manual, 0, sizeX / 2 - manualTitle.length() / 2, "%s", manualTitle.c_str());
        wattroff(manual, COLOR_PAIR(10));

        mvwprintw(description, 0 + ySize, sizeX / 2 - menuTitle.length() / 2 - 3, "%s", menuTitle.c_str());
        for (int i = 0; i < 3; i++)
            mvwprintw(description, 2 + (i * 2) + ySize, sizeX / 2 - menuTxt[2].length() / 2 - 3, "%s", menuTxt[i].c_str());

        mvwprintw(description, 11 + ySize, sizeX / 2 - shorTitle.length() / 2 - 3, "%s", shorTitle.c_str());
        for (int i = 0; i < 7; i++)
            mvwprintw(description, 13 + (i * 2) + ySize, sizeX / 2 - shorTxt[6].length() / 2 - 3, "%s", shorTxt[i].c_str());

        if (txtLines >= desSizeY) {
            wattron(scrollBar, COLOR_PAIR(10));
            box(scrollBar, 0, 0);
            wattroff(scrollBar, COLOR_PAIR(10));
        }
        
        refresh();
        wrefresh(manual);
        wrefresh(description);
        wrefresh(scrollBar);
        
    RE:
        switch (getch()) {
        case KEY_UP:
            if (yScroll) yScroll--; else goto RE;
            if (ySize) ySize++;
            break;
        case KEY_DOWN:
            if (yScroll < desSizeY - scrollBarLen) yScroll++; else goto RE;
            if (ySize > desSizeY - txtLines && txtLines > desSizeY) ySize--;
            break;
        case 27: // ESC
            delwin(scrollBar);
            delwin(description);
            delwin(manual);
            return;
        }
    }
}