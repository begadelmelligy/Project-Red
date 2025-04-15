#include "game.h"

int main(void)
{

    Game *game = (Game *)calloc(1, sizeof(Game)); // like malloc but init with 0
    if (!game)
    {
        fprintf(stderr, "Failed to allocate memory for game\n");
        return 1;
    }

    init_game(game);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime(); // delta time

        handle_input(game);
        update_game(game, dt);
        draw_game(game);
    }
    clean_up(game);
    free(game);
    CloseWindow();

    return 0;
}