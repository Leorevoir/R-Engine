#include "raylib.h"

#include <math.h>

int main(void) {

  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "raylib example");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawRectangle(-6000, 320, 13000, 8000, DARKGRAY);
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
