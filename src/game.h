#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "raymath.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* ========== GAME CONSTANTS ========== */

#define MAX_STAFF_PER_BUILDING 15
#define MAX_BUILDINGS_PER_CITY 20
#define MAX_CITIES 4
#define MAX_STAFF_OWNED 1000

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

#define ARENA_SIZE (10 * 1024 * 1024) // 10 MB

#define CAMERA_DISTANCE 50.0f                 // only for clipping
#define CAMERA_ANGLE 35.264f * DEG2RAD        // 35.264° = arctan(1/sqrt(2))
#define CAMERA_ROTATION_ANGLE 45.0f * DEG2RAD // 45° rotation around vertical axis

/* ========== GAME ENUMS ========== */

typedef enum
{
    MAP_SIZE_TINY = 32,
    MAP_SIZE_SMALL = 64,
    MAP_SIZE_MEDIUM = 128,
    MAP_SIZE_LARGE = 256

} CitySizes;

typedef enum
{
    CITY_0 = 0,
    CITY_1 = 10000,
    CITY_2 = 100000,
    CITY_3 = 1000000

} CityPrices;

typedef enum
{
    CITY_EAST,
    CITY_WEST,
    CITY_NORTH,
    CITY_SOUTH,
    CITY_COUNT
} CityId;

typedef enum
{
    BUILDING_RESTAURANT_SMALL,
    BUILDING_RESTAURANT_MEDIUM,
    BUILDING_RESTAURANT_LARGE,
    TEMPLATE_COUNT
} BuildingType;

typedef enum
{
    ROLE_COOK,
    ROLE_SERVER,
    ROLE_MANAGER,
    ROLE_COUNT
} StaffRole;

typedef enum
{
    RARITY_COMMON,
    RARITY_RARE,
    RARITY_VERY_RARE,
    RARITY_UNIQUE,
    RARITY_COUNT
} StaffRarity;

typedef enum
{
    CUSTOMER_STATE_IDLE,
    CUSTOMER_STATE_MOVING,
    CUSTOMER_STATE_EATING,

} CustomerState;

typedef enum
{
    MAIN_MENU_SCENE,
    PLANET_SCENE,
    CITY_SCENE,
    GAME_OVER_SCENE,
    VICTORY_SCENE

} GameScenes;

typedef enum
{
    GRID_CELL_EMPTY,    // Available for building
    GRID_CELL_BLOCKED,  // Permanently blocked
    GRID_CELL_BUILDING, // Contains a building
    GRID_CELL_NOT_USED  // Extra grid not used for smaller maps

} GridCellType;

/* ========== GAME DATA ========== */

typedef struct
{
    BuildingType type;
    uint8_t staff_capacity;
    uint32_t base_cost;
    uint32_t maintenance_cost;
    Model model;

} BuildingTemplate;

typedef struct
{
    uint32_t id;
    char name[64];
    StaffRole role;
    uint32_t salary;
    int32_t assigned_building_id; // (-1 if unassigned)
    CityId home_city_id;
    StaffRarity rarity;
    float base_efficiency;

} Staff;

typedef struct
{
    BuildingTemplate template;
    uint32_t id;
    Vector3 position;
    float rotation_angle;
    bool is_operational;
    Staff *assigned_staff[MAX_STAFF_PER_BUILDING];
    uint32_t current_staff_count;

} Building;

typedef struct
{
    CityId name_id;
    bool is_unlocked;
    uint64_t price_to_unlock;
    uint32_t current_building_count;
    Model city_model;

    Building buildings[MAX_BUILDINGS_PER_CITY];
} City;

typedef struct
{
    Model model;

} Planet;

typedef struct
{
    uint64_t net_worth;
} Player;

typedef struct
{
    char *base;
    char *current;
    uint64_t size;
    uint64_t used;

} MemoryArena;

typedef struct
{
    Rectangle panel_rect;
    Rectangle small_restaurant_btn;
    Rectangle medium_restaurant_btn;
    Rectangle large_restaurant_btn;
    float button_padding;
    float button_height;

} BuildingMenu;

typedef struct
{
    Rectangle panel_rect;
    Rectangle volume_slider_area;
    float current_volume;
    Rectangle resolution_dropdown_rect;
    int selected_resolution_index;
    Rectangle apply_button_rect;
    Rectangle back_button_rect;

} SettingsMenu;

typedef struct
{
    Rectangle net_worth_display_area;
    Rectangle player_level_display_area;
    Rectangle current_city_display_area;
    Rectangle time_control_area;

} HUDUi;

typedef struct
{
    BuildingMenu building_ui;
    SettingsMenu settings_ui;
    HUDUi hud_ui;

} UIData;

typedef struct
{
    Model small_restaurant_model;
    Model medium_restaurant_model;
    Model large_restaurant_model;

    Model cities_model[MAX_CITIES];

    Model planet;

    Texture2D intro_texture;

} Assets;

typedef struct
{
    Player player;

    UIData ui_data;

    Planet planet;

    BuildingTemplate building_templates[TEMPLATE_COUNT];

    City cities[MAX_CITIES];

    Staff staff_owned[MAX_STAFF_OWNED];

    Assets assets;

} GameData;

/* ========== GAME STATE ========== */

typedef struct
{
    GameScenes current_scene;

    CityId current_city;
    int32_t selected_building_id;
    int32_t selected_staff_id;
    BuildingType selected_building_type_to_place;
    Vector3 building_placement_position;
    float building_placement_rotation_angle;

    bool is_building_placement_mode;
    bool is_paused;
    bool is_game_over;
    bool is_victory;

} GameState;

/* ========== MAIN GAME STRUCTURE ========== */

typedef struct
{
    GameData data;
    GameState state;
    Camera3D camera;
    MemoryArena arena;

} Game;

/* ========== FUNCTION PROTOTYPES ========== */
void init_game(Game *game);
void handle_input(Game *game);
void update_game(Game *game, float dt);
void draw_game(Game *game);
void clean_up(Game *game);

void place_building(City *city, BuildingType type, Vector3 position, BuildingTemplate template, float rotation_angle);
void hire_staff();
void sell_staff();
void assign_staff();
void collect_money();
Vector3 get_grid_position_from_mouse(Game *game);
bool unlock_city(Game *game, int city_index);
const char *get_city_name(CityId id);

#endif // GAME_H
