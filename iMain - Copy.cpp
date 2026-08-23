#define _CRT_SECURE_NO_WARNINGS
// ============================================================
// Tales of Austia — entry point (modular build)
// The original monolithic source is preserved as iMain_backup.cpp.
// This file only wires the three parts together, in dependency
// order: State (globals) -> Logic (functions) -> Render (callbacks).
// ============================================================
#include "iGraphics.h"
#include "Utils.hpp"
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "GameState.hpp"
#include "GameLogic.hpp"
#include "GameRender.hpp"

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
