#include "TSPFBinaryPatchPlugin.h"
#include <string.h>

typedef struct {
    BinaryPatch *patches;
    int count;
} PluginContext;

static PluginContext *context = NULL;

static int binary_patch_initialize(void) {
    printf("Initializing binary patch plugin\n");
    return 0;
}

static int binary_patch_apply(MachO *macho) {
    if (!context || !context->patches) return -1;

    for (int i = 0; i < context->count; i++) {
        BinaryPatch *patch = &context->patches[i];
        uint8_t current[patch->size];
        
        if (macho_read_at_offset(macho, patch->offset, patch->size, current) != 0) {
            printf("Failed to read at offset 0x%llx\n", patch->offset);
            continue;
        }
        
        if (patch->original && memcmp(current, patch->original, patch->size) != 0) {
            printf("Original bytes don't match at offset 0x%llx\n", patch->offset);
            continue;
        }
        
        if (macho_write_at_offset(macho, patch->offset, patch->size, patch->patch) != 0) {
            printf("Failed to write patch at offset 0x%llx\n", patch->offset);
            continue;
        }
        
        printf("Successfully applied patch at offset 0x%llx\n", patch->offset);
    }
    
    return 0;
}

static void binary_patch_cleanup(void) {
    if (context) {
        free(context);
        context = NULL;
    }
}

TSPFPlugin *tspf_binary_patch_plugin_create(BinaryPatch *patches, int count) {
    TSPFPlugin *plugin = (TSPFPlugin *)calloc(1, sizeof(TSPFPlugin));
    if (!plugin) return NULL;
    
    context = (PluginContext *)calloc(1, sizeof(PluginContext));
    if (!context) {
        free(plugin);
        return NULL;
    }
    
    context->patches = patches;
    context->count = count;
    
    plugin->name = "Binary Patch Plugin";
    plugin->version = "1.0.0";
    plugin->initialize = binary_patch_initialize;
    plugin->patch = binary_patch_apply;
    plugin->cleanup = binary_patch_cleanup;
    
    return plugin;
}