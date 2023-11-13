#pragma once

#include "types.h"

// Une liste d'elements dynamiques
typedef struct array_list
{
    size_t len;       // Nombre d'elements
    size_t capacity;  // Nombre maximum d'elements
    size_t item_size; // Taille en OCTET des elements
    void *items;      // POINTEUR vers les elements stockés
} Array_List;

// Le crud habituel (créer, retirer, etc...)
Array_List *array_list_create(size_t item_size, size_t initial_capacity);
size_t array_list_append(Array_List *list, void *item);
void *array_list_get(Array_List *list, size_t index);
u8 array_list_remove(Array_List *list, size_t index);
