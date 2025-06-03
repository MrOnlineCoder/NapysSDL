#ifndef NAPYS_H
#define NAPYS_H

#include <SDL3/SDL.h>

#include <SDL3_ttf/SDL_ttf.h>

#define NAPYS_MAX_FONT_SIZE 256

typedef NapysHashmap NapysHashmap;

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
    NAPYS_REGISTRY_ENTRY_COLOR
} NapysRegistryEntryType;

typedef struct
{
    NapysRegistryEntryType type;

    char *str;
    SDL_Color color;
} NapysRegistryEntry;

typedef struct
{
    NapysHashmap *registry;
    NapysHashmap *fonts;
} NapysContext;

const char *NapysGetError();

NapysContext *NapysCreateContext();

bool NapysRegisterFont(NapysContext *ctx, TTF_Font *font, const char *name);
bool NapysRegisterString(NapysContext *ctx, const char *key, const char *value);
bool NapysRegisterColor(NapysContext *ctx, const char *key, SDL_Color color);

void NapysDestroyContext(NapysContext *ctx);

typedef enum
{
    NAPYS_COMMAND_TYPE_NONE,
    NAPYS_COMMAND_TYPE_DRAW_TEXT,
    NAPYS_COMMAND_TYPE_SET_COLOR,
    NAPYS_COMMAND_TYPE_DRAW_IMAGE,
    NAPYS_COMMAND_TYPE_SET_FONT
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

typedef struct
{
    NapysContext *ctx;
    TTF_TextEngine *engine;
} NapysRendererTTF;

NapysRendererTTF *NapysCreateRendererTTF(NapysContext *ctx, TTF_TextEngine *engine);
void NapysDestroyRendererTTF(NapysRendererTTF *renderer);

void NapysExecuteCommandList(NapysRendererTTF *renderer, NapysCommandList *list);

#endif