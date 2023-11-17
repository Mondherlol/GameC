#pragma once

#include <linmath.h>
#include <stdbool.h>

#include "types.h"

// Boite allignée avec les axes de la fenêtre
typedef struct aabb
{
    vec2 position;  // Position du CENTRE de la boite (x,y)
    vec2 half_size; // Moitié de la largeur et de la hauteur
} AABB;
/*
    On stocke les vec de la moitié de la taille pour simplifier les calculs de colisions
    Par exemple un AABB défini tel que :
         position ( x , y ) et half_size( demi_hauteur , demi_largeur ),
    On peut déduire les coordonnées du coin inferieur gauche facilement avec :
         ( x - demi_largeur, y - demi_hauteur )
*/

// Un corps (Body) qui représente un objet de jeu affecté par la physique
typedef struct body
{
    AABB aabb;         // Boite englobant le corps
    vec2 velocity;     // Sa vélocité -> La vitesse de l'obet dans une direction
    vec2 acceleration; // Son acceleration -> C'est les variations de vitesse et de direction à venir de l'objet en gros
} Body;

typedef struct static_body
{
    AABB aabb;
} Static_Body;

typedef struct hit
{
    bool is_hit;
    float time;
    vec2 position;
    vec2 normal;
} Hit;

void physics_init(void);
void physics_update(void);
size_t physics_body_create(vec2 position, vec2 size); // Crée et renvoie l'index d'un Body
Body *physics_body_get(size_t index);                 // Récupérer un body de la liste à un index donné

size_t physics_static_body_create(vec2 position, vec2 size); // Crée un body static et renvoie son index
Static_Body *physics_static_body_get(size_t index);          // Récupére un static body à un index donné

bool physics_point_intersect_aabb(vec2 point, AABB aabb);
bool physics_aabb_intersect_aabb(AABB a, AABB b);
AABB aabb_minkowski_difference(AABB a, AABB b);
void aabb_penetration_vector(vec2 r, AABB aabb);
void aabb_min_max(vec2 min, vec2 max, AABB aabb);
Hit ray_intersect_aabb(vec2 position, vec2 magnitude, AABB aabb);
