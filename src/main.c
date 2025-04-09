#include "game.h"

int main(void)
{
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "Mars Restaurant Empire");
    SetTargetFPS(60);

    init_game();

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime(); // delta time

        handle_input();
        update_game(dt);
        draw_game();
    }
    clean_up();
    CloseWindow();

    return 0;
}