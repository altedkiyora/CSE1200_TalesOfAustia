#pragma once
// ============================================================
// PART 2 of 3 — GameLogic.hpp
// Asset loading, audio helpers, fight/transition control,
// walkable-border collision and input handling, fixedUpdate().
// Depends on GameState.hpp (included by iMain.cpp first).
// ============================================================

#include "GameState.hpp"

// ----------------------------------------------------
void loadAssets() {
    img_main = iLoadImage("Images/main.jpg");
    img_1stScene = iLoadImage("Images/1stScene.jpg");
    img_floor_even = iLoadImage("Images/floor_even_closed.jpg");
    img_floor_odd = iLoadImage("Images/floor_odd_opened.jpg");
    img_penfound = iLoadImage("Images/penfound.jpg");
    img_mainground = iLoadImage("Images/2d view/mainground.jpg");

    char buf[100];
    for (int i = 1; i <= 4; i++) {
        sprintf(buf, "Images/playerMove/walk down/walkDown%d.png", i);
        img_walkDown[i - 1] = iLoadImage(buf);
        sprintf(buf, "Images/playerMove/walkup/walkUp%d.png", i);
        img_walkUp[i - 1] = iLoadImage(buf);
        sprintf(buf, "Images/playerMove/walk side left/left%d.png", i);
        img_walkLeft[i - 1] = iLoadImage(buf);
        sprintf(buf, "Images/playerMove/walk side right/right%d.png", i);
        img_walkRight[i - 1] = iLoadImage(buf);
    }

    for(int i=1; i<=7; i++) {
        sprintf(buf, "Images/2d view/playerSlide/slide%d.png", i);
        img_slide[i-1] = iLoadImage(buf);
    }
    img_slideIdle = iLoadImage("Images/2d view/playerSlide/sprites.png");

    for(int i=1; i<=6; i++) {
        sprintf(buf, "Images/characters/seniorIdle/senior%d.png", i);
        img_senior[i-1] = iLoadImage(buf);
    }

    for(int i=1; i<=8; i++) {
        sprintf(buf, "Images/characters/cr/cr%d.png", i);
        img_cr[i-1] = iLoadImage(buf);
    }

    // fight projectiles (newProjectile set, native 64x64, white-keyed)
    for(int i=1; i<=11; i++) {
        sprintf(buf, "Images/2d view/newProjectile/proj%d.png", i);
        img_projectile[i-1] = iLoadImage(buf);
    }
}

// ----------------------------------------------------
void playSound(const char* name, bool loop = false) {
    if (!soundOn) return;
    char command[256];
    sprintf(command, "play %s %s", name, loop ? "repeat" : "from 0");
    mciSendString(command, NULL, 0, NULL);
}

void openSounds() {
    // Aliases mapped to the actual files shipped in the Audios/ folder.
    // WAVs must not use "type mpegvideo"; MCI infers the device from the extension.
    mciSendString("open \"Audios/mp3forMain_TabooMelody.wav\" alias bgm", NULL, 0, NULL);
    mciSendString("open \"Audios/mp3forCheckpoint1.wav\" alias click", NULL, 0, NULL);
    mciSendString("open \"Audios/rbd.wav\" alias rbd", NULL, 0, NULL);
    mciSendString("open \"Audios/mp3forGame_AURAMONSTERARRIVES.wav\" alias fight", NULL, 0, NULL);
}

// ----------------------------------------------------
void resetFight() {
    fightTimer = 30;
    fightTick = 0;
    fightAccMs = 0;
    fightLastMs = GetTickCount();
    fightPlayerX = SCREEN_WIDTH / 2;
    fightPlayerFrame = 0;
    for(int i=0; i<MAX_PROJ; i++) {
        projectiles[i].active = false;
    }
    playSound("fight", true);
}

void startTransition(int nextState) {
    gameState = STATE_TRANSITION;
    nextStateAfterTransition = nextState;
    transitionTimer = 0;
    transitionStartMs = GetTickCount();
    mciSendString("stop bgm", NULL, 0, NULL);
    playSound("rbd", false);
}

// ----------------------------------------------------
// Walkable borders. All coordinates use the scene's bottom-left as (0,0),
// same origin iGraphics draws with. The player anchor (sprite bottom-left
// corner, 64x64) must stay inside the border polygon of the active scene.
static const double POLY_1ST[][2] = {
    {77,141},{862,141},{746,444},{752,512},{234,512},{232,444}
};
static const double POLY_FLOOR[][2] = {
    {75,191},{114,797},{403,797},{314,352},{1044,352},{1047,191}
};
// 2d side-view fight map: plain border rectangle 1031 wide x 245 tall.
#define FIGHT_BORDER_W 1031
#define FIGHT_BORDER_H 245

// Even-odd ray casting: true if (px,py) is inside the closed polygon.
static bool pointInPoly(double px, double py, const double poly[][2], int n) {
    bool inside = false;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        double xi = poly[i][0], yi = poly[i][1];
        double xj = poly[j][0], yj = poly[j][1];
        if (((yi > py) != (yj > py)) &&
            (px < (xj - xi) * (py - yi) / (yj - yi) + xi))
            inside = !inside;
    }
    return inside;
}

static bool walkableAt(double x, double y) {
    if (gameState == STATE_FIRST_SCENE)
        return pointInPoly(x, y, POLY_1ST, 6);
    if (gameState == STATE_FIGHT_ONLAIN)
        return x >= 0 && x + 64 <= FIGHT_BORDER_W && y >= 0 && y + 64 <= FIGHT_BORDER_H;
    if (gameState >= STATE_FLOOR_SENIOR && gameState <= STATE_FLOOR_PENFOUND)
        return pointInPoly(x, y, POLY_FLOOR, 6);
    return true;
}

bool playerBlockedAt(double nx, double ny) {
    const double S = 64;
    if (gameState == STATE_FLOOR_SENIOR &&
        nx < seniorX + S && nx + S > seniorX &&
        ny < seniorY + S && ny + S > seniorY) return true;
    if (gameState == STATE_FLOOR_CR &&
        nx < crX + S && nx + S > crX &&
        ny < crY + S && ny + S > crY) return true;
    return false;
}

void handleInput() {
    // --- Menu confirm (ENTER) ---
    bool enterNow = (isKeyPressed('\r') || isKeyPressed('\n')) != 0;
    if (enterNow && !enterWasDown && gameState == STATE_MENU) {
        gameState = STATE_FIRST_SCENE;
        playerX = 512; playerY = 170;
        showDialogue = false;
    }
    enterWasDown = enterNow;

    // --- Interact (H), edge-triggered ---
    bool hNow = (isKeyPressed('h') || isKeyPressed('H')) != 0;
    if (hNow && !hWasDown) {
        if (gameState == STATE_FIRST_SCENE) {
            if (playerY > 400 && playerX > 400 && playerX < 600) {
                startTransition(STATE_FLOOR_SENIOR);
                playerX = 900; playerY = 200;
            } else {
                strcpy(dialogueText, "I need to get closer to the center stairs.");
                showDialogue = true;
            }
        }
        else if (gameState == STATE_FLOOR_SENIOR) {
            double dist = getEuclideanDistance(playerX, seniorX, playerY, seniorY);
            if (dist < 100) {
                if (!showDialogue) {
                    strcpy(dialogueText, "Senior: tor intro de... (Press H to continue)");
                    showDialogue = true;
                } else {
                    showDialogue = false;
                    startTransition(STATE_FLOOR_CR);
                    playerX = 900; playerY = 200;
                }
            }
        }
        else if (gameState == STATE_FLOOR_CR) {
            double dist = getEuclideanDistance(playerX, crX, playerY, crY);
            if (dist < 100) {
                if (!showDialogue) {
                    strcpy(dialogueText, "CR: You are late! Fight me! ONLAIN!");
                    showDialogue = true;
                } else {
                    showDialogue = false;
                    mciSendString("stop bgm", NULL, 0, NULL);
                    gameState = STATE_FIGHT_ONLAIN;
                    resetFight();
                }
            }
        }
    }
    hWasDown = hNow;

    // --- Fight movement (A/D or Left/Right): slide while held ---
    if (gameState == STATE_FIGHT_ONLAIN) {
        bool slidingNow = false;
        if (isKeyPressed('a') || isKeyPressed('A') || isSpecialKeyPressed(GLUT_KEY_LEFT)) { fightPlayerX -= DODGE_STEP; slidingNow = true; }
        if (isKeyPressed('d') || isKeyPressed('D') || isSpecialKeyPressed(GLUT_KEY_RIGHT)) { fightPlayerX += DODGE_STEP; slidingNow = true; }
        if (fightPlayerX < 0) fightPlayerX = 0;
        if (fightPlayerX > FIGHT_BORDER_W - 64) fightPlayerX = FIGHT_BORDER_W - 64;
        isSliding = slidingNow;
    }
    // --- Explore movement (WASD or Arrows) in gameplay states only ---
    else if (gameState == STATE_FIRST_SCENE ||
             (gameState >= STATE_FLOOR_SENIOR && gameState <= STATE_FLOOR_PENFOUND &&
              gameState != STATE_FIGHT_ONLAIN)) {
        bool movingNow = false;

        double dx = 0, dy = 0;
        if (isKeyPressed('w') || isKeyPressed('W') || isSpecialKeyPressed(GLUT_KEY_UP)) { dy += MOVE_STEP; playerDir = 1; movingNow = true; }
        if (isKeyPressed('s') || isKeyPressed('S') || isSpecialKeyPressed(GLUT_KEY_DOWN)) { dy -= MOVE_STEP; playerDir = 0; movingNow = true; }
        if (isKeyPressed('a') || isKeyPressed('A') || isSpecialKeyPressed(GLUT_KEY_LEFT)) { dx -= MOVE_STEP; playerDir = 2; movingNow = true; }
        if (isKeyPressed('d') || isKeyPressed('D') || isSpecialKeyPressed(GLUT_KEY_RIGHT)) { dx += MOVE_STEP; playerDir = 3; movingNow = true; }

        double nx = playerX + dx, ny = playerY + dy;

        // Polygon border + NPC blocking, axis-separated so the player
        // slides along walls instead of sticking.
        if (walkableAt(nx, ny) && !playerBlockedAt(nx, ny)) {
            playerX = nx; playerY = ny;
        } else {
            if (dx != 0 && walkableAt(nx, playerY) && !playerBlockedAt(nx, playerY)) playerX = nx;
            if (dy != 0 && walkableAt(playerX, ny) && !playerBlockedAt(playerX, ny)) playerY = ny;
        }

        isMoving = movingNow;

        // Floor exit: top-left of NOCR leads to PENFOUND.
        if ((gameState == STATE_FLOOR_NOCR || gameState == STATE_FLOOR_PENFOUND)
            && playerX < 200 && playerY > SCREEN_HEIGHT - 300) {
            if (gameState == STATE_FLOOR_NOCR) {
                startTransition(STATE_FLOOR_PENFOUND);
                playerX = 900; playerY = 200;
            }
        }
    }
}

// ----------------------------------------------------
void fixedUpdate() {
    handleInput();

    if (gameState == STATE_TRANSITION) {
        transitionTimer++;
        if (GetTickCount() - transitionStartMs >= 2000) { // 2 real seconds
            gameState = nextStateAfterTransition;
            if (gameState != STATE_FIGHT_ONLAIN) {
                playSound("bgm", true);
            }
        }
        return;
    }

    if (gameState == STATE_FIRST_SCENE || gameState >= STATE_FLOOR_SENIOR) {
        animTick++;
        bool advFrame = (animTick % 3 == 0);

        if (isMoving) {
            if (advFrame) playerFrame = (playerFrame + 1) % 4;
        } else {
            playerFrame = 0;
        }

        if (advFrame) {
            seniorFrame = (seniorFrame + 1) % 6;
            crFrame = (crFrame + 1) % 8;
        }

        // isMoving/isSliding are recomputed every tick in handleInput(),
        // so animation frames stop automatically when no key is held.
    }

    if (gameState == STATE_FIGHT_ONLAIN) {
        animTick++;
        bool advFrame = (animTick % 3 == 0);

        if (isSliding) {
            if (advFrame) fightPlayerFrame = (fightPlayerFrame + 1) % 7;
        } else {
            fightPlayerFrame = 0;
        }

        // Each projectile is ONE object animating through the folder's
        // image sequence (proj0..proj10) while it falls; all projectiles
        // advance in sync on animation ticks.
        if (advFrame) {
            for(int i=0; i<MAX_PROJ; i++) {
                if(projectiles[i].active)
                    projectiles[i].frame = (projectiles[i].frame + 1) % 11;
            }
        }

        if (rand() % 5 == 0) {
            for(int i=0; i<MAX_PROJ; i++) {
                if(!projectiles[i].active) {
                    projectiles[i].active = true;
                    projectiles[i].x = rand() % (SCREEN_WIDTH - 64);
                    projectiles[i].y = SCREEN_HEIGHT;
                    projectiles[i].frame = 0;
                    break;
                }
            }
        }

        for(int i=0; i<MAX_PROJ; i++) {
            if(projectiles[i].active) {
                projectiles[i].y -= (4 + rand() % 4);
                if(projectiles[i].y < 100) {
                    projectiles[i].active = false;
                }
            }
        }

        unsigned long nowMs = GetTickCount();
        if (nowMs >= fightLastMs) fightAccMs += nowMs - fightLastMs;
        fightLastMs = nowMs;
        while (fightAccMs >= 1000) {
            fightAccMs -= 1000;
            fightTimer--;
        }
        if (fightTimer <= 0) {
            mciSendString("stop fight", NULL, 0, NULL);
            startTransition(STATE_FLOOR_NOCR);
            playerX = 900; playerY = 200;
        }
    }
}
