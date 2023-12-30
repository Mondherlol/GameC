#pragma once

// Structure pour un visiteur
typedef struct visitor
{
    char name[10];
    char socket_id[30];
} Visitor;

// Fonction pour initialiser la gestion des visiteurs
void visitors_init();
void addVisitor(const char *name, const char *socket_id);
void removeVisitor(const char *socket_id);
Visitor *getVisitor(const char *socket_id);