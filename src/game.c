#include "game.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

GameState game_state = {0};
World world = {0};
BuildingTemplate building_templates[3] = {0};

void init_game(void)
{
    // Seed random number generator
    srand(time(NULL));

    game_state.current_view = VIEW_WORLD_MAP;
    game_state.current_city_id = 0;
    game_state.selected_plot_id = UINT32_MAX;     // Invalid ID
    game_state.selected_building_id = UINT32_MAX; // Invalid ID
    game_state.time_scale = 1.0f;

    init_building_templates();
    init_world();
    init_player();
    init_skills();
}

void handle_input() {};
void update_game(float dt) {};
void draw_game() {};
void clean_up() {};

void init_building_templates() {};
void init_world() {};
void init_player() {};
void init_skills() {};
