#ifndef NAPYS_INTERNAL_H
#define NAPYS_INTERNAL_H

#include <napys.h>

NapysHashmap *NapysCreateHashmap();

void NapysDestroyHashmap(NapysHashmap *map);

void NapysHashmapStorePointer(NapysHashmap *map, const char *key, void *value);

void *NapysHashmapGetPointer(NapysHashmap *map, const char *key);

#endif