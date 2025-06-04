#include <napys.h>
#include "napys_internal.h"

#include <SDL3/SDL.h>

static char napys_error[512] = {0};

const char *NapysGetError()
{
    return napys_error;
}

bool NapysSetError(const char *message)
{
    if (message == NULL)
    {
        napys_error[0] = '\0';
    }
    else
    {
        SDL_strlcpy(napys_error, message, sizeof(napys_error));
    }

    return false;
}

NapysHashmap *NapysCreateHashmap()
{
    NapysHashmap *map = SDL_malloc(sizeof(NapysHashmap));
    if (map != NULL)
    {
        map->data = SDL_CreateProperties();
        if (map->data == 0)
        {
            SDL_free(map);
            return NULL;
        }
    }
    return map;
}

void NapysDestroyHashmap(NapysHashmap *map)
{
    if (map != NULL)
    {
        SDL_DestroyProperties(map->data);
        SDL_free(map);
    }
}

void NapysHashmapStorePointer(NapysHashmap *map, const char *key, void *value)
{
    if (map != NULL && key != NULL && value != NULL)
    {
        SDL_SetPointerProperty(map->data, key, value);
    }
}

void *NapysHashmapGetPointer(NapysHashmap *map, const char *key)
{
    if (map != NULL && key != NULL)
    {
        return SDL_GetPointerProperty(map->data, key, NULL);
    }
    return NULL;
}

static void NapysHashmapCallbackWrapper(void *userdata, SDL_PropertiesID props, const char *name)
{
    NapysHashmapCallback callback = (NapysHashmapCallback)userdata;

    callback(name, SDL_GetPointerProperty(props, name, NULL), userdata);
}

void *NapysIterateHashmap(NapysHashmap *map, NapysHashmapCallback callback, void *userdata)
{
    if (map == NULL || callback == NULL)
    {
        return NULL;
    }

    SDL_EnumerateProperties(map->data, NapysHashmapCallbackWrapper, callback);
    return NULL;
}