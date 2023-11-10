#pragma once

#include <stdio.h>

// Erreur et fermer le programme
#define ERROR_EXIT(...)               \
    {                                 \
        fprintf(stderr, __VA_ARGS__); \
        exit(1);                      \
    }
// Erreur sans fermer le programme
#define ERROR_RETURN(R, ...)          \
    {                                 \
        fprintf(stderr, __VA_ARGS__); \
        return R;                     \
    }
