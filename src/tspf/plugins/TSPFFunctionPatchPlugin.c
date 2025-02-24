#include "TSPFFunctionPatchPlugin.h"
#include <string.h>

typedef struct {
    FunctionPatch *patches;
    int count;
} PluginContext;

static PluginContext *context = NULL;

static int find_symbol_address(MachO *macho, const char *symbolName, uint64_t *address) {
    __block bool found = false;
    
    macho_enumerate_symbols(macho, ^(const char *name, uint8_t type, uint64_t vmaddr, bool *stop) {
        if (strcmp(name, symbolName) == 0) {
            *address = vmaddr;
            found = true;
            *stop = true;
        }
    });
    
    return found ? 0 : -1;
}

static int function_patch_initialize(void) {
    printf("Initializing function patch plugin\n");
    return 0;
}

static int function_patch_apply(MachO *macho) {
    if (!context || !context->patches) return -1;

    for (int i = 0; i < context->count; i++) {
        FunctionPatch *patch = &context->patches[i];
        uint64_t targetAddr = patch->address;
        
        if (patch->useSymbol) {
            if (find_symbol_address(macho, patch->symbolName, &targetAddr) != 0) {
                printf("Failed to find symbol: %s\n", patch->symbolName);
                continue;
            }
        }
        
        uint64_t fileOffset;
        MachOSegment *segment;
        if (macho_translate_vmaddr_to_fileoff(macho, targetAddr, &fileOffset, &segment) != 0) {
            printf("Failed to translate address 0x%llx to file offset\n", targetAddr);
            continue;
        }
        
        if (macho_write_at_offset(macho, fileOffset, patch->patchSize, patch->patchBytes) != 0) {
            printf("Failed to write patch at offset 0x%llx\n", fileOffset);
            continue;
        }
        
        printf("Successfully patched function at address 0x%llx\n", targetAddr);
    }
    
    return 0;
}

static void function_patch_cleanup(void) {
    if (context) {
        free(context);
        context = NULL;
    }
}

TSPFPlugin *tspf_function_patch_plugin_create(FunctionPatch *patches, int count) {
    TSPFPlugin *plugin = (TSPFPlugin *)calloc(1, sizeof(TSPFPlugin));
    if (!plugin) return NULL;
    
    context = (PluginContext *)calloc(1, sizeof(PluginContext));
    if (!context) {
        free(plugin);
        return NULL;
    }
    
    context->patches = patches;
    context->count = count;
    
    plugin->name = "Function Patch Plugin";
    plugin->version = "1.0.0";
    plugin->initialize = function_patch_initialize;
    plugin->patch = function_patch_apply;
    plugin->cleanup = function_patch_cleanup;
    
    return plugin;
}