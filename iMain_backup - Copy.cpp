#define _CRT_SECURE_NO_WARNINGS
#include "iGraphics.h"
#include "Utils.hpp"
#include <time.h>
#include <stdlib.h>
#include <string.h>

// --- Game States ---
enum GameState {
    STATE_MENU = 0,
    STATE_ABOUT = 1,
    STATE_INSTRUCTIONS = 2,
    STATE_FIRST_SCENE = 3,
    STATE_TRANSITION = 4,
    STATE_FLOOR_SENIOR = 5,
    STATE_FLOOR_CR = 6,
    STATE_FIGHT_ONLAIN = 7,
    STATE_FLOOR_NOCR = 8,
    STATE_FLOOR_PENFOUND = 9
};

int gameState = STATE_MENU;
int nextStateAfterTransition = STATE_FLOOR_SENIOR;

// --- Assets ---
int img_main, img_1stScene, img_floor_even, img_floor_odd, img_penfound, img_mainground;
int img_walkDown[4], img_walkUp[4], img_walkLeft[4], img_walkRight[4];
int img_slide[7], img_slideIdle;
int img_senior[6];
int img_cr[8];
int img_projectile[11];

// --- Variables ---
// Player Top-Down
double playerX = 800, playerY = 100;
int playerDir = 2; // 0=Down, 1=Up, 2=Left, 3=Right
int playerFrame = 0;
bool isMoving = false;
int animTick = 0;

const double MOVE_STEP = 3.75;
const double DODGE_STEP = 6.25;

// NPCs
double seniorX = 300, seniorY = 300;
int seniorFrame = 0;
double crX = 300, crY = 300;
int crFrame = 0;

// Fight
double fightPlayerX = 500;
int fightPlayerFrame = 0;
bool isSliding = false;
int fightTimer = 30;
int fightTick = 0;
unsigned long fightAccMs = 0;
unsigned long fightLastMs = 0;

struct Projectile {
    double x, y;
    bool active;
    int frame;
};
#define MAX_PROJ 10
Projectile projectiles[MAX_PROJ];

// Transition
int transitionTimer = 0;
unsigned long transitionStartMs = 0;

// Audio
bool soundOn = true;

// UI State
bool showDialogue = false;
char dialogueText[200] = "";

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

// ----------------------------------------------------
// INPUT (iGraphics v4.0):
// v4 never calls iKeyboard()/iSpecialKeyboard(); its handlers only record
// key state into keyPressed[]/specialKeyPressed[]. All input is therefore
// polled here via isKeyPressed()/isSpecialKeyPressed() once per tick,
// called from fixedUpdate(). One-shot keys use rising-edge detection.
bool enterWasDown = false;
bool hWasDown = false;

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

// Kept for framework compatibility; iGraphics v4 does not invoke these.
// All keyboard logic lives in handleInput() above.
void iKeyboard(unsigned char key) {}
void iSpecialKeyboard(unsigned char key) {}
void iSpecialKeyboardUp(unsigned char key) {}

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

// ----------------------------------------------------
int main() {
    srand((unsigned)time(NULL));
    openSounds();
    playSound("bgm", true);
    iInitialize(SCREEN_WIDTH, SCREEN_HEIGHT, "Tales of Austia");
    loadAssets();
    iSetTimer(50, fixedUpdate);
    iStart();
    return 0;
}