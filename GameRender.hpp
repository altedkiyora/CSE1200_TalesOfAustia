#pragma once
// ============================================================
// PART 3 of 3 — GameRender.hpp
// All iGraphics framework callbacks: iDraw, mouse handlers and
// the legacy keyboard stubs kept for framework compatibility.
// Depends on GameState.hpp + GameLogic.hpp (included first).
// ============================================================

#include "GameState.hpp"
#include "GameLogic.hpp"

// ----------------------------------------------------
void iDraw() {
    iClear();

    if (gameState == STATE_MENU) {
        iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, img_main);
        iSetColor(255, 255, 255);
        iText(SCREEN_WIDTH / 2 - 100, 50, "Press ENTER to Start", GLUT_BITMAP_TIMES_ROMAN_24);
    }
    else if (gameState == STATE_ABOUT) {
        iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, img_main);
        iSetColor(0, 0, 0);
        iFilledRectangle(200, 200, 624, 386);
        iSetColor(255, 255, 255);
        iText(250, 500, "About Tales of Austia", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(250, 450, "A dark fantasy adventure game.", GLUT_BITMAP_HELVETICA_18);
        iText(250, 250, "Click anywhere to go back.", GLUT_BITMAP_HELVETICA_18);
    }
    else if (gameState == STATE_INSTRUCTIONS) {
        iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, img_main);
        iSetColor(0, 0, 0);
        iFilledRectangle(200, 200, 624, 386);
        iSetColor(255, 255, 255);
        iText(250, 500, "Instructions", GLUT_BITMAP_TIMES_ROMAN_24);
        iText(250, 450, "Use Arrow Keys or WASD to move.", GLUT_BITMAP_HELVETICA_18);
        iText(250, 420, "Press 'H' to interact with objects and characters.", GLUT_BITMAP_HELVETICA_18);
        iText(250, 250, "Click anywhere to go back.", GLUT_BITMAP_HELVETICA_18);
    }
    else if (gameState == STATE_FIRST_SCENE) {
        iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, img_1stScene);

        int img = img_walkDown[0];
        if (playerDir == 0) img = img_walkDown[playerFrame];
        else if (playerDir == 1) img = img_walkUp[playerFrame];
        else if (playerDir == 2) img = img_walkLeft[playerFrame];
        else if (playerDir == 3) img = img_walkRight[playerFrame];
        iShowImage(playerX, playerY, 64, 64, img);

        if (showDialogue) {
            iSetColor(0, 0, 0);
            iFilledRectangle(0, 0, SCREEN_WIDTH, 100);
            iSetColor(255, 255, 255);
            iText(50, 50, dialogueText, GLUT_BITMAP_TIMES_ROMAN_24);
        } else {
            iSetColor(255,255,255);
            iText(10, 700, "Walk up the stairs and press H near the center.", GLUT_BITMAP_HELVETICA_18);
        }
    }
    else if (gameState == STATE_TRANSITION) {
        iSetColor(0, 0, 0);
        iFilledRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        iSetColor(255, 255, 255);
        if (transitionTimer > 10) {
            iText(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2, "Entering the Void...", GLUT_BITMAP_TIMES_ROMAN_24);
        }
    }
    else if (gameState >= STATE_FLOOR_SENIOR && gameState <= STATE_FLOOR_PENFOUND && gameState != STATE_FIGHT_ONLAIN) {

        if (gameState == STATE_FLOOR_SENIOR) iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, img_floor_even);
        else if (gameState == STATE_FLOOR_CR) iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, img_floor_odd);
        else if (gameState == STATE_FLOOR_NOCR) iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, img_floor_even);
        else if (gameState == STATE_FLOOR_PENFOUND) iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, img_floor_odd);

        if (gameState == STATE_FLOOR_SENIOR) {
            iShowImage(seniorX, seniorY, 64, 64, img_senior[seniorFrame]);
        }
        if (gameState == STATE_FLOOR_CR) {
            iShowImage(crX, crY, 64, 64, img_cr[crFrame]);
        }

        int img = img_walkDown[0];
        if (playerDir == 0) img = img_walkDown[playerFrame];
        else if (playerDir == 1) img = img_walkUp[playerFrame];
        else if (playerDir == 2) img = img_walkLeft[playerFrame];
        else if (playerDir == 3) img = img_walkRight[playerFrame];
        iShowImage(playerX, playerY, 64, 64, img);

        if (showDialogue) {
            iSetColor(0, 0, 0);
            iFilledRectangle(0, 0, SCREEN_WIDTH, 100);
            iSetColor(255, 255, 255);
            iText(50, 50, dialogueText, GLUT_BITMAP_TIMES_ROMAN_24);
        } else {
            iSetColor(255,255,255);
            if (gameState == STATE_FLOOR_PENFOUND) {
                iText(10, 700, "You found the pen! End of Chapter 1.", GLUT_BITMAP_HELVETICA_18);
            } else if (gameState == STATE_FLOOR_NOCR) {
                iText(10, 700, "Move to the top-left to proceed.", GLUT_BITMAP_HELVETICA_18);
            }
        }
    }
    else if (gameState == STATE_FIGHT_ONLAIN) {
        iShowImage(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, img_mainground);

        for(int i=0; i<MAX_PROJ; i++) {
            if(projectiles[i].active) {
                iShowImage(projectiles[i].x, projectiles[i].y, 64, 64, img_projectile[projectiles[i].frame]);
            }
        }

        int pImg = isSliding ? img_slide[fightPlayerFrame] : img_slideIdle;
        iShowImage(fightPlayerX, 100, 64, 64, pImg);

        iSetColor(255,255,255);
        char buf[50];
        sprintf(buf, "Time Left: %d", fightTimer);
        iText(50, 700, buf, GLUT_BITMAP_TIMES_ROMAN_24);
        iText(50, 660, "Survive! Use Left/Right Arrow Keys to dodge.", GLUT_BITMAP_HELVETICA_18);
    }
}

// ----------------------------------------------------
void iMouseMove(int mx, int my) {}
void iPassiveMouseMove(int mx, int my) {}

void iMouse(int button, int state, int mx, int my) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        playSound("click");

        if (gameState == STATE_MENU) {
            if (my >= 480 && my <= 560 && mx >= 360 && mx <= 660) {
                gameState = STATE_FIRST_SCENE;
                playerX = 512; playerY = 170;
                showDialogue = false;
            }
            else if (my >= 350 && my <= 430 && mx >= 360 && mx <= 660) {
                gameState = STATE_ABOUT;
            }
            else if (my >= 220 && my <= 300 && mx >= 360 && mx <= 660) {
                gameState = STATE_INSTRUCTIONS;
            }
            else if (my >= 50 && my <= 150 && mx >= 850 && mx <= 950) {
                soundOn = !soundOn;
                if(soundOn) playSound("bgm", true);
                else mciSendString("stop bgm", NULL, 0, NULL);
            }
        }
        else if (gameState == STATE_ABOUT || gameState == STATE_INSTRUCTIONS) {
            gameState = STATE_MENU;
        }
    }
}

// Kept for framework compatibility; iGraphics v4 does not invoke these.
// All keyboard logic lives in handleInput() in GameLogic.hpp.
void iKeyboard(unsigned char key) {}
void iSpecialKeyboard(unsigned char key) {}
void iSpecialKeyboardUp(unsigned char key) {}
