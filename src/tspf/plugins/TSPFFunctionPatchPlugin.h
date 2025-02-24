#ifndef TSPF_FUNCTION_PATCH_PLUGIN_H
#define TSPF_FUNCTION_PATCH_PLUGIN_H

#include "../TSPFManager.h"

typedef struct {
    const char *symbolName;
    uint64_t address;
    uint8_t *patchBytes;
    size_t patchSize;
    bool useSymbol;
} FunctionPatch;

TSPFPlugin *tspf_function_patch_plugin_create(FunctionPatch *patches, int count);

#endif 