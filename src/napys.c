#include <napys.h>

#include <SDL3/SDL.h>

static char napys_error[512] = {0};

typedef struct
{
    SDL_PropertiesID data;
} NapysHashmap;

typedef void (*NapysHashmapCallback)(const char *key, void *value, void *userdata);

const char *NapysGetError()
{
    return napys_error;
}

static bool NapysSetError(const char *message)
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

static NapysHashmap *NapysCreateHashmap()
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

static void NapysDestroyHashmap(NapysHashmap *map)
{
    if (map != NULL)
    {
        SDL_DestroyProperties(map->data);
        SDL_free(map);
    }
}

static void NapysHashmapStorePointer(NapysHashmap *map, const char *key, void *value)
{
    if (map != NULL && key != NULL && value != NULL)
    {
        SDL_SetPropertyPointer(map->data, key, value);
    }
}

static void *NapysHashmapGetPointer(NapysHashmap *map, const char *key)
{
    if (map != NULL && key != NULL)
    {
        return SDL_GetPropertyPointer(map->data, key);
    }
    return NULL;
}

static void *NapysIterateHashmap(NapysHashmap *map, NapysHashmapCallback callback, void *userdata)
{
    if (map == NULL || callback == NULL)
    {
        return NULL;
    }

    SDL_EnumerateProperties(map->data, callback, map);
    return NULL;
}

static NapysFontCache *NapysCreateFontCache(TTF_Font *fnt)
{
    NapysFontCache *cache = SDL_malloc(sizeof(NapysFontCache));

    if (!cache)
    {
        NapysSetError("Failed to allocate memory for font cache");
        return NULL;
    }

    cache->base = fnt;

    for (int i = 0; i < NAPYS_MAX_FONT_SIZE; i++)
    {
        cache->sizes[i] = NULL;
    }

    int ptsize = TTF_GetFontSize(fnt);

    cache->sizes[ptsize] = fnt;
    return cache;
}

static TTF_Font *NapysQueryFontCache(NapysFontCache *cache, int ptsize)
{
    if (!cache || ptsize < 0 || ptsize >= NAPYS_MAX_FONT_SIZE)
    {
        return NULL;
    }

    // If the requested size is already cached, return it
    if (cache->sizes[ptsize])
    {
        return cache->sizes[ptsize];
    }

    TTF_Font *new_font = TTF_CopyFont(cache->base);

    if (!new_font)
    {
        NapysSetError("Failed to copy font");
        return NULL;
    }

    TTF_SetFontSize(new_font, ptsize);
    cache->sizes[ptsize] = new_font;

    return new_font;
}

static void NapysDestroyFontCache(NapysFontCache *cache)
{
    if (cache)
    {
        for (int i = 0; i < NAPYS_MAX_FONT_SIZE; i++)
        {
            if (cache->sizes[i] && cache->sizes[i] != cache->base)
            {
                TTF_CloseFont(cache->sizes[i]);
            }
        }
        SDL_free(cache);
    }
}

NapysContext *NapysCreateContext()
{
    NapysContext *ctx = SDL_malloc(sizeof(NapysContext));

    if (!ctx)
        return NULL;

    ctx->registry = NapysCreateHashmap();
    ctx->fonts = NapysCreateHashmap();

    if (!ctx->registry || !ctx->fonts)
    {
        NapysSetError("Failed to create context: could not allocate hashmaps");

        SDL_free(ctx);
        return NULL;
    }

    return ctx;
}

static void NapysDestroyFontCacheCallback(const char *key, void *value, void *userdata)
{
    if (value != NULL)
    {
        NapysFontCache *cache = (NapysFontCache *)value;
        NapysDestroyFontCache(cache);
    }
}

void NapysDestroyContext(NapysContext *ctx)
{
    if (ctx != NULL)
    {
        if (ctx->registry != NULL)
        {
            NapysDestroyHashmap(ctx->registry);
        }

        if (ctx->fonts != NULL)
        {
            NapysIterateHashmap(ctx->fonts, NapysDestroyFontCacheCallback, NULL);
            NapysDestroyHashmap(ctx->fonts);
        }

        SDL_free(ctx);
    }
}

bool NapysRegisterFont(NapysContext *ctx, TTF_Font *font, const char *name)
{
    if (!font)
    {
        return NapysSetError("Invalid font pointer");
    }

    if (!ctx)
    {
        return NapysSetError("Invalid context pointer");
    }

    char *font_name = NULL;

    if (name)
    {
        font_name = SDL_strdup(name);
    }
    else if (!font_name)
    {
        const char *ttf_family = TTF_GetFontFamilyName(font);

        if (ttf_family)
        {
            font_name = SDL_strdup(ttf_family);
        }
    }

    if (!font_name)
    {
        return NapysSetError("Cannot determine font name");
    }

    if (SDL_GetPropertyPointer(ctx->registry->data, font_name) != NULL)
    {
        SDL_free(font_name);
        return NapysSetError("Font already registered");
    }

    NapysFontCache *cache = NapysCreateFontCache(font);

    if (!cache)
    {
        SDL_free(font_name);
        return false;
    }

    NapysHashmapStorePointer(ctx->registry, font_name, cache);

    SDL_free(font_name);

    return true;
}

bool NapysRegisterString(NapysContext *ctx, const char *key, const char *value)
{
    if (!ctx || !key || !value)
    {
        return NapysSetError("Invalid context, key, or value");
    }

    NapysRegistryEntry *entry = SDL_malloc(sizeof(NapysRegistryEntry));
    if (!entry)
    {
        return NapysSetError("Failed to allocate memory for registry entry");
    }

    entry->str = SDL_strdup(value);
    entry->type = NAPYS_REGISTRY_ENTRY_STRING;

    NapysHashmapStorePointer(ctx->registry, key, entry);

    return true;
}

bool NapysRegisterColor(NapysContext *ctx, const char *key, SDL_Color color)
{
    if (!ctx || !key)
    {
        return NapysSetError("Invalid context or key");
    }

    NapysRegistryEntry *entry = SDL_malloc(sizeof(NapysRegistryEntry));
    if (!entry)
    {
        return NapysSetError("Failed to allocate memory for registry entry");
    }

    entry->color = color;
    entry->type = NAPYS_REGISTRY_ENTRY_COLOR;

    NapysHashmapStorePointer(ctx->registry, key, entry);

    return true;
}

NapysCommandList *NapysCreateCommandList()
{
    NapysCommandList *list = SDL_malloc(sizeof(NapysCommandList));

    if (!list)
    {
        NapysSetError("Failed to allocate memory for command list");
        return NULL;
    }

    list->cmds = NULL;
    list->cmd_count = 0;
    list->cmd_capacity = 0;

    return list;
}

void NapysClearCommandList(NapysCommandList *list)
{
    if (list)
    {
        for (int i = 0; i < list->cmd_count; i++)
        {
            if (list->cmds[i].data)
            {
                SDL_free(list->cmds[i].data);
            }
        }
        list->cmd_count = 0;
    }
}

void NapysDestroyCommandList(NapysCommandList *list)
{
    if (list != NULL)
    {
        if (list->cmds != NULL)
        {
            NapysClearCommandList(list);
            SDL_free(list->cmds);
        }
        SDL_free(list);
    }
}

bool NapysAddCommand(NapysCommandList *list, NapysCommand cmd)
{
    if (list == NULL)
    {
        return NapysSetError("Invalid command list");
    }

    if (list->cmd_count >= list->cmd_capacity)
    {
        int new_capacity = list->cmd_capacity == 0 ? 4 : list->cmd_capacity * 2;
        NapysCommand *new_cmds = SDL_realloc(list->cmds, new_capacity * sizeof(NapysCommand));
        if (!new_cmds)
        {
            return NapysSetError("Failed to allocate memory for commands");
        }
        list->cmds = new_cmds;
        list->cmd_capacity = new_capacity;
    }

    list->cmds[list->cmd_count] = cmd;
    list->cmd_count++;

    return true;
}

bool NapysAddDrawTextCommand(NapysCommandList *list, const char *text)
{
    if (!list || !text)
    {
        return NapysSetError("Invalid command list or text");
    }

    NapysCommand cmd;
    cmd.type = NAPYS_COMMAND_TYPE_DRAW_TEXT;
    cmd.data = SDL_strdup(text);

    return NapysAddCommand(list, cmd);
}

bool NapysAddSetColorCommand(NapysCommandList *list, const char *color_name)
{
    if (!list || color_name)
        return NapysSetError("Invalid command list");

    NapysCommand cmd;
    cmd.type = NAPYS_COMMAND_TYPE_SET_COLOR;
    cmd.data = SDL_strdup(color_name);

    return NapysAddCommand(list, cmd);
}