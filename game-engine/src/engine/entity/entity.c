#include "../array_list.h"
#include "../entity.h"
#include "../animation.h"
#include "../util.h"

static Array_List *entity_list;

void entity_init(void)
{
    entity_list = array_list_create(sizeof(Entity), 0);
}

size_t entity_create(vec2 position, vec2 size, vec2 sprite_offset, vec2 velocity, u8 collision_layer, u8 collision_mask, bool is_kinematic, size_t animation_id, On_Hit on_hit, On_Hit_Static on_hit_static)
{
    size_t id = entity_list->len;

    // Chercher entité inactive
    for (size_t i = 0; i < entity_list->len; ++i)
    {
        Entity *entity = array_list_get(entity_list, i);
        if (!entity->is_active)
        {
            id = i;
            break;
        }
    }

    if (id == entity_list->len)
    {
        if (array_list_append(entity_list, &(Entity){0}) == (size_t)-1) // L'ajouter à la liste de body
            ERROR_EXIT("Erreur lors de l'ajout du Body à la liste\n");
    }

    Entity *entity = entity_get(id);

    *entity = (Entity){
        .is_active = true,
        .animation_id = animation_id,
        .body_id = physics_body_create(
            position,
            size,
            velocity,
            collision_layer,
            collision_mask,
            is_kinematic,
            on_hit,
            on_hit_static),

    };

    return id;
}
Entity *entity_get(size_t id)
{
    return array_list_get(entity_list, id);
}
size_t entity_count()
{
    return entity_list->len;
}

void entity_reset(void)
{
    entity_list->len = 0;
}

size_t entity_id_by_body_id(size_t body_id)
{
    for (size_t i = 0; i < entity_list->len; ++i)
    {
        Entity *entity = entity_get(i);
        if (entity->body_id == body_id)
        {
            return i;
        }
    }

    return (size_t)-1;
}

Entity *entity_by_body_id(size_t body_id)
{
    for (size_t i = 0; i < entity_list->len; ++i)
    {
        Entity *entity = entity_get(i);
        if (entity->body_id == body_id)
        {
            return entity;
        }
    }

    return NULL;
}

void entity_damage(size_t entity_id, u8 amount)
{
    // Blesser l'entité....A faire
}

void entity_destroy(size_t entity_id) // Detruire l'entité
{
    Entity *entity = entity_get(entity_id);

    if (entity->body_id != (size_t)-1)
    {
        physics_body_destroy(entity->body_id);
    }

    if (entity->animation_id != (size_t)-1)
    {
        animation_destroy(entity->animation_id);
    }

    entity->is_active = false;
}
