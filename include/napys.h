#ifndef NAPYS_H
#define NAPYS_H

#include <SDL3/SDL.h>

#include <SDL3_ttf/SDL_ttf.h>

#define NAPYS_MAX_FONT_SIZE 256
#define NAPYS_TTF_RENDERER_MAX_TEXTS 128

typedef struct NapysHashmap NapysHashmap;

typedef struct
{
    TTF_Font *sizes[NAPYS_MAX_FONT_SIZE];
    TTF_Font *base;
} NapysFontCache;

typedef enum
{
    NAPYS_REGISTRY_ENTRY_NONE,
    NAPYS_REGISTRY_ENTRY_STRING,
    NAPYS_REGISTRY_ENTRY_IMAGE,
    NAPYS_REGISTRY_ENTRY_COLOR,
    NAPYS_REGISTRY_ENTRY_SIZE
} NapysRegistryEntryType;

typedef struct
{
    NapysRegistryEntryType type;

    char *str;
    SDL_Color color;
    int ptsize;
    void *img;
} NapysRegistryEntry;

typedef struct
{
    NapysHashmap *registry;
    NapysHashmap *fonts;

    NapysFontCache *default_font_cache;
} NapysContext;

const char *NapysGetError();

NapysContext *NapysCreateContext();

bool NapysRegisterFont(NapysContext *ctx, TTF_Font *font, const char *name);
bool NapysRegisterString(NapysContext *ctx, const char *key, const char *value);
bool NapysRegisterColor(NapysContext *ctx, const char *key, SDL_Color color);
bool NapysRegisterSize(NapysContext *ctx, const char *key, int pt);
bool NapysRegisterImage(NapysContext *ctx, const char *key, void *img);

void NapysRegisterCSSColors(NapysContext *ctx);

void NapysDestroyContext(NapysContext *ctx);

typedef enum
{
    NAPYS_COMMAND_TYPE_NONE,
    NAPYS_COMMAND_TYPE_DRAW_TEXT,
    NAPYS_COMMAND_TYPE_SET_COLOR,
    NAPYS_COMMAND_TYPE_DRAW_IMAGE,
    NAPYS_COMMAND_TYPE_SET_FONT,
    NAPYS_COMMAND_TYPE_SET_SIZE,
    NAPYS_COMMAND_TYPE_NEWLINE,
} NapysCommandType;

typedef struct
{
    NapysCommandType type;
    char *data;
} NapysCommand;

typedef struct
{
    NapysCommand *cmds;
    int cmd_count;
    int cmd_capacity;
} NapysCommandList;

NapysCommandList *NapysCreateCommandList();
void NapysClearCommandList(NapysCommandList *list);
void NapysDestroyCommandList(NapysCommandList *list);

bool NapysAddCommand(NapysCommandList *list, NapysCommand cmd);

bool NapysAddDrawTextCommand(NapysCommandList *list, const char *text);
bool NapysAddSetColorCommand(NapysCommandList *list, const char *color_name);
bool NapysAddSetFontCommand(NapysCommandList *list, const char *color_name);
bool NapysAddSetSizeCommand(NapysCommandList *list, const char *size_name);
bool NapysAddNewlineCommand(NapysCommandList *list);
bool NapysAddDrawImageCommand(NapysCommandList *list, const char *image_name);

typedef struct
{
    TTF_Text *text;
    SDL_Texture *img;
    int x;
    int y;
} NapysFragmentTTF;

typedef struct
{
    NapysContext *ctx;
    TTF_TextEngine *engine;
    SDL_Renderer *sdl_renderer;

    NapysFragmentTTF fragments[NAPYS_TTF_RENDERER_MAX_TEXTS];
    int fragments_count;
    int fragment_pointer;

    SDL_Color current_color;
    TTF_Font *current_font;
    NapysFontCache *current_font_cache;
    int current_font_size;

    int draw_x;
    int draw_y;

    SDL_Rect bounds;
} NapysRendererTTF;

NapysRendererTTF *NapysCreateRendererTTF(NapysContext *ctx, SDL_Renderer *renderer);
void NapysDestroyRendererTTF(NapysRendererTTF *renderer);

void NapysExecuteCommandList(NapysRendererTTF *renderer, NapysCommandList *list);

void NapysRenderTTF(NapysRendererTTF *renderer, float x, float y);

bool NapysGetRenderedTextBounds(NapysRendererTTF *renderer, SDL_Rect *output);

typedef struct
{
    const char *left_tag;
    const char *right_tag;
} NapysRichTextOptions;

NapysCommandList *NapysParseRichText(const char *text, const NapysRichTextOptions *options);

#endif