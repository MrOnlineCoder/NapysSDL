#ifndef NAPYS_INTERNAL_H
#define NAPYS_INTERNAL_H

#include <napys.h>

bool NapysSetError(const char *message);

typedef struct NapysHashmap
{
    SDL_PropertiesID data;
} NapysHashmap;

typedef void (*NapysHashmapCallback)(const char *key, void *value, void *userdata);

NapysHashmap *NapysCreateHashmap();
void NapysHashmapStorePointer(NapysHashmap *map, const char *key, void *value);
void *NapysHashmapGetPointer(NapysHashmap *map, const char *key);
void *NapysIterateHashmap(NapysHashmap *map, NapysHashmapCallback callback, void *userdata);
void NapysDestroyHashmap(NapysHashmap *map);

NapysFontCache *NapysCreateFontCache(TTF_Font *fnt);
TTF_Font *NapysQueryFontCache(NapysFontCache *cache, int ptsize);
void NapysDestroyFontCache(NapysFontCache *cache);

#endif