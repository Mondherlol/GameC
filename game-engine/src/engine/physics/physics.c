#include "../global.h"
#include "../array_list.h"
#include "../util.h"
#include "../physics.h"
#include "physics_internal.h"

static Physics_State_Internal state;

static u32 iterations = 1;
static float tic_rate;

// Initalise le min et le max d'un aabb
// min -> sommet bas gauche, max -> sommet haut droite
void aabb_min_max(vec2 min, vec2 max, AABB aabb)
{
    vec2_sub(min, aabb.position, aabb.half_size); // Met dans min la valeur min, retire de la position la half_size
    vec2_add(max, aabb.position, aabb.half_size);
}

// En gros ça vérifie si un point est DANS ou touche une boite aabb donné
bool physics_point_intersect_aabb(vec2 point, AABB aabb)
{
    vec2 min, max;
    aabb_min_max(min, max, aabb); // On récupere le min et max du aabb

    // On retourne si le point touche ou est dans le AABB

    return point[0] >= min[0] && // Le point doit etre au moins apres le minimum sur le plan X
           point[0] <= max[0] && // ET au moins avant le maximum
           point[1] >= min[1] && // ET au moins apres le minimum sur le plan Y
           point[1] <= max[1];   // ET au moins avant le maxmimum
}

void physics_init(void)
{
    state.body_list = array_list_create(sizeof(Body), 0); // On crée une liste vide qui va contenir des struct Body
    state.static_body_list = array_list_create(sizeof(Static_Body), 0);

    state.gravity = -200;
    state.terminal_velocity = -10000;

    tic_rate = 1.f / iterations;
}

static void update_sweep_result(Hit *result, Body *body, size_t other_id, vec2 velocity)
{
    Body *other = physics_body_get(other_id);

    if ((body->collision_mask & other->collision_layer) == 0)
    {
        return;
    }

    AABB sum_aabb = other->aabb;
    vec2_add(sum_aabb.half_size, sum_aabb.half_size, body->aabb.half_size);

    Hit hit = ray_intersect_aabb(body->aabb.position, velocity, sum_aabb);
    if (hit.is_hit)
    {
        if (body->on_hit && (body->collision_mask & other->collision_layer) == 0)
        {
            body->on_hit(body, other, hit);
        }

        if (hit.time < result->time)
        {
            *result = hit;
        }
        else if (hit.time == result->time)
        {
            // Solve highest velocity axis first.
            if (fabsf(velocity[0]) > fabsf(velocity[1]) && hit.normal[0] != 0)
            {
                *result = hit;
            }
            else if (fabsf(velocity[1]) > fabsf(velocity[0]) && hit.normal[1] != 0)
            {
                *result = hit;
            }
        }

        result->other_id = other_id;
    }
}

static void update_sweep_result_static(Hit *result, Body *body, size_t other_id, vec2 velocity)
{
    Static_Body *static_body = physics_static_body_get(other_id);

    if ((body->collision_mask & static_body->collision_layer) == 0)
    {
        return;
    }

    AABB sum_aabb = static_body->aabb;
    vec2_add(sum_aabb.half_size, sum_aabb.half_size, body->aabb.half_size);

    Hit hit = ray_intersect_aabb(body->aabb.position, velocity, sum_aabb);
    if (hit.is_hit)
    {
        if (hit.time < result->time)
        {
            *result = hit;
        }
        else if (hit.time == result->time)
        {
            // Solve highest velocity axis first.
            if (fabsf(velocity[0]) > fabsf(velocity[1]) && hit.normal[0] != 0)
            {
                *result = hit;
            }
            else if (fabsf(velocity[1]) > fabsf(velocity[0]) && hit.normal[1] != 0)
            {
                *result = hit;
            }
        }

        result->other_id = other_id;
    }
}

static Hit sweep_static_bodies(Body *body, vec2 velocity)
{
    Hit result = {.time = 0xBEEF};

    for (u32 i = 0; i < state.static_body_list->len; ++i)
    {
        update_sweep_result_static(&result, body, i, velocity);
    }

    return result;
}

static Hit sweep_bodies(Body *body, vec2 velocity)
{
    Hit result = {.time = 0xBEEF};

    for (u32 i = 0; i < state.body_list->len; ++i)
    {
        Body *other = physics_body_get(i);

        if (body == other)
        {
            continue;
        }

        update_sweep_result(&result, body, i, velocity);
    }

    return result;
}

static void sweep_response(Body *body, vec2 velocity)
{
    Hit hit = sweep_static_bodies(body, velocity);
    Hit hit_moving = sweep_bodies(body, velocity);

    if (hit_moving.is_hit)
    {
        if (body->on_hit != NULL)
        {
            body->on_hit(body, physics_body_get(hit_moving.other_id), hit_moving);
        }
    }
    if (hit.is_hit)
    {
        body->aabb.position[0] = hit.position[0];
        body->aabb.position[1] = hit.position[1];

        if (hit.normal[0] != 0)
        {
            body->aabb.position[1] += velocity[1];
            body->velocity[0] = 0;
        }
        else if (hit.normal[1] != 0)
        {
            body->aabb.position[0] += velocity[0];
            body->velocity[1] = 0;
        }
        if (body->on_hit_static != NULL)
        {
            body->on_hit_static(body, physics_static_body_get(hit.other_id), hit);
        }
    }
    else
    {
        vec2_add(body->aabb.position, body->aabb.position, velocity);
    }
}

static void stationnary_response(Body *body)
{
    for (u32 i = 0; i < state.static_body_list->len; ++i)
    {
        Static_Body *static_body = physics_static_body_get(i);

        AABB aabb = aabb_minkowski_difference(static_body->aabb, body->aabb);

        vec2 min, max;
        aabb_min_max(min, max, aabb);

        if (min[0] <= 0 && max[0] >= 0 && min[1] <= 0 && max[1] >= 0)
        {
            vec2 penetration_vector;
            aabb_penetration_vector(penetration_vector, aabb);

            vec2_add(body->aabb.position, body->aabb.position, penetration_vector);
        }
    }
}
void physics_update(void)
{
    Body *body;

    for (u32 i = 0; i < state.body_list->len; ++i) // Pour tous les body dans la liste
    {
        body = array_list_get(state.body_list, i);

        body->velocity[1] += state.gravity;
        if (state.terminal_velocity > body->velocity[1])
        {
            body->velocity[1] = state.terminal_velocity;
        }

        body->velocity[0] += body->acceleration[0];
        body->velocity[0] += body->acceleration[1];

        vec2 scaled_velocity;
        vec2_scale(scaled_velocity, body->velocity, global.time.delta * tic_rate);

        for (u32 j = 0; j < iterations; ++j)
        {
            sweep_response(body, scaled_velocity);
            stationnary_response(body);
        }

        // body->velocity[0] += body->acceleration[0] * global.time.delta;  // Incrémenter la velocité selon l'acceleration et le temps écoulé
        // body->velocity[1] += body->acceleration[1] * global.time.delta;  // Pareil pour la velocité y

        // body->aabb.position[0] += body->velocity[0] * global.time.delta; // Déplacer l'objet selon la velocité
        // body->aabb.position[1] += body->velocity[0] * global.time.delta; // Pareil pour la position y
    }
}
size_t physics_body_create(vec2 position, vec2 size, vec2 velocity, u8 collision_layer, u8 collision_mask, On_Hit on_hit, On_Hit_Static on_hit_static)
{
    Body body = {
        .aabb = {
            .position = {position[0], position[1]},
            .half_size = {size[0] * 0.5, size[1] * 0.5}},
        .velocity = {velocity[0], velocity[1]}, // Vélocité passé en paramètre
        .collision_layer = collision_layer,
        .collision_mask = collision_mask,
        .on_hit = on_hit,
        .on_hit_static = on_hit_static,
    };
    if (array_list_append(state.body_list, &body) == (size_t)-1) // L'ajouter à la liste de body
        ERROR_EXIT("Erreur lors de l'ajout du Body à la liste\n");

    return state.body_list->len - 1; // Renvoyer l'id du dernier body ajouté
}
Body *physics_body_get(size_t index)
{
    return array_list_get(state.body_list, index);
}

size_t physics_static_body_create(vec2 position, vec2 size, u8 collision_layer)
{
    Static_Body static_body = {
        .aabb = {
            .position = {position[0], position[1]},
            .half_size = {size[0] * 0.5, size[1] * 0.5}},
        .collision_layer = collision_layer,
    };

    if (array_list_append(state.static_body_list, &static_body) == (size_t)-1) // L'ajouter à la liste de static body
        ERROR_EXIT("Erreur lors de l'ajout du Static Body à la liste\n");

    return state.static_body_list->len - 1; // Renvoyer l'id du dernier static body ajouté
}

Static_Body *physics_static_body_get(size_t index)
{
    return array_list_get(state.static_body_list, index);
}

bool physics_aabb_intersect_aabb(AABB a, AABB b)
{
    vec2 min, max;
    aabb_min_max(min, max, aabb_minkowski_difference(a, b));

    return (min[0] <= 0 && max[0] >= 0 && min[1] <= 0 && max[1] >= 0);
}
AABB aabb_minkowski_difference(AABB a, AABB b)
{
    AABB result;
    vec2_sub(result.position, a.position, b.position);
    vec2_add(result.half_size, a.half_size, b.half_size);

    return result;
}
void aabb_penetration_vector(vec2 r, AABB aabb)
{
    vec2 min, max;
    aabb_min_max(min, max, aabb);

    float min_dist = fabsf(min[0]);
    r[0] = min[0];
    r[1] = 0;

    if (fabsf(max[0]) < min_dist)
    {
        min_dist = fabsf(max[0]);
        r[0] = max[0];
    }

    if (fabsf(min[1]) < min_dist)
    {
        min_dist = fabsf(min[1]);
        r[0] = 0;
        r[1] = min[1];
    }

    if (fabsf(max[1]) < min_dist)
    {
        r[0] = 0;
        r[1] = max[1];
    }
}

Hit ray_intersect_aabb(vec2 position, vec2 magnitude, AABB aabb)
{
    Hit hit = {0};
    vec2 min, max;
    aabb_min_max(min, max, aabb); // Calculer le min et le max du aabb

    float last_entry = -INFINITY;
    float first_exit = INFINITY;

    for (u8 i = 0; i < 2; ++i)
    {
        if (magnitude[i] != 0)
        {
            float t1 = (min[i] - position[i]) / magnitude[i];
            float t2 = (max[i] - position[i]) / magnitude[i];

            last_entry = fmaxf(last_entry, fminf(t1, t2));
            first_exit = fminf(first_exit, fmax(t1, t2));
        }
        else if (position[i] <= min[i] || position[i] >= max[i])
        {
            return hit;
        }
    }
    if (first_exit > last_entry && first_exit > 0 && last_entry < 1)
    {
        hit.position[0] = position[0] + magnitude[0] * last_entry;
        hit.position[1] = position[1] + magnitude[1] * last_entry;

        hit.is_hit = true;
        hit.time = last_entry;

        float dx = hit.position[0] - aabb.position[0];
        float dy = hit.position[1] - aabb.position[1];
        float px = aabb.half_size[0] - fabs(dx);
        float py = aabb.half_size[1] - fabs(dy);

        if (px < py)
        {
            hit.normal[0] = (dx > 0) - (dx < 0);
        }
        else
        {
            hit.normal[1] = (dy > 0) - (dy < 0);
        }
    }

    return hit;
}