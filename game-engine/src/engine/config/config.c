#include <string.h>

#include "../global.h"
#include "../io.h"
#include "../util.h"
#include "../input.h"

static const char *CONFIG_DEFAULT =
    "[player]\n"
    "username = motaru\n"
    "[controls]\n"
    "up = UP\n"
    "down = DOWN\n"
    "left = LEFT\n"
    "right = RIGHT\n"
    "shoot = X\n"
    "escape = Escape\n"
    "\n";

static char tmp_buffer[20] = {0};

static char *config_get_value(const char *config_buffer, const char *value)
{
    char *line = strstr(config_buffer, value);
    if (!line)
        ERROR_EXIT("Config de %s introuvable."
                   "Tentez de supprimer config.ini et relancer le jeu.\n",
                   value);
    size_t len = strlen(line);
    char *end = line + len;

    char *curr = line;
    char *tmp_ptr = &tmp_buffer[0];
    // Placer le pointeur sur '='
    while (*curr != '=' && curr != end)
        ++curr;
    // Passer le '='
    ++curr;
    // Passer les espaces
    while (*curr == ' ')
        ++curr;
    // Récuperer tous les char jusqu'a fin de ligne
    while (*curr != '\n' && *curr != 0 && curr != end)
        *tmp_ptr++ = *curr++;

    *tmp_ptr = 0;

    return tmp_buffer;
}
static void load_controls(const char *config_buffer)
{
    config_key_bind(INPUT_KEY_LEFT, config_get_value(config_buffer, "left"));
    config_key_bind(INPUT_KEY_RIGHT, config_get_value(config_buffer, "right"));
    config_key_bind(INPUT_KEY_UP, config_get_value(config_buffer, "up"));
    config_key_bind(INPUT_KEY_DOWN, config_get_value(config_buffer, "down"));
    config_key_bind(INPUT_KEY_SHOOT, config_get_value(config_buffer, "shoot"));
    config_key_bind(INPUT_KEY_ESCAPE, config_get_value(config_buffer, "escape"));
}
static void load_username(const char *config_buffer)
{
    const char *username_value = config_get_value(config_buffer, "username");
    printf("username value = %s", username_value);
    if (username_value != NULL && strlen(username_value) <= 6)
        strncpy(global.username, username_value, 6);
    else
        strncpy(global.username, "Anon", 6);

    global.username[6] = '\0';
}

static int config_load(void)
{
    File file_config = io_file_read("./config.ini");
    if (!file_config.is_valid)
        return 1;

    load_controls(file_config.data);
    load_username(file_config.data);
    free(file_config.data);

    return 0;
}

void config_init(void)
{
    if (config_load() == 0)
        return;

    io_file_write((void *)CONFIG_DEFAULT, strlen(CONFIG_DEFAULT), "./config.ini");

    if (config_load() != 0)
        ERROR_EXIT("Impossible de charger ou creer config.ini.\n");
}

void config_key_bind(Input_Key key, const char *key_name)
{
    SDL_Scancode scan_code = SDL_GetScancodeFromName(key_name);

    if (scan_code == SDL_SCANCODE_UNKNOWN)
        ERROR_RETURN(, " Erreur d'assignation de touche. Touche non reconnue : %s \n", key_name);

    global.config.keybinds[key] = scan_code;
}