#include <stdio.h>
#include <stdlib.h>
#include <errno.h> //Pour gestion des erreurs

#include "../types.h"
#include "../util.h"
#include "../io.h"

// Un chunk = 4096 bits
#define MAX_CHUNK_SIZE 2097152 // Equivalent à 20mb environ
#define IO_READ_ERROR_GENERALE "Erreur lors de la lecture du fichier : %s. Erreur num : %d \n "
#define IO_READ_ERROR_MEMOIRE "Mempire saturee pour lire le fichier : %s \n"

File io_file_read(const char *path)
{
    File file = {.is_valid = false};

    FILE *fp = fopen(path, "rb"); // Ouvrir en lecture binaire
    if (!fp || ferror(fp))
    {
        ERROR_RETURN(file, IO_READ_ERROR_GENERALE, path, errno);
    }

    char *data = NULL;
    char *tmp;
    size_t used = 0;
    size_t size = 0;
    size_t n;

    while (true)
    {
        if (used + MAX_CHUNK_SIZE + 1 > size)
        {
            size = used + MAX_CHUNK_SIZE + 1;

            if (size <= used)
            {
                free(data);
                ERROR_RETURN(file, "Fichier trop gros (MAX 20MB) : %s ", path);
            }
            tmp = realloc(data, size);
            if (!tmp)
            {
                free(data);
                ERROR_RETURN(file, IO_READ_ERROR_MEMOIRE, path);
            }

            data = tmp;
        }

        n = fread(data + used, 1, MAX_CHUNK_SIZE, fp);
        if (n == 0)
            break;

        used += n;
    }
    if (ferror(fp))
    {
        free(data);
        ERROR_RETURN(file, IO_READ_ERROR_GENERALE, path, errno);
    }

    tmp = realloc(data, used + 1);

    if (!tmp)
    {
        free(data);
        ERROR_RETURN(file, IO_READ_ERROR_MEMOIRE, path);
    }
    data = tmp;
    data[used] = 0;

    file.data = data;
    file.len = used;
    file.is_valid = true;

    return file;
}

int io_file_write(void *buffer, size_t size, const char *path)
{
    FILE *fp = fopen(path, "wb");
    if (!fp || ferror(fp))
        ERROR_RETURN(1, "Impossible d'ecrire dans le fichier : %s.\n", path);

    size_t chunks_written = fwrite(buffer, size, 1, fp);

    fclose(fp);

    if (chunks_written != 1)
        ERROR_RETURN(1, "Erreur d'ecriture."
                        "1 Chunk attendu, reçu : %zu.\n",
                     chunks_written);

    return 0;
}

int io_file_append(void *buffer, size_t size, const char *path)
{
    FILE *fp = fopen(path, "ab");
    if (!fp || ferror(fp))
        ERROR_RETURN(1, "Impossible d'ecrire dans le fichier : %s.\n", path);

    size_t chunks_written = fwrite(buffer, size, 1, fp);

    fclose(fp);

    if (chunks_written != 1)
        ERROR_RETURN(1, "Erreur d'ecriture."
                        "1 Chunk attendu, reçu : %zu.\n",
                     chunks_written);

    return 0;
}