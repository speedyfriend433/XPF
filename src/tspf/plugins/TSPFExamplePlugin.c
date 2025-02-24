#include "TSPFExamplePlugin.h"

static int example_initialize(void) {
    printf("Example plugin initialized\n");
    return 0;
}

static int example_patch(MachO *macho) {
    printf("Applying example patches to application\n");
    
    // example 1: JB detection patch
    const char *jbTargetSymbol = "_isJailbroken";
    uint8_t jbPatchBytes[] = {
        0x00, 0x00, 0x80, 0xD2, // mov x0, #0 - return false
        0xC0, 0x03, 0x5F, 0xD6  // ret
    };
    
    // example 2: Geometry Dash noclip patch
    const char *noclipTargetSymbol = "_PlayLayer_destroyPlayer";
    uint8_t noclipPatchBytes[] = {
        0x08, 0x00, 0x80, 0xD2, // mov x8, #0x300000000
        0x08, 0x00, 0x00, 0x91, // add x8, x8, #0x0
        0x08, 0x01, 0x40, 0xF9, // ldr x8, [x8]
        0x00, 0x01, 0x1F, 0xD6  // br x8
    };
    
    // Apply jb detection
    __block bool found = false;
    __block uint64_t targetAddr = 0;
    
    macho_enumerate_symbols(macho, ^(const char *name, uint8_t type, uint64_t vmaddr, bool *stop) {
        if (strcmp(name, jbTargetSymbol) == 0) {
            targetAddr = vmaddr;
            found = true;
            *stop = true;
        }
    });
    
    if (found) {
        uint64_t fileOffset;
        MachOSegment *segment;
        if (macho_translate_vmaddr_to_fileoff(macho, targetAddr, &fileOffset, &segment) == 0) {
            uint8_t originalBytes[sizeof(jbPatchBytes)];
            if (macho_read_at_offset(macho, fileOffset, sizeof(jbPatchBytes), originalBytes) == 0 &&
                macho_write_at_offset(macho, fileOffset, sizeof(jbPatchBytes), jbPatchBytes) == 0) {
                printf("Successfully patched '%s' at address 0x%llx\n", jbTargetSymbol, targetAddr);
            }
        }
    }
    
    // apply noclip patch
    found = false;
    targetAddr = 0;
    
    macho_enumerate_symbols(macho, ^(const char *name, uint8_t type, uint64_t vmaddr, bool *stop) {
        if (strcmp(name, noclipTargetSymbol) == 0) {
            targetAddr = vmaddr;
            found = true;
            *stop = true;
        }
    });
    
    if (found) {
        uint64_t fileOffset;
        MachOSegment *segment;
        if (macho_translate_vmaddr_to_fileoff(macho, targetAddr, &fileOffset, &segment) == 0) {
            uint8_t originalBytes[sizeof(noclipPatchBytes)];
            if (macho_read_at_offset(macho, fileOffset, sizeof(noclipPatchBytes), originalBytes) == 0 &&
                macho_write_at_offset(macho, fileOffset, sizeof(noclipPatchBytes), noclipPatchBytes) == 0) {
                printf("Successfully patched '%s' at address 0x%llx\n", noclipTargetSymbol, targetAddr);
            }
        }
    }
    
    return 0;
}

static void example_cleanup(void) {
    printf("Example plugin cleanup\n");
}

TSPFPlugin *tspf_example_plugin_create(void) {
    TSPFPlugin *plugin = (TSPFPlugin *)calloc(1, sizeof(TSPFPlugin));
    if (!plugin) return NULL;
    
    plugin->name = "Example Plugin";
    plugin->version = "1.0.0";
    plugin->initialize = example_initialize;
    plugin->patch = example_patch;
    plugin->cleanup = example_cleanup;
    
    return plugin;
}