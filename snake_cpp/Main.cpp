#include "GameManager.h"
#include "Stage.h" 

int main()
{
    // 이제 GameManager가 전체적인 프로그램 흐름과 UI를 통제합니다.
    GameManager gm;
    string game;
    
    while (1) {
        game = gm.menu();
        clear();

        if (game == "PLAY") {
            // 게임이 시작될 때만 Stage 객체를 생성하여 플레이
            Stage view;
            view.play();
        }
        else if (game == "HELP") {
            gm.help();
        }
        else {
            break; // EXIT
        }
    }
    return 0;
}