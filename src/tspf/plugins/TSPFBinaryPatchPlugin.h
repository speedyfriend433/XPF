#ifndef TSPF_BINARY_PATCH_PLUGIN_H
#define TSPF_BINARY_PATCH_PLUGIN_H

#include "../TSPFManager.h"

typedef struct {
    uint64_t offset;
    uint8_t *original;
    uint8_t *patch;
    size_t size;
} BinaryPatch;

TSPFPlugin *tspf_binary_patch_plugin_create(BinaryPatch *patches, int count);

#endif