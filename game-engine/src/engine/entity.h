#pragma once

#include <stdbool.h>
#include <linmath.h>

#include "physics.h"
#include "types.h"

typedef struct entity
{
    size_t body_id;
    size_t animation_id;
    vec2 sprite_offset; // Decalage du spirte par rapport a la position du corps
    u32 texture_slot;   // Emplacement de sa texture pour le rendu
    bool is_active;
    bool is_enraged;
    u8 health; // Points de vie
    float speed;

} Entity;

void entity_init(void);
size_t entity_create(vec2 position, vec2 size, vec2 sprite_offset, vec2 velocity, u8 collision_layer, u8 collision_mask, bool is_kinematic, size_t animation_id, On_Hit on_hit, On_Hit_Static on_hit_static, float health, float speed);
Entity *entity_get(size_t id); //  Obtenir un pointeur vers une entité à partir de son identifiant
size_t entity_count();
void entity_reset(void);

Entity *entity_by_body_id(size_t body_id);   //  Obtenir un pointeur vers une entité à partir de son body id
size_t entity_id_by_body_id(size_t body_id); // Fonction pour obtenir l'id d'une entité à partir de son body id

bool entity_damage(size_t entity_id, u8 amount); // Infliger des degats
void entity_destroy(size_t entity_id);