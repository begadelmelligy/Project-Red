#include "game.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

void init_game(Game *game)
{
    // Initialize Raylib window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Project Red");
    SetTargetFPS(60);

    // initialize the arena
    game->arena.size = 1024 * 1024; // Example: 1MB
    game->arena.base = (char *)malloc(game->arena.size);
    if (!game->arena.base)
    {
        fprintf(stderr, "Failed to allocate arena\n");
        exit(1);
    }
    game->arena.current = game->arena.base;
    game->arena.used = 0;
    /* ======================================== */

    // initialize the arena
    game->camera.offset.x = SCREEN_WIDTH / 2.0f;
    game->camera.offset.y = SCREEN_HEIGHT / 2.0f;
    game->camera.rotation = 0.0f;
    game->camera.target.x = 0;
    game->camera.target.y = 0;
    game->camera.zoom = 1.0f;
    /* ======================================== */

    // Load assets
    game->data.assets.customer_eating_texture = LoadTexture("assets/customer/customer_eating.png");
    game->data.assets.customer_idel_texture = LoadTexture("assets/customer/customer_idel.png");
    game->data.assets.customer_walking_texture = LoadTexture("assets/customer/customer_walking.png");

    game->data.assets.large_restaurant_texture = LoadTexture("assets/buildings/large_restaurant.png");
    game->data.assets.medium_restaurant_texture = LoadTexture("assets/buildings/medium_restaurant.png");
    game->data.assets.small_restaurant_texture = LoadTexture("assets/buildings/small_restaurant.png");

    game->data.assets.cities_texture[0] = LoadTexture("assets/cities/city_map_1.png");
    game->data.assets.cities_texture[1] = LoadTexture("assets/cities/city_map_2.png");
    game->data.assets.cities_texture[2] = LoadTexture("assets/cities/city_map_3.png");
    game->data.assets.cities_texture[3] = LoadTexture("assets/cities/city_map_4.png");

    game->data.assets.intro_texture = LoadTexture("assets/red_intro.png"); // Load intro texture
    /* ======================================== */

    // Initialize building templates
    game->data.building_templates[BUILDING_RESTAURANT_SMALL] = (BuildingTemplate){
        .type = BUILDING_RESTAURANT_SMALL,
        .name = "Small Restaurant",
        .staff_capacity = 5,
        .base_cost = 5000,
        .maintenance_cost = 500,
        .size = (Vector2){64, 64},
        .texture = game->data.assets.small_restaurant_texture};

    game->data.building_templates[BUILDING_RESTAURANT_MEDIUM] = (BuildingTemplate){
        .type = BUILDING_RESTAURANT_MEDIUM,
        .name = "Medium Restaurant",
        .staff_capacity = 15,
        .base_cost = 15000,
        .maintenance_cost = 1000,
        .size = (Vector2){128, 128},
        .texture = game->data.assets.medium_restaurant_texture};

    game->data.building_templates[BUILDING_RESTAURANT_LARGE] = (BuildingTemplate){
        .type = BUILDING_RESTAURANT_LARGE,
        .name = "Large Restaurant",
        .staff_capacity = 30,
        .base_cost = 30000,
        .maintenance_cost = 2000,
        .size = (Vector2){256, 256},
        .texture = game->data.assets.large_restaurant_texture};
    /* ======================================== */

    // Initialize player data
    game->data.player.current_exp = 0;
    game->data.player.exp_to_next_level = 1000;
    game->data.player.level = 1;
    game->data.player.net_worth = 10000;
    game->data.player.num_owned_plots = 0;
    game->data.player.skill_points_available = 1;
    game->data.player.skill_points_spent = 0;
    Skill skills[SKILL_COUNT] = {
        {
            .id = SKILL_RARITY_CHANCE_1,
            .name = "Mr Lucky",
            .description = "Increase rarity chance by 20%",
            .cost = 1,
            .is_unlocked = false,
        },

        {
            .id = SKILL_REDUCED_ROLL_COST_1,
            .name = "Talent Hunter",
            .description = "Reduce staff recruitment cost by 20%",
            .cost = 2,
            .is_unlocked = false,
        },

        {
            .id = SKILL_FASTER_COOKING_1,
            .name = "Master Chief",
            .description = "Increase cook efficiency by 20%",
            .cost = 3,
            .is_unlocked = false,
        },

        {
            .id = SKILL_GUARDS_ATTACK_1,
            .name = "Cobra Forces",
            .description = "Increase guard attack by 20%",
            .cost = 2,
            .is_unlocked = false,
        },

        {
            .id = SKILL_BUILDINGS_HEALTH_1,
            .name = "Concrete Building",
            .description = "Increase building health by 20%",
            .cost = 1,
            .is_unlocked = false,
        }};

    for (int i = 0; i < SKILL_COUNT; i++)
    {
        game->data.player.skills[i] = skills[i];
    }
    /* ======================================== */

    // Initialize Cities
    init_city(game, 0, 32, 32);
    /* ======================================== */

    // Initialize game state
    game->state.current_city = -1;
    game->state.current_view = INTRO_VIEW; // Set initial view to INTRO_VIEW
    game->state.is_game_over = false;
    game->state.is_paused = false;
    game->state.is_victory = false;
    game->state.selected_building_id = -1;
    game->state.selected_plot = -1;
    game->state.selected_staff_id = -1;
    game->state.selected_plot = -1;
    game->state.selected_staff_id = -1;
    game->state.time_scale = 1;
    game->state.ui_state.show_main_menu = true;
    /* ======================================== */

    game->data.current_staff_count = 0;
    srand(time(NULL)); // Seed random number generator
}

void init_city(Game *game, CityId id, uint32_t width, uint32_t height)
{
    game->data.world.cities[id].name_id = id;
    game->data.world.cities[id].is_unlocked = false;
    game->data.world.cities[id].unlock_networth_requirement = 1000000;
    game->data.world.cities[id].building_count = 0;
    game->data.world.cities[id].plots_unlocked = 0;
    game->data.world.cities[id].population = 0;
    game->data.world.cities[id].grid_width = width;
    game->data.world.cities[id].grid_height = height;
    game->data.world.cities[id].city_map = game->data.assets.cities_texture[id]; // Match texture to id

    game->data.world.cities[id].grid = (GridCell **)arena_alloc(&game->arena, height * sizeof(GridCell *));
    if (!game->data.world.cities[id].grid)
    {
        TraceLog(LOG_ERROR, "Failed to allocate grid rows");
        return;
    }
    for (uint32_t i = 0; i < height; i++)
    {
        game->data.world.cities[id].grid[i] = (GridCell *)arena_alloc(&game->arena, width * sizeof(GridCell));
        if (!game->data.world.cities[id].grid[i])
        {
            TraceLog(LOG_ERROR, "Failed to allocate grid row %u", i);
            return;
        }
    }
    for (uint32_t y = 0; y < height; y++)
    {
        for (uint32_t x = 0; x < width; x++)
        {
            game->data.world.cities[id].grid[y][x].type = GRID_CELL_EMPTY;
            game->data.world.cities[id].grid[y][x].building_id = -1;
            game->data.world.cities[id].grid[y][x].plot_id = 0; // all plot 0 fix !!
        }
    }
}

void handle_input(Game *game)
{
    // Handle Intro View Input
    if (game->state.current_view == INTRO_VIEW)
    {
        float buttonWidth = 200;
        float buttonHeight = 50;
        float buttonX = (SCREEN_WIDTH - buttonWidth) / 2.0f;
        float buttonY = SCREEN_HEIGHT * 0.75f - (buttonHeight / 2.0f);

        if (GuiButton((Rectangle){buttonX, buttonY, buttonWidth, buttonHeight}, "Start Game"))
        {
            game->state.current_view = MAIN_MENU_VIEW;
        }
    }
}

void update_game(Game *game, float dt) {}

void draw_game(Game *game)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    switch (game->state.current_view)
    {
    case INTRO_VIEW:
    {
        DrawTexture(game->data.assets.intro_texture, 0, 0, WHITE);

        float buttonWidth = 200;
        float buttonHeight = 50;
        float buttonX = (SCREEN_WIDTH - buttonWidth) / 2.0f;
        float buttonY = SCREEN_HEIGHT * 0.75f - (buttonHeight / 2.0f);

        if (GuiButton((Rectangle){buttonX, buttonY, buttonWidth, buttonHeight}, "Start Game"))
        {
            //////
        }
    }
    break;
    case MAIN_MENU_VIEW:
    {
        DrawText("Main Menu Placeholder", 10, 10, 20, DARKGRAY);
    }
    break;
    case PLANET_VIEW:
    {
        DrawText("Planet View Placeholder", 10, 10, 20, DARKGRAY);
    }
    break;
    case CITY_VIEW:
    {
        DrawText("City View Placeholder", 10, 10, 20, DARKGRAY);
    }
    break;
    case SETTING_GAME_VIEW:
    {
        DrawText("Game Settings Placeholder", 10, 10, 20, DARKGRAY);
    }
    break;
    case SETTING_MAIN_MENU_VIEW:
    {
        DrawText("Main Menu Settings Placeholder", 10, 10, 20, DARKGRAY);
    }
    break;
    case GAME_OVER_VIEW:
    {
        DrawText("Game Over Placeholder", 10, 10, 20, DARKGRAY);
    }
    break;
    case VICTORY_VIEW:
    {
        DrawText("Victory Placeholder", 10, 10, 20, DARKGRAY);
    }
    break;
    }

    EndDrawing();
}

void *arena_alloc(MemoryArena *arena, size_t size)
{
    if (arena->used + size > arena->size)
    {
        printf("Arena out of memory!\n");
        return NULL;
    }
    void *result = arena->current;
    arena->current += size;
    arena->used += size;
    return result;
}

void arena_free(MemoryArena *arena)
{
    free(arena->base);
    arena->base = NULL;
    arena->current = NULL;
    arena->size = 0;
    arena->used = 0;
}

void clean_up(Game *game)
{
    // Unload assets
    UnloadTexture(game->data.assets.small_restaurant_texture);
    UnloadTexture(game->data.assets.medium_restaurant_texture);
    UnloadTexture(game->data.assets.large_restaurant_texture);
    // unload everything here dont forget

    // Free arena
    arena_free(&game->arena);

    UnloadTexture(game->data.assets.intro_texture); // Unload intro texture
}
