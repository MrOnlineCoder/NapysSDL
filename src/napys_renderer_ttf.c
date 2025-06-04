#include <napys.h>
#include "napys_internal.h"

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

static NapysFragmentTTF *NapysGetNextTextFragment(NapysRendererTTF *rdr, const char *contents)
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

    TTF_Text *ttf_text = TTF_CreateText(rdr->engine, rdr->current_font, contents, 0);
    if (!ttf_text)
    {
        NapysSetError("Failed to create TTF_Text");
        return NULL;
    }

    TTF_SetTextColor(ttf_text, rdr->current_color.r, rdr->current_color.g, rdr->current_color.b, rdr->current_color.a);

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
            cur_fragment = NapysGetNextTextFragment(rdr, cmd->data);

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
        else if (cmd->type == NAPYS_COMMAND_TYPE_USE_STRING)
        {
            NapysRegistryEntry *entry = (NapysRegistryEntry *)NapysHashmapGetPointer(rdr->ctx->registry, cmd->data);

            if (entry && entry->type == NAPYS_REGISTRY_ENTRY_STRING)
            {
                cur_fragment = NapysGetNextTextFragment(rdr, entry->str);

                advance_position = true;
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