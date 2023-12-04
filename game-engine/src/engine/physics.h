#pragma once

#include <linmath.h>
#include <stdbool.h>

#include "types.h"

typedef struct hit Hit;
typedef struct body Body;
typedef struct static_body Static_Body;

typedef void (*On_Hit)(Body *self, Body *other, Hit hit);               // Lorsqu'une collision a lieu entre deux Body mobile
typedef void (*On_Hit_Static)(Body *self, Static_Body *other, Hit hit); // Pareil mais entre un body static et un un mobile

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
struct body
{
    AABB aabb;         // Boite englobant le corps
    vec2 velocity;     // Sa vélocité -> La vitesse de l'obet dans une direction
    vec2 acceleration; // Son acceleration -> C'est les variations de vitesse et de direction à venir de l'objet en gros
    On_Hit on_hit;
    On_Hit_Static on_hit_static;
    u8 collision_layer; // Plan de collision, en gros quelle catégorie de collision peut il donner
    u8 collision_mask;  // Pour définir quelles collisions sont autorisées
    bool is_kinematic ;// Pour rester au meme endroit et ne pas etre affectés par la gravité ou un corp statique
    bool is_active ;//Pour la réutilisation  
};

// Un corp statique pour les objets innanimés (murs, etc...)
struct static_body
{
    AABB aabb;
    u8 collision_layer;
};

struct hit
{
    size_t other_id; // Id de l'autre body avec qui y a eu collision
    bool is_hit;     // Y a t-il eu une collision
    float time;      // Temps de la collision
    vec2 position;   // Position de la collision
    vec2 normal;     // Vecteur qui pointe vers la direction perpendiculaire à la surface de l'impact
    //                  Ca permet de determiner la reflexion du corps lors de l'impact
};

void physics_init(void);
void physics_update(void);
size_t physics_body_create(vec2 position, vec2 size, vec2 velocity, u8 collision_layer, u8 collision_mask,bool is_kinematic, On_Hit on_hit, On_Hit_Static on_hit_static); // Crée et renvoie l'index d'un Body
Body *physics_body_get(size_t index);                                                                                                                   // Récupérer un body de la liste à un index donné

size_t physics_static_body_create(vec2 position, vec2 size, u8 collision_layer); // Crée un body static et renvoie son index
Static_Body *physics_static_body_get(size_t index);                              // Récupére un static body à un index donné

bool physics_point_intersect_aabb(vec2 point, AABB aabb); // Collision entre un point et une boite
bool physics_aabb_intersect_aabb(AABB a, AABB b);         // Collision entre deux boites
AABB aabb_minkowski_difference(AABB a, AABB b);
void aabb_penetration_vector(vec2 r, AABB aabb);
void aabb_min_max(vec2 min, vec2 max, AABB aabb);
Hit ray_intersect_aabb(vec2 position, vec2 magnitude, AABB aabb);
