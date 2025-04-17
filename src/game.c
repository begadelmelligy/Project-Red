#include "game.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

void init_game(Game *game)
{
    // init window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Project Red");
    SetTargetFPS(60);
    /* ======================================== */

    // camera init
    // Calculate initial camera position using spherical coordinates relative to the target (origin).
    // CAMERA_DISTANCE: Distance from the target (origin).
    // CAMERA_ANGLE: Elevation angle from the XZ plane (0 = horizontal, PI/2 = directly above).
    // CAMERA_ROTATION_ANGLE: Azimuth angle around the Y axis (rotation on the XZ plane).
    game->camera.position = (Vector3){
        CAMERA_DISTANCE * cosf(CAMERA_ANGLE) * sinf(CAMERA_ROTATION_ANGLE), // X = r * cos(elevation) * sin(azimuth)
        CAMERA_DISTANCE * sinf(CAMERA_ANGLE),                               // Y = r * sin(elevation)
        CAMERA_DISTANCE * cosf(CAMERA_ANGLE) * cosf(CAMERA_ROTATION_ANGLE)  // Z = r * cos(elevation) * cos(azimuth)
    };

    game->camera.target = (Vector3){0, 0, 0}; // Look at the origin
    game->camera.up = (Vector3){0, 1, 0};     // Standard up direction
    game->camera.projection = CAMERA_ORTHOGRAPHIC;
    game->camera.fovy = 30.0f;
    /* ======================================== */

    // load assets
    game->data.assets.cities_model[0] = LoadModel("assets/city_0.vox");
    game->data.assets.cities_model[1] = LoadModel("assets/city_1.vox");
    game->data.assets.cities_model[2] = LoadModel("assets/city_2.vox");
    game->data.assets.cities_model[3] = LoadModel("assets/city_3.vox");

    game->data.assets.intro_texture = LoadTexture("assets/intro_texture.png");

    game->data.assets.large_restaurant_model = LoadModel("assets/large_rest.vox");
    game->data.assets.medium_restaurant_model = LoadModel("assets/meduim_rest.vox");
    game->data.assets.small_restaurant_model = LoadModel("assets/small_rest.vox");

    game->data.assets.planet = LoadModel("assets/planet.vox");
    /* ======================================== */

    // init templates
    game->data.building_templates[0].base_cost = 1000;
    game->data.building_templates[0].maintenance_cost = 100;
    game->data.building_templates[0].staff_capacity = 5;
    game->data.building_templates[0].type = BUILDING_RESTAURANT_SMALL;
    game->data.building_templates[0].model = game->data.assets.small_restaurant_model;

    game->data.building_templates[1].base_cost = 5000;
    game->data.building_templates[1].maintenance_cost = 1000;
    game->data.building_templates[1].staff_capacity = 10;
    game->data.building_templates[1].type = BUILDING_RESTAURANT_MEDIUM;
    game->data.building_templates[1].model = game->data.assets.medium_restaurant_model;

    game->data.building_templates[2].base_cost = 10000;
    game->data.building_templates[2].maintenance_cost = 5000;
    game->data.building_templates[2].staff_capacity = 15;
    game->data.building_templates[2].type = BUILDING_RESTAURANT_LARGE;
    game->data.building_templates[2].model = game->data.assets.large_restaurant_model;
    /* ======================================== */

    // init player
    game->data.player.net_worth = 5000;
    /* ======================================== */

    // init staff
    for (size_t i = 0; i < MAX_STAFF_OWNED; i++)
    {
        game->data.staff_owned[i] = (Staff){0};
        game->data.staff_owned[i].assigned_building_id = -1;
    }
    /* ======================================== */

    // init world
    game->data.planet.model = game->data.assets.planet;
    /* ======================================== */

    // init cities
    CityPrices city_prices[4] = {CITY_0, CITY_1, CITY_2, CITY_3};
    for (size_t i = 0; i < MAX_CITIES; i++)
    {
        game->data.cities[i].city_model = game->data.assets.cities_model[i];
        game->data.cities[i].current_building_count = 0;
        game->data.cities[i].is_unlocked = (i == 0) ? true : false;
        game->data.cities[i].name_id = (CityId)i;
        game->data.cities[i].price_to_unlock = city_prices[i];
        for (size_t j = 0; j < MAX_BUILDINGS_PER_CITY; j++)
        {
            game->data.cities[i].buildings[j] = (Building){0};
            game->data.cities[i].buildings[j].id = -1;
            game->data.cities[i].buildings[j].rotation_angle = 0.0f; // Initialize rotation
        }
    }
    /* ======================================== */

    // Initialize game state
    game->state.current_city = 0;
    game->state.current_scene = MAIN_MENU_SCENE;
    game->state.is_game_over = false;
    game->state.is_paused = false;
    game->state.is_victory = false;
    game->state.selected_building_id = -1;
    game->state.selected_staff_id = -1;
    game->state.is_building_placement_mode = false;
    game->state.building_placement_position = (Vector3){0, 0, 0};
    game->state.selected_building_type_to_place = -1;
    game->state.building_placement_rotation_angle = 0.0f; // Initialize placement rotation
    /* ======================================== */
}

Vector3 get_grid_position_from_mouse(Game *game)
{
    Vector2 mousePosition = GetMousePosition();
    Ray ray = GetMouseRay(mousePosition, game->camera);
    Vector3 planeNormal = {0.0f, 1.0f, 0.0f}; // Normal vector of the ground plane (pointing straight up along Y).
    Vector3 planePoint = {0.0f, 0.0f, 0.0f};  // A point on the ground plane (the origin).

    // --- Ray-Plane Intersection Calculation ---
    // Formula: t = dot(planePoint - ray.position, planeNormal) / dot(ray.direction, planeNormal)
    // where 't' is the distance along the ray direction where the intersection occurs.

    // Calculate the denominator of the intersection formula.
    float denominator = Vector3DotProduct(ray.direction, planeNormal);

    // Check if the ray is parallel to the plane (denominator is close to zero).
    // If so, no intersection or infinite intersections (ray lies in the plane).
    if (fabsf(denominator) < 0.0001f)
        return (Vector3){0, 0, 0}; // Return a default position (e.g., origin)

    // Calculate the numerator: dot product of the vector from ray origin to a point on the plane and the plane normal.
    float t = Vector3DotProduct(Vector3Subtract(planePoint, ray.position), planeNormal) / denominator;

    // If t is negative, the intersection point is behind the ray's origin.
    if (t < 0.0f)
        return (Vector3){0, 0, 0}; // Return a default position

    // Calculate the actual intersection point in world coordinates:
    // Intersection = ray.position + ray.direction * t
    Vector3 intersection = Vector3Add(ray.position, Vector3Scale(ray.direction, t));

    // --- Grid Snapping Logic ---
    // Define grid properties (ideally as constants elsewhere).
    const float spacing = 0.1f;                           // Gap between grid cells.
    const float floorCubeSize = 2.0f;                     // Size of the cube representing a cell.
    const int floorGridSize = 20;                         // Number of cells along X and Z axes.
    const float cellSize = floorCubeSize + spacing;       // Total size of a cell including spacing.
    const float gridTotalSpan = floorGridSize * cellSize; // Total width/depth of the grid.
    const float gridHalfSpan = gridTotalSpan / 2.0f;      // Half the span, used to center the grid at the origin.

    // Convert world intersection coordinates (X, Z) to grid indices.
    // 1. Shift the coordinate system origin from the center of the grid (-gridHalfSpan)
    //    to the corner (0) by adding gridHalfSpan.
    // 2. Divide by the total size of a cell (cube + spacing) to get the index.
    // 3. Use floorf() to get the integer index of the cell the point falls into.
    float gridX = floorf((intersection.x + gridHalfSpan) / cellSize);
    float gridZ = floorf((intersection.z + gridHalfSpan) / cellSize);

    // Clamp grid indices to ensure they stay within the valid range [0, floorGridSize - 1].
    gridX = fmaxf(0, fminf(gridX, floorGridSize - 1));
    gridZ = fmaxf(0, fminf(gridZ, floorGridSize - 1));

    // Convert the clamped grid indices back to world coordinates for the center of the target grid cell.
    // 1. Multiply the index by the cell size to get the position of the cell's corner.
    // 2. Subtract gridHalfSpan to shift the coordinate system back so the grid is centered at the world origin.
    // 3. Add half the cube size (floorCubeSize / 2.0f) to get the center point of the cell.
    Vector3 gridPosition = {
        gridX * cellSize - gridHalfSpan + (floorCubeSize / 2.0f),
        0.0f, // Place the object on the ground plane (Y=0).
        gridZ * cellSize - gridHalfSpan + (floorCubeSize / 2.0f)};

    return gridPosition;
}

bool unlock_city(Game *game, int city_index)
{
    if (city_index < 0 || city_index >= MAX_CITIES)
        return false;
    if (game->data.cities[city_index].is_unlocked)
        return true;

    CityPrices price = game->data.cities[city_index].price_to_unlock;

    if (game->data.player.net_worth >= price)
    {
        game->data.player.net_worth -= price;
        game->data.cities[city_index].is_unlocked = true;
        return true;
    }

    return false;
}

const char *get_city_name(CityId id)
{
    switch (id)
    {
    case CITY_EAST:
        return "East City";
    case CITY_WEST:
        return "West City";
    case CITY_NORTH:
        return "North City";
    case CITY_SOUTH:
        return "South City";
    default:
        return "Unknown City";
    }
}

void handle_input(Game *game)
{
    if (game->state.current_scene == MAIN_MENU_SCENE)
    {
        float buttonWidth = 200;
        float buttonHeight = 50;
        float buttonX = (SCREEN_WIDTH - buttonWidth) / 2.0f;
        float buttonY = SCREEN_HEIGHT * 0.75f - (buttonHeight / 2.0f);
        Rectangle startButtonRect = {buttonX, buttonY, buttonWidth, buttonHeight};

        if (GuiButton(startButtonRect, "Start Game"))
        {
            game->state.current_scene = PLANET_SCENE;
        }
    }
    else if (game->state.current_scene == PLANET_SCENE)
    {
        char netWorthText[50];
        sprintf(netWorthText, "Net Worth: $%llu", game->data.player.net_worth);
        DrawText(netWorthText, 20, 20, 20, GREEN);

        // Back to main menu button
        Rectangle backButtonRect = {SCREEN_WIDTH - 220, 20, 200, 50};
        if (GuiButton(backButtonRect, "Back to Main Menu"))
        {
            game->state.current_scene = MAIN_MENU_SCENE;
        }

        float buttonWidth = 200;
        float buttonHeight = 50;
        float buttonSpacing = 20;
        float startY = 100;

        for (int i = 0; i < MAX_CITIES; i++)
        {
            City *city = &game->data.cities[i];
            Rectangle cityBtnRect = {50, startY + (buttonHeight + buttonSpacing) * i, buttonWidth, buttonHeight};

            char buttonText[100];
            if (city->is_unlocked)
            {
                sprintf(buttonText, "Go to %s", get_city_name(city->name_id));
                if (GuiButton(cityBtnRect, buttonText))
                {
                    game->state.current_city = i;
                    game->state.current_scene = CITY_SCENE;
                }
            }
            else
            {
                sprintf(buttonText, "Unlock %s ($%llu)", get_city_name(city->name_id), city->price_to_unlock);
                if (GuiButton(cityBtnRect, buttonText))
                {
                    if (unlock_city(game, i))
                    {
                        game->state.current_city = i;
                        game->state.current_scene = CITY_SCENE;
                    }
                }
            }
        }
    }
    else if (game->state.current_scene == CITY_SCENE)
    {
        char netWorthText[50];
        sprintf(netWorthText, "Net Worth: $%llu", game->data.player.net_worth);
        DrawText(netWorthText, 20, 20, 20, GREEN);

        char cityText[50];
        sprintf(cityText, "Current City: %s", get_city_name(game->data.cities[game->state.current_city].name_id));
        DrawText(cityText, 20, 50, 20, WHITE);

        float buttonWidth = 200;
        float buttonHeight = 50;
        float buttonX = SCREEN_WIDTH - buttonWidth - 20;
        float buttonY = 20;
        Rectangle goToPlanetButtonRect = {buttonX, buttonY, buttonWidth, buttonHeight};

        if (GuiButton(goToPlanetButtonRect, "Return to Planet"))
        {
            game->state.current_scene = PLANET_SCENE;
        }

        char smallRestText[50], mediumRestText[50], largeRestText[50];
        sprintf(smallRestText, "Small Restaurant ($%u)", game->data.building_templates[0].base_cost);
        sprintf(mediumRestText, "Medium Restaurant ($%u)", game->data.building_templates[1].base_cost);
        sprintf(largeRestText, "Large Restaurant ($%u)", game->data.building_templates[2].base_cost);

        Rectangle buildingModeRect = {20, 80, 250, 30};
        if (GuiButton(buildingModeRect, smallRestText))
        {
            if (game->data.player.net_worth >= game->data.building_templates[0].base_cost)
            {
                game->state.is_building_placement_mode = true;
                game->state.selected_building_type_to_place = BUILDING_RESTAURANT_SMALL;
            }
        }

        Rectangle buildingModeRect2 = {20, 120, 250, 30};
        if (GuiButton(buildingModeRect2, mediumRestText))
        {
            if (game->data.player.net_worth >= game->data.building_templates[1].base_cost)
            {
                game->state.is_building_placement_mode = true;
                game->state.selected_building_type_to_place = BUILDING_RESTAURANT_MEDIUM;
            }
        }

        Rectangle buildingModeRect3 = {20, 160, 250, 30};
        if (GuiButton(buildingModeRect3, largeRestText))
        {
            if (game->data.player.net_worth >= game->data.building_templates[2].base_cost)
            {
                game->state.is_building_placement_mode = true;
                game->state.selected_building_type_to_place = BUILDING_RESTAURANT_LARGE;
            }
        }

        if (game->state.is_building_placement_mode)
        {
            game->state.building_placement_position = get_grid_position_from_mouse(game);

            // Handle rotation input
            if (IsKeyPressed(KEY_R))
            {
                game->state.building_placement_rotation_angle = fmodf(game->state.building_placement_rotation_angle + 90.0f, 360.0f);
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                City *current_city = &game->data.cities[game->state.current_city];

                if (current_city->current_building_count < MAX_BUILDINGS_PER_CITY)
                {
                    BuildingTemplate template = game->data.building_templates[game->state.selected_building_type_to_place];

                    if (game->data.player.net_worth >= template.base_cost)
                    {
                        game->data.player.net_worth -= template.base_cost;

                        place_building(current_city, game->state.selected_building_type_to_place,
                                       game->state.building_placement_position, template,
                                       game->state.building_placement_rotation_angle);

                        current_city->current_building_count++;
                    }

                    game->state.is_building_placement_mode = false;
                    game->state.building_placement_rotation_angle = 0.0f; // Reset rotation
                }
            }

            if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
            {
                game->state.is_building_placement_mode = false;
                game->state.building_placement_rotation_angle = 0.0f; // Reset rotation
            }
        }
    }
}

void update_game(Game *game, float dt)
{
    ///////
}

void draw_game(Game *game)
{
    BeginDrawing();
    ClearBackground(BLACK);

    switch (game->state.current_scene)
    {
    case MAIN_MENU_SCENE:
    {
        DrawText("Project Red", SCREEN_WIDTH / 2 - MeasureText("Project Red", 40) / 2, SCREEN_HEIGHT / 3, 40, RED);

        float buttonWidth = 200;
        float buttonHeight = 50;
        float buttonX = (SCREEN_WIDTH - buttonWidth) / 2.0f;
        float buttonY = SCREEN_HEIGHT * 0.75f - (buttonHeight / 2.0f);
        Rectangle startButtonRect = {buttonX, buttonY, buttonWidth, buttonHeight};

        GuiButton(startButtonRect, "Start Game");
    }
    break;
    case PLANET_SCENE:
    {
        char netWorthText[50];
        sprintf(netWorthText, "Net Worth: $%llu", game->data.player.net_worth);
        DrawText(netWorthText, 20, 20, 20, GREEN);

        DrawText("Select a City", 50, 60, 30, WHITE);

        // Back to main menu button
        Rectangle backButtonRect = {SCREEN_WIDTH - 220, 20, 200, 50};
        GuiButton(backButtonRect, "Back to Main Menu");

        BeginMode3D(game->camera);
        DrawModel(game->data.assets.planet, (Vector3){0, 0, 0}, 1.0f, WHITE);
        EndMode3D();

        float buttonWidth = 200;
        float buttonHeight = 50;
        float buttonSpacing = 20;
        float startY = 100;

        for (int i = 0; i < MAX_CITIES; i++)
        {
            City *city = &game->data.cities[i];
            Rectangle cityBtnRect = {50, startY + (buttonHeight + buttonSpacing) * i, buttonWidth, buttonHeight};

            char buttonText[100];
            if (city->is_unlocked)
            {
                sprintf(buttonText, "Go to %s", get_city_name(city->name_id));
                GuiButton(cityBtnRect, buttonText);
            }
            else
            {
                sprintf(buttonText, "Unlock %s ($%llu)", get_city_name(city->name_id), city->price_to_unlock);

                if (game->data.player.net_worth < city->price_to_unlock)
                {
                    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xE74C3CFF);
                    GuiButton(cityBtnRect, buttonText);
                    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xFFFFFFFF);
                }
                else
                {
                    GuiButton(cityBtnRect, buttonText);
                }
            }
        }
    }
    break;
    case CITY_SCENE:
    {
        const float spacing = 0.1f;
        const int floorGridSize = 20;
        const float floorCubeSize = 2.0f;
        const float gridSpan = (floorGridSize * (floorCubeSize + spacing)) / 2.0f;

        BeginMode3D(game->camera);

        City *city = &game->data.cities[game->state.current_city];
        for (int j = 0; j < city->current_building_count; j++)
        {
            Building *building = &city->buildings[j];
            if (building->id >= 0)
            {
                DrawModelEx(building->template.model, building->position,
                            (Vector3){0, 1, 0}, building->rotation_angle,
                            (Vector3){0.2f, 0.2f, 0.2f}, WHITE);
            }
        }

        if (game->state.is_building_placement_mode)
        {
            Model previewModel = game->data.building_templates[game->state.selected_building_type_to_place].model;
            Color previewColor = {255, 255, 255, 128};
            DrawModelEx(previewModel, game->state.building_placement_position,
                        (Vector3){0, 1, 0}, game->state.building_placement_rotation_angle,
                        (Vector3){0.2f, 0.2f, 0.2f}, previewColor);
        }

        for (int x = 0; x < floorGridSize; x++)
        {
            for (int z = 0; z < floorGridSize; z++)
            {
                Vector3 cubePos = {
                    x * (floorCubeSize + spacing) - gridSpan,
                    -floorCubeSize / 2.0f, // Center the cube so its top is at Y=0
                    z * (floorCubeSize + spacing) - gridSpan};
                DrawCube(cubePos, floorCubeSize, floorCubeSize, floorCubeSize, DARKGRAY);
                DrawCubeWires(cubePos, floorCubeSize, floorCubeSize, floorCubeSize, BLACK);
            }
        }

        EndMode3D();

        char netWorthText[50];
        sprintf(netWorthText, "Net Worth: $%llu", game->data.player.net_worth);
        DrawText(netWorthText, 20, 20, 20, GREEN);

        char cityText[50];
        sprintf(cityText, "Current City: %s", get_city_name(game->data.cities[game->state.current_city].name_id));
        DrawText(cityText, 20, 50, 20, WHITE);

        float buttonWidth = 200;
        float buttonHeight = 50;
        float buttonX = SCREEN_WIDTH - buttonWidth - 20;
        float buttonY = 20;
        Rectangle goToPlanetButtonRect = {buttonX, buttonY, buttonWidth, buttonHeight};
        GuiButton(goToPlanetButtonRect, "Return to Planet");

        char smallRestText[50], mediumRestText[50], largeRestText[50];
        sprintf(smallRestText, "Small Restaurant ($%u)", game->data.building_templates[0].base_cost);
        sprintf(mediumRestText, "Medium Restaurant ($%u)", game->data.building_templates[1].base_cost);
        sprintf(largeRestText, "Large Restaurant ($%u)", game->data.building_templates[2].base_cost);

        Rectangle buildingModeRect = {20, 80, 250, 30};
        if (game->data.player.net_worth < game->data.building_templates[0].base_cost)
        {
            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xE74C3CFF);
            GuiButton(buildingModeRect, smallRestText);
            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xFFFFFFFF);
        }
        else
        {
            GuiButton(buildingModeRect, smallRestText);
        }

        Rectangle buildingModeRect2 = {20, 120, 250, 30};
        if (game->data.player.net_worth < game->data.building_templates[1].base_cost)
        {
            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xE74C3CFF);
            GuiButton(buildingModeRect2, mediumRestText);
            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xFFFFFFFF);
        }
        else
        {
            GuiButton(buildingModeRect2, mediumRestText);
        }

        Rectangle buildingModeRect3 = {20, 160, 250, 30};
        if (game->data.player.net_worth < game->data.building_templates[2].base_cost)
        {
            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xE74C3CFF);
            GuiButton(buildingModeRect3, largeRestText);
            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, 0xFFFFFFFF);
        }
        else
        {
            GuiButton(buildingModeRect3, largeRestText);
        }

        if (game->state.is_building_placement_mode)
        {
            DrawText("Click to place building. Right-click to cancel.", 20, 200, 20, WHITE);
            DrawText("Press R to rotate.", 20, 225, 20, WHITE);
        }
    }
    break;
    case GAME_OVER_SCENE:
    {
        DrawText("Game Over Placeholder", 10, 10, 20, DARKGRAY);
    }
    break;
    case VICTORY_SCENE:
    {
        DrawText("Victory Placeholder", 10, 10, 20, DARKGRAY);
    }
    break;
    }

    EndDrawing();
}

void clean_up(Game *game)
{
    for (int i = 0; i < MAX_CITIES; i++)
    {
        UnloadModel(game->data.assets.cities_model[i]);
    }

    UnloadModel(game->data.assets.small_restaurant_model);
    UnloadModel(game->data.assets.medium_restaurant_model);
    UnloadModel(game->data.assets.large_restaurant_model);
    UnloadModel(game->data.assets.planet);

    UnloadTexture(game->data.assets.intro_texture);

    CloseWindow();
}

void place_building(City *city, BuildingType type, Vector3 position, BuildingTemplate template, float rotation_angle)
{
    int building_index = -1;
    for (int i = 0; i < MAX_BUILDINGS_PER_CITY; i++)
    {
        if (city->buildings[i].id == -1)
        {
            building_index = i;
            break;
        }
    }

    if (building_index == -1)
        return;

    city->buildings[building_index].current_staff_count = 0;
    city->buildings[building_index].id = building_index;
    city->buildings[building_index].is_operational = false;
    city->buildings[building_index].position = position;
    city->buildings[building_index].template = template;
    city->buildings[building_index].rotation_angle = rotation_angle;

    for (size_t i = 0; i < MAX_STAFF_PER_BUILDING; i++)
    {
        city->buildings[building_index].assigned_staff[i] = NULL;
    }
}

void hire_staff() {}
void sell_staff() {}
void assign_staff() {}
void collect_money() {}