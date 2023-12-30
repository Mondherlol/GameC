#include "../visitors.h"
#include "../array_list.h"
#include "../global.h"

void visitors_init()
{
    // Initialise la liste des Visiteurs
    global.visitors = array_list_create(sizeof(Visitor), 10);
}

void addVisitor(const char *name, const char *socket_id)
{
    // Ajoute un Visitor à la liste
    Visitor newVisitor;
    strncpy(newVisitor.name, name, sizeof(newVisitor.name) - 1);
    strncpy(newVisitor.socket_id, socket_id, sizeof(newVisitor.socket_id) - 1);

    newVisitor.name[sizeof(newVisitor.name) - 1] = '\0';           // Assure une terminaison correcte de la chaîne
    newVisitor.socket_id[sizeof(newVisitor.socket_id) - 1] = '\0'; // Assure une terminaison correcte de la chaîne

    array_list_append(global.visitors, &newVisitor);
}

void removeVisitor(const char *socket_id)
{
    // Retire un Visitor de la liste
    size_t len = global.visitors->len;

    for (size_t i = 0; i < len; ++i)
    {
        Visitor *Visitor = array_list_get(global.visitors, i);

        if (strcmp(Visitor->socket_id, socket_id) == 0)
        {
            array_list_remove(global.visitors, i);
            break; // On a trouvé le Visitor, on peut sortir de la boucle
        }
    }
}

Visitor *getVisitor(const char *socket_id)
{
    size_t len = global.visitors->len;

    for (size_t i = 0; i < len; ++i)
    {
        Visitor *visitor = array_list_get(global.visitors, i);

        if (strcmp(visitor->socket_id, socket_id) == 0)
        {
            return visitor; // On a trouvé le Visitor, on le renvoie
        }
    }

    return NULL; // Le Visitor n'a pas été trouvé
}