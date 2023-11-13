#include "../global.h"
#include "../array_list.h"
#include "../util.h"
#include "../physics.h"
#include "physics_internal.h"

static Physics_State_Internal state;

void physics_init(void)
{
    state.body_list = array_list_create(sizeof(Body), 0); // On crée une liste vide qui va contenir des struct Body
}
void physics_update(void)
{
    Body *body;

    for (u32 i = 0; i < state.body_list->len; ++i) // Pour tous les body dans la liste
    {
        body = array_list_get(state.body_list, i);
        body->velocity[0] += body->acceleration[0] * global.time.delta;  // Incrémenter la velocité selon l'acceleration et le temps écoulé
        body->velocity[1] += body->acceleration[1] * global.time.delta;  // Pareil pour la velocité y
        body->aabb.position[0] += body->velocity[0] * global.time.delta; // Déplacer l'objet selon la velocité
        body->aabb.position[1] += body->velocity[0] * global.time.delta; // Pareil pour la position y
    }
}
size_t physics_body_create(vec2 position, vec2 size)
{
    Body body = {
        .aabb = {
            .position = {position[0], position[1]},
            .half_size = {size[0] * 0.5, size[1] * 0.5}},
        .velocity = {0, 0}}; // Par défaut objet immobile

    if (array_list_append(state.body_list, &body) == (size_t)-1) // L'ajouter à la liste de body
        ERROR_EXIT("Erreur lors de l'ajout du Body à la liste\n");

    return state.body_list->len - 1; // Renvoyer l'id du dernier body ajouté
}
Body *physics_body_get(size_t index)
{
    return array_list_get(state.body_list, index);
}