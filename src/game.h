#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "raymath.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAX_STAFF_PER_BUILDING 50
#define MAX_BUILDINGS_PER_CITY 50
#define MAX_PLOTS_PER_CITY 100
#define MAX_CITIES 10

typedef enum
{
    BUILDING_RESTAURANT_SMALL,
    BUILDING_RESTAURANT_MEDIUM,
    BUILDING_RESTAURANT_LARGE,
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

typedef struct
{
    uint32_t id;
    char name[64];
    StaffRole role;
    uint32_t salary;
    int32_t assigned_building_id; // (-1 if unassigned)
    StaffRarity rarity;
    float base_efficiency;
} Staff;

typedef struct
{
    BuildingType type;
    char name[64];
    uint8_t staff_capacity;
    uint32_t base_cost;
    uint32_t mauint32_tenance_cost;
    uint32_t income;
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
    Staff *assigned_staff[MAX_STAFF_PER_BUILDING]; // array of pouint32_ters to staffs ... to allow any staff move to anybuilding and any upgrade to staff apply to the real staff inside the building
    uint32_t current_staff_count;
} Building;

typedef struct
{
    uint32_t id;
    uint32_t city_id;
    Rectangle area; // buildable area
    bool is_owned_by_player;
    bool has_building;
    int32_t building_id; // (-1 if none)
    uint32_t purchase_cost;
} LandPlot;

typedef struct
{
    uint32_t id;
    char name[50];
    Vector2 map_position; // Position on the world map view
    bool is_unlocked;
    uint64_t unlock_networth_requirement;
    LandPlot plots[MAX_PLOTS_PER_CITY];
    uint32_t plot_count;
    uint64_t population;
    uint32_t health_meter;
} City;

typedef enum
{
    SKILL_GAMBA_CHANCE_1,      // +2% Rarity
    SKILL_GAMBA_CHANCE_2,      // +4%  Rarity (req: 1)
    SKILL_UNIQUE_BOOST_1,      // +0.5% Unique chance (req: GAMBA_CHANCE_2)
    SKILL_REDUCED_ROLL_COST_1, // -10% roll cost

    SKILL_FASTER_COOKING_1, // +5% efficiency Cooks
    SKILL_LOWER_SALARIES_1, // -5% all salaries

    SKILL_STRONGER_GUARDS_1,   // +10% Guard attack
    SKILL_TOUGHER_BUILDINGS_1, // +15% Building health

    SKILL_COUNT
} SkillId;

#define MAX_PREREQS 3
typedef struct Skill
{
    SkillId id;
    char name[64];
    char description[256];
    int cost;
    SkillId prerequisites[MAX_PREREQS];
    int num_prerequisites;
    bool unlocked;
    Vector2 ui_position; // For drawing the tree visually
} Skill;

typedef struct
{
    uint64_t net_worth;
    uint32_t owned_plot_ids[MAX_PLOTS_PER_CITY * MAX_CITIES];
    uint32_t num_owned_plots;
    uint32_t level;
    uint64_t current_exp;
    uint64_t exp_to_next_level;

    float gamba_bonus_percent;
    float unique_bonus_percent;
    float roll_cost_modifier;
    float global_cook_efficiency_bonus;
    float global_salary_modifier;
    float global_guard_damage_bonus;
    float global_building_health_bonus;

    Skill skills[SKILL_COUNT];
    int skill_points_available;
    int skill_points_spent;
} Player;

typedef struct
{
    City cities[MAX_CITIES];
    uint32_t city_count;
} World;

typedef enum
{
    VIEW_WORLD_MAP,
    VIEW_CITY,
    VIEW_BUILDING_MODE,
} GameView;

typedef struct
{
    Player player;

    Staff staff_pool[MAX_STAFF_PER_BUILDING * MAX_BUILDINGS_PER_CITY * MAX_CITIES]; // Pool of all staff
    uint32_t staff_count;

    GameView current_view;
    uint32_t current_city_id;
    uint32_t selected_plot_id;
    uint32_t selected_building_id;

    // UI State
    bool show_main_menu;
    bool show_build_menu;

    double game_time;
    float time_scale;

} GameState;

void init_game();
void handle_input();
void update_game(float dt);
void draw_game();
void clean_up();

void init_building_templates(void);
void init_world();
void init_player(void);
void init_skills(void);

#endif // GAME_H
