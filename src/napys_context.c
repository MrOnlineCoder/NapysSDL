#include <napys.h>
#include "napys_internal.h"

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

NapysFontCache *NapysCreateFontCache(TTF_Font *fnt)
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

TTF_Font *NapysQueryFontCache(NapysFontCache *cache, int ptsize)
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

void NapysDestroyFontCache(NapysFontCache *cache)
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

void NapysRegisterCSSColors(NapysContext *ctx)
{
    if (!ctx)
        return;

    NapysRegisterColor(ctx, "black", (SDL_Color){0, 0, 0, 255});
    NapysRegisterColor(ctx, "white", (SDL_Color){255, 255, 255, 255});

    NapysRegisterColor(ctx, "red", (SDL_Color){255, 0, 0, 255});
    NapysRegisterColor(ctx, "green", (SDL_Color){0, 255, 0, 255});
    NapysRegisterColor(ctx, "blue", (SDL_Color){0, 0, 255, 255});
    NapysRegisterColor(ctx, "yellow", (SDL_Color){255, 255, 0, 255});
    NapysRegisterColor(ctx, "cyan", (SDL_Color){0, 255, 255, 255});
    NapysRegisterColor(ctx, "magenta", (SDL_Color){255, 0, 255, 255});
    NapysRegisterColor(ctx, "gray", (SDL_Color){128, 128, 128, 255});
    NapysRegisterColor(ctx, "darkred", (SDL_Color){139, 0, 0, 255});
    NapysRegisterColor(ctx, "darkgreen", (SDL_Color){0, 100, 0, 255});
    NapysRegisterColor(ctx, "darkblue", (SDL_Color){0, 0, 139, 255});
    NapysRegisterColor(ctx, "darkgray", (SDL_Color){169, 169, 169, 255});
    NapysRegisterColor(ctx, "lightgray", (SDL_Color){211, 211, 211, 255});
    NapysRegisterColor(ctx, "orange", (SDL_Color){255, 165, 0, 255});
    NapysRegisterColor(ctx, "purple", (SDL_Color){128, 0, 128, 255});
    NapysRegisterColor(ctx, "pink", (SDL_Color){255, 192, 203, 255});
    NapysRegisterColor(ctx, "brown", (SDL_Color){165, 42, 42, 255});
    NapysRegisterColor(ctx, "gold", (SDL_Color){255, 215, 0, 255});
    NapysRegisterColor(ctx, "silver", (SDL_Color){192, 192, 192, 255});
    NapysRegisterColor(ctx, "lightblue", (SDL_Color){173, 216, 230, 255});
    NapysRegisterColor(ctx, "lightgreen", (SDL_Color){144, 238, 144, 255});

    NapysRegisterColor(ctx, "transparent", (SDL_Color){0, 0, 0, 0}); // Special case for transparency
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

    if (NapysHashmapGetPointer(ctx->registry, font_name) != NULL)
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

    NapysHashmapStorePointer(ctx->fonts, font_name, cache);

    if (!ctx->default_font_cache)
    {
        ctx->default_font_cache = cache;
    }

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

bool NapysRegisterSize(NapysContext *ctx, const char *key, int pt)
{
    if (!ctx || !key || pt < 0)
    {
        return NapysSetError("Invalid context, key, or point size");
    }

    NapysRegistryEntry *entry = SDL_malloc(sizeof(NapysRegistryEntry));
    if (!entry)
    {
        return NapysSetError("Failed to allocate memory for registry entry");
    }

    entry->type = NAPYS_REGISTRY_ENTRY_SIZE;
    entry->str = SDL_strdup(key);
    entry->ptsize = pt;

    NapysHashmapStorePointer(ctx->registry, key, entry);

    return true;
}

bool NapysRegisterImage(NapysContext *ctx, const char *key, void *img)
{
    if (!ctx || !key || !img)
    {
        return NapysSetError("Invalid context, key, or image");
    }

    NapysRegistryEntry *entry = SDL_malloc(sizeof(NapysRegistryEntry));
    if (!entry)
    {
        return NapysSetError("Failed to allocate memory for registry entry");
    }

    entry->type = NAPYS_REGISTRY_ENTRY_IMAGE;
    entry->str = NULL;
    entry->img = img;

    NapysHashmapStorePointer(ctx->registry, key, entry);

    return true;
}