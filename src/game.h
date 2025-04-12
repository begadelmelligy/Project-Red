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

#define MAX_STAFF_PER_BUILDING 10
#define MAX_BUILDINGS_PER_CITY 20
#define MAX_PLOTS_PER_CITY 4
#define MAX_CITIES 4
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define MAX_CUSTOMERS (MAX_CITIES * 100)
#define ARENA_SIZE (10 * 1024 * 1024) // 10 MB
#define GRID_CELL_SIZE 64

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
    CUSTOMER_STATE_LEAVING

} CustomerState;

typedef enum
{
    SKILL_RARITY_CHANCE_1,     // +20% more chance for staff rolling
    SKILL_REDUCED_ROLL_COST_1, // -20% Less rolling cost
    SKILL_FASTER_COOKING_1,    // +20% faster cooking
    SKILL_GUARDS_ATTACK_1,     // +20% Guard attack
    SKILL_BUILDINGS_HEALTH_1,  // +20% Building health
    SKILL_COUNT
} SkillId;

typedef struct
{
    SkillId id;
    char name[64];
    char description[256];
    int cost;
    bool is_unlocked;

} Skill;

typedef enum
{
    INTRO_VIEW,
    MAIN_MENU_VIEW,
    PLANET_VIEW,
    CITY_VIEW,
    SETTING_GAME_VIEW,
    SETTING_MAIN_MENU_VIEW,
    GAME_OVER_VIEW,
    VICTORY_VIEW

} GameView;

typedef enum
{
    GRID_CELL_EMPTY,       // Available for building
    GRID_CELL_BLOCKED,     // Permanently blocked
    GRID_CELL_BUILDING,    // Contains a building
    GRID_CELL_LOCKED_PLOT, // Part of a locked plot
    GRID_CELL_NOT_USED     // Extra grid not used for smaller maps

} GridCellType;

/* ========== GAME DATA ========== */

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
    BuildingType type;
    char name[64];
    uint8_t staff_capacity;
    uint32_t base_cost;
    uint32_t maintenance_cost;
    Vector2 size;
    Texture2D texture;

} BuildingTemplate;

typedef struct
{
    BuildingTemplate template;
    uint32_t id;
    uint32_t city_id;
    uint32_t plot_id;
    BuildingType type;
    Vector2 position;
    bool is_operational;
    Staff *assigned_staff[MAX_STAFF_PER_BUILDING];
    uint32_t current_staff_count;

} Building;

typedef struct
{
    uint32_t id;
    uint32_t city_id;
    bool is_unlocked;
    uint32_t purchase_cost;
    uint32_t grid_row_start;
    uint32_t grid_col_start;
    uint32_t grid_row_end;
    uint32_t grid_col_end;
} LandPlot;

typedef struct
{
    GridCellType type;
    int32_t building_id;
    uint32_t plot_id;
} GridCell;

typedef struct
{
    CityId name_id;
    bool is_unlocked;
    uint64_t unlock_networth_requirement;
    uint32_t building_count;
    uint32_t plots_unlocked;
    uint64_t population;
    Texture2D city_map;
    uint32_t grid_width;
    uint32_t grid_height;

    GridCell **grid; // double pointer for 2d grid[y][x]... pointer to colums and pointer to the pointer of colums for rows to allow us to have dynamic grid sizes for the maps could be just grid[100] or grid[MAX_GRID] but why we are cool xdd this is the common way to have a dynamic 2d array
    LandPlot plots[MAX_PLOTS_PER_CITY];
    Building buildings[MAX_BUILDINGS_PER_CITY];
} City;

typedef struct
{
    City cities[MAX_CITIES];
    uint32_t city_count;
    Model model; // 3D planet model (GLB)

} World;

typedef struct
{
    uint64_t net_worth;
    uint16_t num_owned_plots;
    uint16_t level;
    uint64_t current_exp;
    uint64_t exp_to_next_level;

    Skill skills[SKILL_COUNT];
    int skill_points_available;
    int skill_points_spent;

} Player;

typedef struct
{
    Texture2D small_restaurant_texture;
    Texture2D medium_restaurant_texture;
    Texture2D large_restaurant_texture;

    Texture2D customer_idel_texture;
    Texture2D customer_walking_texture;
    Texture2D customer_eating_texture;

    Texture2D cities_texture[MAX_CITIES];

    Texture2D intro_texture;

} Assets;

typedef struct
{
    uint32_t id;
    Texture2D sprite_sheet;
    Rectangle *frames; // allocate dynamically (arena) - pointer to allow different frames/animation
    uint32_t frame_count;
    float frame_duration;
    uint32_t current_frame;
    float timer;
    bool is_looping;

} Animation;

typedef struct
{
    uint32_t id;
    Vector2 position;
    Vector2 target_position;
    float speed;
    CustomerState state;
    int32_t target_building_id;
    float satisfaction;
    Animation *animation; // allocate dynamically (arena) - pointer to be able to point to any animation"idle,walking,eating...etc"

} Customer;

typedef struct
{
    char *base;
    char *current;
    size_t size;
    size_t used;

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
    Rectangle list_view_rect;
    Rectangle scroll_bar_rect;
    float scroll_offset_y;
    Rectangle hire_button_rect;
    Rectangle assign_button_rect;
    Rectangle fire_button_rect;

} StaffMenu;

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
    BuildingMenu building_menu;
    StaffMenu staff_menu;
    SettingsMenu settings_menu;
    HUDUi hud_data;

} UIData;

typedef struct
{
    Player player;

    UIData ui_data;

    World world;

    BuildingTemplate building_templates[TEMPLATE_COUNT];

    Staff staff_pool[MAX_STAFF_PER_BUILDING * MAX_BUILDINGS_PER_CITY * MAX_CITIES]; // sus dynamic it later
    uint32_t current_staff_count;

    Assets assets;

} GameData;

/* ========== GAME STATE ========== */

typedef struct
{
    bool show_main_menu;
    bool show_build_menu;
    bool show_staff_menu;
    bool show_settings_menu;
    bool building_menu_visible;
    int selected_building_type; // -1 if none selected
} UIState;

typedef struct
{
    GameView current_view;

    int32_t current_city;
    int32_t selected_plot;
    int32_t selected_building_id;
    int32_t selected_staff_id;

    bool is_paused;
    bool is_game_over;
    bool is_victory;

    float time_scale;

    UIState ui_state;

} GameState;

/* ========== MAIN GAME STRUCTURE ========== */

typedef struct
{
    GameData data;
    GameState state;
    Camera2D camera;
    MemoryArena arena;

} Game;

/* ========== FUNCTION PROTOTYPES ========== */
void init_game(Game *game);
void handle_input(Game *game);
void update_game(Game *game, float dt);
void draw_game(Game *game);
void clean_up(Game *game);

void init_city(Game *game, CityId id, uint32_t width, uint32_t height);

void *arena_alloc(MemoryArena *arena, size_t size);
void arena_free(MemoryArena *arena);

#endif // GAME_H
