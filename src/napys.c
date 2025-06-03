#include <napys.h>

#include <SDL3/SDL.h>

static char napys_error[512] = {0};

typedef struct NapysHashmap
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
        SDL_SetPointerProperty(map->data, key, value);
    }
}

static void *NapysHashmapGetPointer(NapysHashmap *map, const char *key)
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

static void *NapysIterateHashmap(NapysHashmap *map, NapysHashmapCallback callback, void *userdata)
{
    if (map == NULL || callback == NULL)
    {
        return NULL;
    }

    SDL_EnumerateProperties(map->data, NapysHashmapCallbackWrapper, callback);
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
    if (!list)
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
    if (!list || !color_name)
        return NapysSetError("Invalid command list");

    NapysCommand cmd;
    cmd.type = NAPYS_COMMAND_TYPE_SET_COLOR;
    cmd.data = SDL_strdup(color_name);

    return NapysAddCommand(list, cmd);
}

bool NapysAddSetFontCommand(NapysCommandList *list, const char *color_name)
{
    if (!list || !color_name)
        return NapysSetError("Invalid command list");

    NapysCommand cmd;
    cmd.type = NAPYS_COMMAND_TYPE_SET_FONT;
    cmd.data = SDL_strdup(color_name);

    return NapysAddCommand(list, cmd);
}

bool NapysAddSetSizeCommand(NapysCommandList *list, const char *size_name)
{
    if (!list || !size_name)
        return NapysSetError("Invalid command list");

    NapysCommand cmd;
    cmd.type = NAPYS_COMMAND_TYPE_SET_SIZE;
    cmd.data = SDL_strdup(size_name);

    return NapysAddCommand(list, cmd);
}

bool NapysAddNewlineCommand(NapysCommandList *list)
{
    if (!list)
        return NapysSetError("Invalid command list");

    NapysCommand cmd;
    cmd.type = NAPYS_COMMAND_TYPE_NEWLINE;
    cmd.data = NULL;

    return NapysAddCommand(list, cmd);
}

bool NapysAddDrawImageCommand(NapysCommandList *list, const char *image_name)
{
    if (!list || !image_name)
        return NapysSetError("Invalid command list or image name");

    NapysCommand cmd;
    cmd.type = NAPYS_COMMAND_TYPE_DRAW_IMAGE;
    cmd.data = SDL_strdup(image_name);

    return NapysAddCommand(list, cmd);
}

static void NapysResetRendererTTF(NapysRendererTTF *rdr)
{
    rdr->current_color = (SDL_Color){255, 255, 255, 255};
    rdr->fragments_count = 0;
    rdr->current_font_size = 12;
    rdr->draw_x = 0;
    rdr->draw_y = 0;
    rdr->bounds = (SDL_Rect){0, 0, 0, 0};
    rdr->fragment_pointer = 0;
    rdr->current_font_cache = rdr->ctx->default_font_cache;
}

NapysRendererTTF *NapysCreateRendererTTF(NapysContext *ctx, SDL_Renderer *renderer)
{
    if (!ctx || !renderer)
    {
        NapysSetError("Invalid context or renderer");
        return NULL;
    }

    TTF_TextEngine *engine = TTF_CreateRendererTextEngine(renderer);
    if (!engine)
    {
        NapysSetError("Failed to create TTF text engine");
        return NULL;
    }

    NapysRendererTTF *nrttf = SDL_malloc(sizeof(NapysRendererTTF));

    if (!nrttf)
    {
        NapysSetError("Failed to allocate memory for NapysRendererTTF");
        return NULL;
    }

    nrttf->ctx = ctx;
    nrttf->engine = engine;
    nrttf->sdl_renderer = renderer;

    SDL_memset(nrttf->fragments, 0, sizeof(nrttf->fragments));

    NapysResetRendererTTF(nrttf);

    return nrttf;
}

void NapysDestroyRendererTTF(NapysRendererTTF *renderer)
{
    if (renderer)
    {
        SDL_free(renderer);
    }
}

static NapysFragmentTTF *NapysGetNextTextFragment(NapysRendererTTF *rdr)
{
    if (rdr->fragment_pointer >= NAPYS_TTF_RENDERER_MAX_TEXTS)
    {
        NapysSetError("Maximum number of text fragments reached");
        return NULL;
    }

    if (rdr->fragment_pointer < rdr->fragments_count)
    {
        return &rdr->fragments[rdr->fragment_pointer++];
    }

    TTF_Text *ttf_text = TTF_CreateText(rdr->engine, rdr->current_font, "", 0);
    if (!ttf_text)
    {
        NapysSetError("Failed to create TTF_Text");
        return NULL;
    }

    NapysFragmentTTF fragment = {0};
    fragment.text = ttf_text;
    fragment.img = NULL;
    fragment.x = rdr->draw_x;
    fragment.y = rdr->draw_y;

    rdr->fragments[rdr->fragments_count] = fragment;

    rdr->fragments_count++;
    rdr->fragment_pointer++;

    return &rdr->fragments[rdr->fragments_count - 1];
}

static NapysFragmentTTF *NapysGetNextImageFragment(NapysRendererTTF *rdr, SDL_Texture *img)
{
    if (rdr->fragment_pointer >= NAPYS_TTF_RENDERER_MAX_TEXTS)
    {
        NapysSetError("Maximum number of text fragments reached");
        return NULL;
    }

    if (rdr->fragment_pointer < rdr->fragments_count)
    {
        return &rdr->fragments[rdr->fragment_pointer++];
    }

    NapysFragmentTTF fragment = {0};
    fragment.text = NULL;
    fragment.img = img;
    fragment.x = rdr->draw_x;
    fragment.y = rdr->draw_y;

    rdr->fragments[rdr->fragments_count] = fragment;

    rdr->fragments_count++;
    rdr->fragment_pointer++;

    return &rdr->fragments[rdr->fragments_count - 1];
}

static void NapysUpdateBounds(NapysRendererTTF *rdr, int x, int y, int width, int height)
{
    if (x < rdr->bounds.x)
        rdr->bounds.x = x;
    if (y < rdr->bounds.y)
        rdr->bounds.y = y;

    const int end_x = x + width;
    const int end_y = y + height;

    if (end_x > rdr->bounds.x + rdr->bounds.w)
    {
        rdr->bounds.w += end_x - (rdr->bounds.x + rdr->bounds.w);
    }

    if (y + height > rdr->bounds.h + rdr->bounds.y)
    {
        rdr->bounds.h += end_y - (rdr->bounds.y + rdr->bounds.h);
    }
}

void NapysExecuteCommandList(NapysRendererTTF *rdr, NapysCommandList *list)
{
    if (!rdr || !list)
    {
        NapysSetError("Invalid renderer or command list");
        return;
    }

    NapysResetRendererTTF(rdr);

    NapysFragmentTTF *cur_fragment = NULL;

    rdr->current_font = NapysQueryFontCache(rdr->ctx->default_font_cache, rdr->current_font_size);

    for (int ci = 0; ci < list->cmd_count; ci++)
    {
        NapysCommand *cmd = &list->cmds[ci];

        bool advance_position = false;

        if (cmd->type == NAPYS_COMMAND_TYPE_DRAW_TEXT)
        {
            cur_fragment = NapysGetNextTextFragment(rdr);

            TTF_SetTextColor(cur_fragment->text, rdr->current_color.r, rdr->current_color.g, rdr->current_color.b, rdr->current_color.a);
            TTF_SetTextString(cur_fragment->text, cmd->data, SDL_strlen(cmd->data));
            advance_position = true;
        }
        else if (cmd->type == NAPYS_COMMAND_TYPE_SET_COLOR)
        {
            NapysRegistryEntry *entry = (NapysRegistryEntry *)NapysHashmapGetPointer(rdr->ctx->registry, cmd->data);

            if (entry && entry->type == NAPYS_REGISTRY_ENTRY_COLOR)
            {
                rdr->current_color = entry->color;
            }
        }
        else if (cmd->type == NAPYS_COMMAND_TYPE_SET_FONT)
        {
            NapysFontCache *font_cache = (NapysFontCache *)NapysHashmapGetPointer(rdr->ctx->fonts, cmd->data);

            if (font_cache && font_cache->base)
            {
                TTF_Font *new_font = NapysQueryFontCache(font_cache, rdr->current_font_size);

                if (new_font)
                {
                    rdr->current_font = new_font;
                    rdr->current_font_cache = font_cache;
                }
            }
        }
        else if (cmd->type == NAPYS_COMMAND_TYPE_SET_SIZE)
        {
            NapysRegistryEntry *entry = (NapysRegistryEntry *)NapysHashmapGetPointer(rdr->ctx->registry, cmd->data);
            if (entry && entry->type == NAPYS_REGISTRY_ENTRY_SIZE)
            {
                rdr->current_font_size = entry->ptsize;

                rdr->current_font = NapysQueryFontCache(rdr->current_font_cache, rdr->current_font_size);
            }
        }
        else if (cmd->type == NAPYS_COMMAND_TYPE_NEWLINE)
        {
            rdr->draw_x = 0;
            rdr->draw_y += TTF_GetFontHeight(rdr->current_font);
        }
        else if (cmd->type == NAPYS_COMMAND_TYPE_DRAW_IMAGE)
        {
            NapysRegistryEntry *entry = (NapysRegistryEntry *)NapysHashmapGetPointer(rdr->ctx->registry, cmd->data);

            if (entry && entry->type == NAPYS_REGISTRY_ENTRY_IMAGE)
            {
                SDL_Texture *img = (SDL_Texture *)entry->img;
                NapysFragmentTTF *img_fragment = NapysGetNextImageFragment(rdr, img);

                if (img_fragment)
                {
                    int line_height = TTF_GetFontHeight(rdr->current_font);
                    float img_width, img_height;
                    SDL_GetTextureSize(img, &img_width, &img_height);

                    img_fragment->x = rdr->draw_x;
                    img_fragment->y = rdr->draw_y + line_height / 2 - img_height / 2;

                    rdr->draw_x += img_width;

                    NapysUpdateBounds(rdr, img_fragment->x, img_fragment->y, img_width, img_height);
                }
            }
        }

        if (advance_position)
        {
            int text_width, text_height;

            TTF_GetTextSize(cur_fragment->text, &text_width, &text_height);

            NapysUpdateBounds(rdr, rdr->draw_x, rdr->draw_y, text_width, text_height);

            rdr->draw_x += text_width;
        }
    }
}

void NapysRenderTTF(NapysRendererTTF *renderer, float x, float y)
{
    if (!renderer || !renderer->engine)
    {
        NapysSetError("Invalid renderer or text engine");
        return;
    }

    for (int i = 0; i <= renderer->fragment_pointer; i++)
    {
        NapysFragmentTTF *fragment = &renderer->fragments[i];

        float draw_x = x + fragment->x;
        float draw_y = y + fragment->y;

        if (fragment)
        {
            if (fragment->text)
            {
                TTF_DrawRendererText(fragment->text, draw_x, draw_y);
            }

            if (fragment->img)
            {
                SDL_FRect img_rect = {draw_x, draw_y, 0, 0};
                SDL_GetTextureSize(fragment->img, &img_rect.w, &img_rect.h);
                SDL_RenderTexture(renderer->sdl_renderer, fragment->img, NULL, &img_rect);
            }
        }
    }
}

static void NapysParseRichTextTag(const char *tag, NapysCommandList *cmd_list)
{
    const char *delimeter = SDL_strstr(tag, ":");

    const char *cmd_name = SDL_strndup(tag, delimeter - tag);
    const char *cmd_value = delimeter != NULL ? SDL_strndup(delimeter + 1, SDL_strlen(tag) - (delimeter - tag) - 1) : NULL;

    NapysCommand cmd = {0};
    cmd.type = NAPYS_COMMAND_TYPE_NONE;

    if (SDL_strncmp(cmd_name, "color", 5) == 0 && cmd_value)
    {
        cmd.type = NAPYS_COMMAND_TYPE_SET_COLOR;
        cmd.data = SDL_strdup(cmd_value);
    }
    else if (SDL_strncmp(cmd_name, "font", 4) == 0 && cmd_value)
    {
        cmd.type = NAPYS_COMMAND_TYPE_SET_FONT;
        cmd.data = SDL_strdup(cmd_value);
    }
    else if (SDL_strncmp(cmd_name, "size", 4) == 0 && cmd_value)
    {
        cmd.type = NAPYS_COMMAND_TYPE_SET_SIZE;
        cmd.data = SDL_strdup(cmd_value);
    }
    else if (SDL_strncmp(cmd_name, "newline", 7) == 0)
    {
        cmd.type = NAPYS_COMMAND_TYPE_NEWLINE;
    }
    else if (SDL_strncmp(cmd_name, "image", 5) == 0 && cmd_value)
    {
        cmd.type = NAPYS_COMMAND_TYPE_DRAW_IMAGE;
        cmd.data = SDL_strdup(cmd_value);
    }

    SDL_free((void *)cmd_name);
    SDL_free((void *)cmd_value);

    if (cmd.type != NAPYS_COMMAND_TYPE_NONE)
    {
        NapysAddCommand(cmd_list, cmd);
    }
}

NapysCommandList *NapysParseRichText(const char *text, const NapysRichTextOptions *options)
{
    NapysCommandList *cmd_list = NapysCreateCommandList();

    const char *left_tag = options && options->left_tag ? options->left_tag : "{{";
    const char *right_tag = options && options->right_tag ? options->right_tag : "}}";

    const int len = SDL_strlen(text);

    const int lt_len = SDL_strlen(left_tag);
    const int rt_len = SDL_strlen(right_tag);

    const char *cursor = text;

    const char *last_text_start = text;

    const char *buffer = SDL_calloc(1, len + 1);

    while (cursor < text + len)
    {
        const char cc = *cursor;
        const int index = cursor - text;

        if (cursor[0] == left_tag[0] && index + lt_len <= len)
        {
            if (SDL_strncmp(cursor, left_tag, lt_len) == 0)
            {
                // Skip the left tag
                cursor += SDL_strlen(left_tag);

                // Try to find the ending right tag
                const char *end_tag = SDL_strnstr(cursor, right_tag, len - index - lt_len);

                if (end_tag)
                {
                    // If tag found, add the last text segment before the tag
                    if (last_text_start < cursor - lt_len)
                    {
                        const char *text_content = SDL_strndup(last_text_start, cursor - lt_len - last_text_start);
                        NapysAddDrawTextCommand(cmd_list, text_content);
                        SDL_free((void *)text_content); // Add draw text command makes a copy of the string
                    }

                    // Add the tag command
                    const char *tag_content = SDL_strndup(cursor, end_tag - cursor);
                    NapysParseRichTextTag(tag_content, cmd_list);
                    SDL_free((void *)tag_content);

                    // Skip the right tag
                    cursor = end_tag + rt_len;
                    last_text_start = cursor;
                }
                else
                {
                    NapysSetError("Unmatched left tag in rich text");
                    NapysDestroyCommandList(cmd_list);
                    SDL_free((void *)buffer);
                    return NULL;
                }
            }
        }
        else
        {
            cursor++;
        }
    }

    if (last_text_start < cursor)
    {
        // Add the last text segment after the last tag
        const char *text_content = SDL_strndup(last_text_start, cursor - last_text_start);
        NapysAddDrawTextCommand(cmd_list, text_content);
        SDL_free((void *)text_content); // Add draw text command makes a copy of the string
    }

    return cmd_list;
}

bool NapysGetRenderedTextBounds(NapysRendererTTF *renderer, SDL_Rect *output)
{
    if (!renderer)
    {
        return NapysSetError("Invalid renderer");
    }

    if (output)
    {
        *output = renderer->bounds;
    }

    return true;
}