#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <glad/glad.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "engine/util.h"
#include "engine/global.h"
#include "engine/config.h"
#include "engine/time.h"
#include "engine/input.h"
#include "engine/physics.h"
#include "engine/entity.h"
#include "engine/render.h"
#include "engine/animation.h"
#include "engine/my_curl.h"
#include "engine/scenes.h"
#include "engine/audio.h"

void reset(void);

static bool DEBUG = false;
static bool CHEAT = false;

static Mix_Chunk *SOUND_JUMP;
static Mix_Chunk *SOUND_SHOOT;
static Mix_Chunk *SOUND_BULLET_HIT_WALL;
static Mix_Chunk *SOUND_HURT;
static Mix_Chunk *SOUND_ENEMY_DEATH;
static Mix_Chunk *SOUND_PLAYER_DEATH;
static Mix_Chunk *SOUND_FRUIT_COLLECT;
static Mix_Chunk *SOUND_GAME_OVER;

static Mix_Music *MUSIC_STAGE_1;
static Mix_Music *MUSIC_GAME_OVER;
static Mix_Music *MUSIC_MENU_PRINCIPAL;



static Sprite_Sheet sprite_sheet_player;
static Sprite_Sheet sprite_sheet_map;
static Sprite_Sheet sprite_sheet_enemy_small;
static Sprite_Sheet sprite_sheet_enemy_medium;
static Sprite_Sheet sprite_sheet_enemy_large;
static Sprite_Sheet sprite_sheet_enemy_flying;
static Sprite_Sheet sprite_sheet_fire;
static Sprite_Sheet sprite_sheet_props;
static Sprite_Sheet sprite_sheet_apple;
static Sprite_Sheet sprite_sheet_banana;
static Sprite_Sheet sprite_sheet_pineapple;
static Sprite_Sheet sprite_sheet_collected;

static const float HEALTH_PLAYER = 0;
static const float SPEED_PLAYER = 250;
static const float JUMP_VELOCITY = 1350;
static const float GROUNDED_TIME = 0.1f; // Temps passé au sol
static const float SPEED_ENEMY_LARGE = 80;
static const float SPEED_ENEMY_MEDIUM = 90;
static const float SPEED_ENEMY_SMALL = 100;
static const float SPEED_ENEMY_FLYING = 100;
static const float HEALTH_ENEMY_LARGE = 3;
static const float HEALTH_ENEMY_SMALL = 1;
static const float HEALTH_ENEMY_FLYING = 1;
static const float HEALTH_ENEMY_MEDIUM = 1;
static const float POINTS_FRUIT = 5;

typedef enum collision_layer
{
    COLLISION_LAYER_PLAYER = 1,
    COLLISION_LAYER_ENEMY = 1 << 1,
    COLLISION_LAYER_TERRAIN = 1 << 2,
    COLLISION_LAYER_ENEMY_PASSTHROUGH = 1 << 3,
    COLLISION_LAYER_PROJECTILE = 1 << 4,
    COLLISION_LAYER_FRUIT = 1 << 5,
    COLLISION_LAYER_FX = 1 << 6,
} Collision_Layer;

typedef enum fruit_type
{
    FRUIT_TYPE_APPLE,//0
    FRUIT_TYPE_BANANA,
    FRUIT_TYPE_PINEAPPLE,
    FRUIT_TYPE_COUNT,
} Fruit_Type;

typedef enum weapon_type
{
    WEAPON_TYPE_SHOTGUN,
    WEAPON_TYPE_PISTOL,
    WEAPON_TYPE_REVOLVER,
    WEAPON_TYPE_SMG,
    WEAPON_TYPE_ROCKET_LAUNCHER,
    WEAPON_TYPE_COUNT,
} Weapon_Type;

typedef enum projectile_type
{
    PROJECTILE_TYPE_SMALL,
    PROJECTILE_TYPE_LARGE,
    PROJECTILE_TYPE_ROCKET,
} Projectile_Type;

typedef struct weapon
{
    float fire_rate; // Time between shots.
    float recoil;
    float projectile_speed;
    Projectile_Type projectile_type;
    vec2 sprite_size;
    vec2 sprite_offset;
    size_t projectile_animation_id;
    Mix_Chunk *sfx;
} Weapon;

typedef struct fruit
{
    // float spawn_rate;
    vec2 sprite_size;
    vec2 sprite_offset;
    size_t fruit_animation_id;
} Fruit;

#define WIDTH 640
#define HEIGHT 360
static Weapon weapons[WEAPON_TYPE_COUNT] = {0};
static Fruit fruits[FRUIT_TYPE_COUNT] = {0};

static const float SPAWN_REGIONS[][4] = {
    {WIDTH * 0.5 - 570 * 0.5, HEIGHT * 0.5 + 5 - 93 * 0.5, 570 / 2, 93 / 2},
    {WIDTH * 0.25 - (WIDTH * 0.4) * 0.5, 70 - 60 * 0.5, (WIDTH * 0.4) / 2, 60 / 2},
    {WIDTH * 0.75 - (WIDTH * 0.4) * 0.5, 70 - 60 * 0.5, (WIDTH * 0.4) / 2, (60) / 2},
    {WIDTH * 0.5 - 570 * 0.5, HEIGHT - 64 - 60 * 0.5, 570 / 2, 60 / 2}};
static const u32 SPAWN_REGION_COUNT = 4;

static const u8 enemy_mask = COLLISION_LAYER_PLAYER | COLLISION_LAYER_TERRAIN;
static u8 player_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_TERRAIN | COLLISION_LAYER_ENEMY_PASSTHROUGH | COLLISION_LAYER_FRUIT;
static const u8 fire_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_PLAYER;
static u8 projectile_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_TERRAIN;
static u8 fruit_mask = COLLISION_LAYER_TERRAIN | COLLISION_LAYER_PLAYER;

static SDL_Window *window;

static size_t anim_player_walk_id;
static size_t anim_player_idle_id;

static size_t anim_ennemy_small_run_id;
static size_t anim_ennemy_medium_run_id;
static size_t anim_ennemy_large_run_id;
static size_t anim_ennemy_flying_fly_id;

static size_t anim_fire_id;
static size_t anim_projectile_small_id;

static size_t anim_fruit_apple_id;
static size_t anim_fruit_banana_id;
static size_t anim_fruit_pineapple_id;
static size_t anim_fruit_collected_id;

static size_t player_id;
static u32 texture_slots[16] = {0};

static Weapon_Type weapon_type = WEAPON_TYPE_PISTOL;
static bool player_is_grounded = false;
static float spawn_timer = 0;
static float ground_timer = 0;
static float shoot_timer = 0;
static int score = 0;
static float fruit_timer = 0;
static u8 fruits_count = 0;

char scoreText[20];

void projectile_on_hit(Body *self, Body *other, Hit hit)
{
    if (other->collision_layer == COLLISION_LAYER_ENEMY && other->is_active)
    {
        Entity *projectile = entity_get(self->entity_id);
        Entity *enemy = entity_get(other->entity_id);
        if (projectile->animation_id == anim_projectile_small_id)
        {
            if (entity_damage(other->entity_id, 1))
            {
                audio_sound_play(SOUND_ENEMY_DEATH);
                score++;
            }
        }
        audio_sound_play(SOUND_HURT);
        entity_destroy(self->entity_id);
    }
}

void projectile_on_hit_static(Body *self, Static_Body *other, Hit hit)
{
    Entity *projectile = entity_get(self->entity_id);
    if (projectile->animation_id == anim_projectile_small_id)
    {
        audio_sound_play(SOUND_SHOOT);
    }
    entity_destroy(self->entity_id);
}
static void spawn_projectile(Projectile_Type projectile_type)
{
    Weapon weapon = weapons[weapon_type];
    Entity *player = entity_get(player_id);
    Body *body = physics_body_get(player->body_id);
    Animation *animation = animation_get(player->animation_id);
    bool is_flipped = animation->is_flipped;
    vec2 velocity = {is_flipped ? -weapon.projectile_speed : weapon.projectile_speed, 0};

    entity_create(body->aabb.position, weapon.sprite_size, weapon.sprite_offset, velocity, COLLISION_LAYER_PROJECTILE, projectile_mask, true, weapon.projectile_animation_id, projectile_on_hit, projectile_on_hit_static, 0, 0, ENTITY_PROJECTILE);
    audio_sound_play(weapon.sfx);
}

void player_on_hit(Body *self, Body *other, Hit hit)
{
    if (other->collision_layer == COLLISION_LAYER_ENEMY && !CHEAT)
    {
        Entity *enemy = entity_get(other->entity_id);
        if (enemy->is_active)
        {
            show_game_over(score, enemy->entity_type);
            
            entity_destroy(self->entity_id);
            // entity_destroy(other->entity_id);
            reset();
        }
    }
    else if (other->collision_layer == COLLISION_LAYER_FRUIT)
    {
        Entity *fruit = entity_get(other->entity_id);
        if (fruit->is_active)
        {
            score += POINTS_FRUIT;
            fruits_count--;
            Body *body = physics_body_get(fruit->body_id);
            entity_create(body->aabb.position, (vec2){32, 32}, fruit->sprite_offset, body->velocity, COLLISION_LAYER_FX, fruit_mask, true, anim_fruit_collected_id, NULL, NULL, 0, 0, ENTITY_FX);
            fruit->is_active = false;
            entity_destroy(other->entity_id);
            audio_sound_play(SOUND_FRUIT_COLLECT);
        }
    }
}

void player_on_hit_static(Body *self, Static_Body *other, Hit hit)
{
    if (hit.normal[1] > 0)
    {
        player_is_grounded = true;
    }
}

void enemy_on_hit_static(Body *self, Static_Body *other, Hit hit)
{
    Entity *entity = entity_get(self->entity_id);

    if (hit.normal[0] > 0)
    {
        if (entity->is_enraged)
            self->velocity[0] = entity->speed * 1.5f;
        else
            self->velocity[0] = entity->speed;
    }

    if (hit.normal[0] < 0)
    {
        if (entity->is_enraged)
            self->velocity[0] = -entity->speed * 1.5f;
        else
            self->velocity[0] = -entity->speed;
    }
}

float frandr(float min, float max)
{
    float r = (rand() % 100) / 100.0f;
    return r * (max - min) + min;
}

void spawn_fruit(Fruit_Type fruit_type)
{

    const float *region = &SPAWN_REGIONS[rand() % SPAWN_REGION_COUNT][0];
    float x = frandr(region[0], region[0] + region[2]);
    float y = frandr(region[1], region[1] + region[3]);

    Fruit fruit = fruits[fruit_type];

    entity_create((vec2){x, y},
                  fruit.sprite_size,
                  fruit.sprite_offset,
                  (vec2){0, 0},
                  COLLISION_LAYER_FRUIT,
                  fruit_mask,
                  false,
                  fruit.fruit_animation_id,
                  NULL,
                  NULL,
                  0,
                  0,
                  ENTITY_FRUIT);
}

void spawn_enemy(Entity_Type enemy_type, bool is_enraged, bool is_flipped)
{
    float spawn_x = is_flipped ? render_width : 0;
    vec2 position = {spawn_x, (render_height - 64)};
    float speed, health;
    vec2 size, sprite_offset;
    size_t animation_id;
    On_Hit_Static on_hit_static = enemy_on_hit_static;

    switch (enemy_type)
    {
    case ENTITY_ENEMY_TYPE_SMALL:
        size[0] = 12;
        size[1] = 25;
        sprite_offset[0] = 0;
        sprite_offset[1] = 3; // Vertical
        speed = SPEED_ENEMY_SMALL;
        health = HEALTH_ENEMY_SMALL;
        animation_id = anim_ennemy_small_run_id;
        break;
    case ENTITY_ENEMY_TYPE_MEDIUM:
        size[0] = 23;
        size[1] = 32;
        sprite_offset[0] = 0;
        sprite_offset[1] = 5;
        speed = SPEED_ENEMY_MEDIUM;
        health = HEALTH_ENEMY_MEDIUM;
        animation_id = anim_ennemy_medium_run_id;
        break;

    case ENTITY_ENEMY_TYPE_LARGE:
        size[0] = 52;
        size[1] = 30;
        sprite_offset[0] = 0;
        sprite_offset[1] = 1;
        speed = SPEED_ENEMY_LARGE;
        health = HEALTH_ENEMY_LARGE;
        animation_id = anim_ennemy_large_run_id;
        break;

    case ENTITY_ENEMY_TYPE_FLYING:
        size[0] = 35;
        size[1] = 30;
        sprite_offset[0] = 0;
        sprite_offset[1] = 1;
        speed = SPEED_ENEMY_FLYING;
        health = HEALTH_ENEMY_FLYING;
        animation_id = anim_ennemy_flying_fly_id;
        break;

    default:
        ERROR_EXIT("ERREUR SPANW ENNEMY= ", "Type d'ennemi invalide.");
        return;
    }

    if (is_enraged)
    {
        speed *= 1.5;
    }

    vec2 velocity = {is_flipped ? -speed : speed, 0};
    size_t id = entity_create(position, size, sprite_offset, velocity, COLLISION_LAYER_ENEMY, enemy_mask, false, animation_id, NULL, on_hit_static, health, speed, enemy_type);
    Entity *entity = entity_get(id);
    entity->is_enraged = is_enraged;
}

void fire_on_hit(Body *self, Body *other, Hit hit)
{
    if (other->collision_layer == COLLISION_LAYER_ENEMY)
    {
        if (other->is_active)
        {
            Entity *enemy = entity_get(other->entity_id);
            entity_destroy(other->entity_id);
        }
    }
    else if (other->collision_layer == COLLISION_LAYER_PLAYER)
    {
        show_game_over(score, ENTITY_FIRE);
        reset();
    }
    else if (other->collision_layer == COLLISION_LAYER_FRUIT)
    {
        entity_destroy(other->entity_id);
        fruits_count = 0;
    }
}

void reset(void)
{
     
    audio_music_play(MUSIC_STAGE_1);

    physics_reset();
    entity_reset();

    ground_timer = 0;
    spawn_timer = 0;
    shoot_timer = 0;
    score = 0;
    fruits_count = 0;

    float width = global.window_width / render_get_scale();
    float height = global.window_height / render_get_scale();

    player_id = entity_create((vec2){100, 200}, // Postion (x,y)
                              (vec2){24, 24},   // Size
                              (vec2){0, 0},     // Offset sprite
                              (vec2){0, 0},     // Velocité
                              COLLISION_LAYER_PLAYER,
                              player_mask,
                              false, // is_kinematic flag
                              (size_t)-1,
                              player_on_hit,        // on hit
                              player_on_hit_static, // on hit static
                              HEALTH_PLAYER,
                              SPEED_PLAYER,
                              ENTITY_PLAYER

    );
    // Initialiser terrain du  niveau
    {

        physics_static_body_create((vec2){width * 0.5, height - 16}, (vec2){width, 32}, COLLISION_LAYER_TERRAIN);           // Plafond
        physics_static_body_create((vec2){width * 0.25 - 16, 16}, (vec2){width * 0.5 - 32, 48}, COLLISION_LAYER_TERRAIN);   // Sol gauche
        physics_static_body_create((vec2){width * 0.75 + 16, 16}, (vec2){width * 0.5 - 32, 48}, COLLISION_LAYER_TERRAIN);   // Sol droite
        physics_static_body_create((vec2){16, height * 0.5 - 3 * 32}, (vec2){32, height}, COLLISION_LAYER_TERRAIN);         // Mur gauche
        physics_static_body_create((vec2){width - 16, height * 0.5 - 3 * 32}, (vec2){32, height}, COLLISION_LAYER_TERRAIN); // Mur droite
        physics_static_body_create((vec2){32 + 64, height - 32 * 3 - 16}, (vec2){128, 32}, COLLISION_LAYER_TERRAIN);
        physics_static_body_create((vec2){width - 32 - 64, height - 32 * 3 - 16}, (vec2){128, 32}, COLLISION_LAYER_TERRAIN);
        physics_static_body_create((vec2){width * 0.5, height - 32 * 3 - 16}, (vec2){192, 32}, COLLISION_LAYER_TERRAIN);
        physics_static_body_create((vec2){width * 0.5, 32 * 3 + 24}, (vec2){448, 32}, COLLISION_LAYER_TERRAIN);         // Plateforme au centre
        physics_static_body_create((vec2){16, height - 64}, (vec2){32, 64}, COLLISION_LAYER_ENEMY_PASSTHROUGH);         // Spawn ennemis de gauche
        physics_static_body_create((vec2){width - 16, height - 64}, (vec2){32, 64}, COLLISION_LAYER_ENEMY_PASSTHROUGH); // Spawn ennemis de droite

        // Trigger de disparation des ennemis (feu)
        physics_trigger_create((vec2){render_width * 0.5, -4}, (vec2){64, 8}, 0, fire_mask, fire_on_hit);
    }
    entity_create((vec2){render_width * 0.5, 0}, (vec2){32, 64}, (vec2){0, 0}, (vec2){0, 0}, 0, 0, true, anim_fire_id, NULL, NULL, 0, 0, ENTITY_FIRE);
    entity_create((vec2){render_width * 0.5 + 16, -16}, (vec2){32, 64}, (vec2){0, 0}, (vec2){0, 0}, 0, 0, true, anim_fire_id, NULL, NULL, 0, 0, ENTITY_FIRE);
    entity_create((vec2){render_width * 0.5 - 16, -16}, (vec2){32, 64}, (vec2){0, 0}, (vec2){0, 0}, 0, 0, true, anim_fire_id, NULL, NULL, 0, 0, ENTITY_FIRE);
}

static void input_handle(Body *body_player)
{
    if (global.input.escape || global.input.start_controller)
        global.should_quit = true;

    Animation *walk_anim = animation_get(anim_player_walk_id);
    Animation *idle_anim = animation_get(anim_player_idle_id);

    float velx = 0;
    float vely = body_player->velocity[1];

    if (global.input.right || global.input.right_controller || global.input.joystick_right_controller)
    {
        velx += SPEED_PLAYER;
        walk_anim->is_flipped = false;
        idle_anim->is_flipped = false;
    }

    if (global.input.left || global.input.left_controller || global.input.joystick_left_controller)
    {
        velx -= SPEED_PLAYER;
        walk_anim->is_flipped = true;
        idle_anim->is_flipped = true;
    }

    if ((global.input.up || global.input.jump_controller) && player_is_grounded)
    {
        player_is_grounded = false;
        vely = JUMP_VELOCITY;
        audio_sound_play(SOUND_JUMP);
    }

    body_player->velocity[0] = velx;
    body_player->velocity[1] = vely;

    if (global.input.shoot && shoot_timer <= 0)
    {
        Weapon weapon = weapons[weapon_type];
        shoot_timer = weapon.fire_rate;
        spawn_projectile(weapon.projectile_type);
    }
}

int main(int argc, char *argv[])
{

    time_init(60);
    SDL_Window *window = render_init();
    config_init();
    controller_init();
    mycurl_init(&global.curl_handle);
    physics_init();
    entity_init();
    animation_init();
    render_text_init();
    audio_init();
    scenes_init();
    score_init();
  
    SDL_ShowCursor(false); // Cacher le curseur

    // Initialiser audios
    {
        audio_sound_load(&SOUND_JUMP, "assets/audio/jump.wav");
        audio_sound_load(&SOUND_BULLET_HIT_WALL, "assets/audio/bullet_hit_wall.wav");
        audio_sound_load(&SOUND_HURT, "assets/audio/hurt.wav");
        audio_sound_load(&SOUND_ENEMY_DEATH, "assets/audio/enemy_death.wav");
        audio_sound_load(&SOUND_PLAYER_DEATH, "assets/audio/player_death.wav");
        audio_sound_load(&SOUND_SHOOT, "assets/audio/shoot.wav");
        audio_sound_load(&SOUND_FRUIT_COLLECT, "assets/audio/Fruit_collect_1.wav");
        audio_sound_load(&SOUND_GAME_OVER, "assets/audio/Game_Over_sound_effect.mp3");


        
        audio_music_load(&MUSIC_STAGE_1, "assets/audio/breezys_mega_quest_2_stage_1.mp3");
        audio_music_load(&MUSIC_GAME_OVER, "assets/audio/Game_Over_music.mp3");
        audio_music_load(&MUSIC_MENU_PRINCIPAL, "assets/audio/Principal_Menu_music.mp3");

        
    }

    // Initialiser sprites
    {
        render_sprite_sheet_init(&sprite_sheet_player, "assets/spritesheets/player.png", 24, 24);
        render_sprite_sheet_init(&sprite_sheet_map, "assets/spritesheets/level_1_map.png", 640, 360);
        render_sprite_sheet_init(&sprite_sheet_enemy_small, "assets/spritesheets/chicken_run(32x34).png", 32, 34);
        render_sprite_sheet_init(&sprite_sheet_enemy_medium, "assets/spritesheets/bunny_run(34x44).png", 34, 44);
        render_sprite_sheet_init(&sprite_sheet_enemy_large, "assets/spritesheets/rhino_run(52x34).png", 52, 34);
        render_sprite_sheet_init(&sprite_sheet_enemy_flying, "assets/spritesheets/bat_flying(46x30).png", 46, 30);
        render_sprite_sheet_init(&sprite_sheet_fire, "assets/spritesheets/fire.png", 32, 64);
        render_sprite_sheet_init(&sprite_sheet_props, "assets/spritesheets/props_16x16.png", 16, 16);
        render_sprite_sheet_init(&sprite_sheet_apple, "assets/spritesheets/Apple.png", 32, 32);
        render_sprite_sheet_init(&sprite_sheet_banana, "assets/spritesheets/Bananas.png", 32, 32);
        render_sprite_sheet_init(&sprite_sheet_pineapple, "assets/spritesheets/Pineapple.png", 32, 32);
        render_sprite_sheet_init(&sprite_sheet_collected, "assets/spritesheets/Collected.png", 32, 32);
    }

    // Initialiser animations
    {

        size_t adef_player_walk_id = animation_definition_create(&sprite_sheet_player, 0.1, 0, (u8[]){1, 2, 3, 4, 5, 6, 7}, 7);
        size_t adef_player_idle_id = animation_definition_create(&sprite_sheet_player, 0, 0, (u8[]){0}, 1);
        anim_player_walk_id = animation_create(adef_player_walk_id, true);
        anim_player_idle_id = animation_create(adef_player_idle_id, false);

        size_t adef_ennemy_flying_fly_id = animation_definition_create(&sprite_sheet_enemy_flying, 0.1, 0, (u8[]){0, 1, 2, 3, 4, 5, 6}, 7);
        size_t adef_ennemy_small_run_id = animation_definition_create(&sprite_sheet_enemy_small, 0.1, 0, (u8[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}, 14);
        size_t adef_ennemy_medium_run_id = animation_definition_create(&sprite_sheet_enemy_medium, 0.1, 0, (u8[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, 12);
        size_t adef_ennemy_large_run_id = animation_definition_create(&sprite_sheet_enemy_large, 0.1, 0, (u8[]){0, 1, 2, 3, 4, 5}, 6);
        anim_ennemy_small_run_id = animation_create(adef_ennemy_small_run_id, true);
        anim_ennemy_medium_run_id = animation_create(adef_ennemy_medium_run_id, true);
        anim_ennemy_large_run_id = animation_create(adef_ennemy_large_run_id, true);
        anim_ennemy_flying_fly_id = animation_create(adef_ennemy_flying_fly_id, true);

        size_t adef_fire_id = animation_definition_create(&sprite_sheet_fire, 0.1, 0, (u8[]){0, 1, 2, 3, 4, 5, 6}, 7);
        anim_fire_id = animation_create(adef_fire_id, true);

        size_t adef_projectile_small_id = animation_definition_create(&sprite_sheet_props, 1, 0, (u8[]){0}, 1);
        anim_projectile_small_id = animation_create(adef_projectile_small_id, false);

        size_t adef_fruit_apple_id = animation_definition_create(&sprite_sheet_apple, 0.1, 0, (u8[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}, 17);
        size_t adef_fruit_banana_id = animation_definition_create(&sprite_sheet_banana, 0.1, 0, (u8[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}, 17);
        size_t adef_fruit_pineapple_id = animation_definition_create(&sprite_sheet_pineapple, 0.1, 0, (u8[]){0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}, 17);
        size_t adef_fruit_collected_id = animation_definition_create(&sprite_sheet_collected, 0.1, 0, (u8[]){0, 1, 2, 3, 4, 5}, 6);
        anim_fruit_apple_id = animation_create(adef_fruit_apple_id, true);
        anim_fruit_banana_id = animation_create(adef_fruit_banana_id, true);
        anim_fruit_pineapple_id = animation_create(adef_fruit_pineapple_id, true);
        anim_fruit_collected_id = animation_create(adef_fruit_collected_id, false);
    }

    // Initialiser armes
    {
        weapons[WEAPON_TYPE_PISTOL] = (Weapon){
            .projectile_type = PROJECTILE_TYPE_SMALL,
            .projectile_speed = 400,
            .fire_rate = 0.4,
            .recoil = 2.0,
            .projectile_animation_id = anim_projectile_small_id,
            .sprite_size = {16, 16},
            .sprite_offset = {0, 0},
            .sfx = SOUND_SHOOT};
    }

    // Initialiser fruits
    {
        fruits[FRUIT_TYPE_APPLE] = (Fruit){
            .fruit_animation_id = anim_fruit_apple_id,
            .sprite_offset = {0, 0},
            .sprite_size = {32, 32},
        };
        fruits[FRUIT_TYPE_BANANA] = (Fruit){
            .fruit_animation_id = anim_fruit_banana_id,
            .sprite_offset = {0, 0},
            .sprite_size = {32, 32},
        };
        fruits[FRUIT_TYPE_PINEAPPLE] = (Fruit){
            .fruit_animation_id = anim_fruit_pineapple_id,
            .sprite_offset = {0, 0},
            .sprite_size = {32, 32},
        };
    }

    reset();

    while (!global.should_quit)
    {
        time_update();
        switch (global.current_screen)
        {
        case MENU_SCREEN:
            display_menu(window);
            break;
        case SCORE_SCREEN:
            display_score(window);
            break;
        case GAME_OVER_SCREEN:
            display_game_over(window);
            break;
        case USERNAME_MENU_SCREEN:
            display_menu(window);
            break;
        case GAME_SCREEN:
        {
            SDL_Event event;

            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                case SDL_QUIT:
                    global.should_quit = true;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym)
                    {
                    case SDLK_r:
                    {
                        printf("Test de la route /ping avec une requête GET dans un thread dédié.......\n");
                        // Créer une structure pour stocker les données de la requête
                        CurlRequestData *curlData = malloc(sizeof(CurlRequestData));
                        curlData->handle = &global.curl_handle;
                        curlData->endpoint = "/ping";

                        // Créer un thread pour effectuer la requête
                        HANDLE thread = CreateThread(NULL, 0, async_curl_request, curlData, 0, NULL);
                        CloseHandle(thread); // Fermer le handle du thread pour libérer ses ressources lorsqu'il a terminé
                    }
                    break;
                    case SDLK_d:
                        DEBUG = !DEBUG;
                        break;
                    case SDLK_c:
                        CHEAT = !CHEAT;
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
            }

            // Mettre a jour les timers
            {
                spawn_timer -= global.time.delta;
                ground_timer -= global.time.delta;
                shoot_timer -= global.time.delta;
            }

            Entity *player = entity_get(player_id);
            Body *body_player = physics_body_get(player->body_id);

            if (body_player->velocity[0] != 0 || (global.input.left || global.input.right))
                player->animation_id = anim_player_walk_id;
            else
                player->animation_id = anim_player_idle_id;

            input_update();
            input_handle(body_player);
            physics_update();

            animation_update(global.time.delta);

            // Spawn enemies
            {
                if (spawn_timer <= 0)
                {
                    spawn_timer = (float)((rand() % 200) + 200) / 100.f;
                    spawn_timer *= 0.2;

                    bool is_flipped = rand() % 100 >= 50;

                    // Utilisation de la nouvelle fonction spawn_enemy avec des probabilités égales
                    u8 random_value = rand() % 4;

                    switch (random_value)
                    {
                    case 0:
                        spawn_enemy(ENTITY_ENEMY_TYPE_SMALL, false, is_flipped);
                        break;

                    case 1:
                        spawn_enemy(ENTITY_ENEMY_TYPE_MEDIUM, false, is_flipped);
                        break;

                    case 2:
                        spawn_enemy(ENTITY_ENEMY_TYPE_LARGE, false, is_flipped);
                        break;

                    case 3:
                        spawn_enemy(ENTITY_ENEMY_TYPE_FLYING, false, is_flipped);
                        break;

                    default:
                        ERROR_EXIT("Type d'ennemi non défini.");
                        break;
                    }
                }
            }
            // Spawn fruits
            {
                if (fruits_count == 0)
                {
                    fruits_count++;
                    u8 random_fruit_value = rand() % FRUIT_TYPE_COUNT ;
                    switch (random_fruit_value)
                    {
                    case 0:
                        spawn_fruit(FRUIT_TYPE_APPLE);
                        break;

                    case 1:
                        spawn_fruit(FRUIT_TYPE_BANANA);
                        break;

                    case 2:
                        spawn_fruit(FRUIT_TYPE_PINEAPPLE);
                        break;

                    default:
                        ERROR_EXIT("Type de fruits non défini.");
                        break;
                    }
                }
            }

            render_begin();

            if (DEBUG)
            {
                for (size_t i = 0; i < entity_count(); ++i)
                {
                    Entity *entity = entity_get(i);
                    Body *body = physics_body_get(entity->body_id);

                    if (body->is_active)
                    {
                        render_aabb((float *)body, YELLOW);
                    }
                    else
                    {
                        render_aabb((float *)body, RED);
                    }
                }

                for (size_t i = 0; i < physics_static_body_count(); ++i)
                {
                    render_aabb((float *)physics_static_body_get(i), WHITE);
                }
            }

            // Rendre le terrain
            render_sprite_sheet_frame(&sprite_sheet_map, 0, 0, (vec2){render_width / 2.0, render_height / 2.0}, false, (vec4){1, 1, 1, DEBUG ? 0.2 : 1}, texture_slots);

            // Gerer l'animation des entity
            for (size_t i = 0; i < entity_count(); ++i)
            {
                Entity *entity = entity_get(i);
                if (!entity->is_active || entity->animation_id == (size_t)-1)
                {
                    continue;
                }

                Body *body = physics_body_get(entity->body_id);
                Animation *anim = animation_get(entity->animation_id);

                if (body->velocity[0] < 0)
                    anim->is_flipped = true;
                else if (body->velocity[0] > 0)
                    anim->is_flipped = false;

                vec2 pos;
                vec2_add(pos, body->aabb.position, entity->sprite_offset);
                animation_render(anim, pos, WHITE, texture_slots);
                if (entity->entity_type == ENTITY_FX && anim->played)
                {
                    // Detruire le FX s'il a finit de jouer son animation
                    entity->is_active = false;
                    entity_destroy(i);
                }
            }

            render_textures(texture_slots);

            render_text(global.username, body_player->aabb.position[0] - 5, body_player->aabb.position[1] + 20, WHITE, 1);

            // Afficher score
            sprintf(scoreText, "%d", score);
            render_text(scoreText, render_width / 2, render_height - 25, YELLOW, 1);

            // Si Debug
            if (DEBUG)
                render_text("DEBUG", render_width - 50, render_height - 50, ORANGE, 1);
            // Si Cheat
            if (CHEAT)
                render_text("CHEAT", render_width - 50, render_height - 25, RED, 1);

            render_end(window);
            break;
        }
        default:
            ERROR_EXIT("ECRAN INEXISTANT");
            break;
        }

        time_update_late();
    }
    mycurl_cleanup(&global.curl_handle);
    return EXIT_SUCCESS;
}