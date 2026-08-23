#pragma once
// ============================================================
// PART 1 of 3 — GameState.hpp
// All global state: game-state enum, asset handles, player/NPC/
// fight/transition/audio/UI variables, projectile pool.
// Included once by iMain.cpp BEFORE the other two parts.
// ============================================================

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

// Input edge-detection flags (rising-edge one-shot keys)
bool enterWasDown = false;
bool hWasDown = false;
