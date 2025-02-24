#include "TSPFPluginTemplate.h"

typedef struct {
    TemplateConfig *config;
} PluginContext;

static PluginContext *context = NULL;

static int template_initialize(void) {
    printf("Initializing template plugin\n");
    
    if (!context || !context->config) return -1;
    
    if (context->config->useSymbolLookup && !context->config->targetSymbol) {
        printf("Symbol lookup enabled but no target symbol provided\n");
        return -1;
    }
    
    if (!context->config->useSymbolLookup && !context->config->targetAddress) {
        printf("Direct addressing enabled but no target address provided\n");
        return -1;
    }
    
    if (!context->config->patchBytes || !context->config->patchSize) {
        printf("No patch data provided\n");
        return -1;
    }
    
    if (context->config->logFile) {
        printf("Logging enabled to: %s\n", context->config->logFile);
    }
    
    if (context->config->userData && context->config->callback) {
        context->config->callback(context->config->userData);
    }
    
    return 0;
}

static int template_patch(MachO *macho) {
    if (!context || !context->config) return -1;
    
    printf("Applying template patch\n");
    
    uint64_t targetAddr = context->config->targetAddress;
    
    if (context->config->useSymbolLookup) {
        __block bool found = false;
        __block uint64_t symbolAddr = 0; 
        
        macho_enumerate_symbols(macho, ^(const char *name, uint8_t type, uint64_t vmaddr, bool *stop) {
            if (strcmp(name, context->config->targetSymbol) == 0) {
                symbolAddr = vmaddr;  
                found = true;
                *stop = true;
            }
        });
        
        if (!found) {
            printf("Symbol not found: %s\n", context->config->targetSymbol);
            return context->config->skipOnError ? 0 : -1;
        }
        
        targetAddr = symbolAddr;  
    }
    
    uint64_t fileOffset;
    MachOSegment *segment;
    if (macho_translate_vmaddr_to_fileoff(macho, targetAddr, &fileOffset, &segment) != 0) {
        printf("Failed to translate address 0x%llx\n", targetAddr);
        return context->config->skipOnError ? 0 : -1;
    }
    
    if (context->config->verifyOriginalBytes && context->config->originalBytes) {
        uint8_t currentBytes[context->config->patchSize];
        if (macho_read_at_offset(macho, fileOffset, context->config->patchSize, currentBytes) != 0) {
            printf("Failed to read current bytes\n");
            return context->config->skipOnError ? 0 : -1;
        }
        
        if (memcmp(currentBytes, context->config->originalBytes, context->config->patchSize) != 0) {
            printf("Original bytes verification failed\n");
            return context->config->skipOnError ? 0 : -1;
        }
    }
    
    if (context->config->flags & TEMPLATE_FLAG_BACKUP) {
        char backupPath[1024];
        snprintf(backupPath, sizeof(backupPath), "%s.backup.%llx", 
                 context->config->logFile ? context->config->logFile : "patch",
                 targetAddr);
    
        uint8_t *backupData = (uint8_t *)malloc(context->config->patchSize);
        if (!backupData) {
            printf("Failed to allocate memory for backup\n");
            return context->config->skipOnError ? 0 : -1;
        }
        
        if (macho_read_at_offset(macho, fileOffset, context->config->patchSize, backupData) != 0) {
            printf("Failed to read data for backup\n");
            free(backupData);
            return context->config->skipOnError ? 0 : -1;
        }
        
        FILE *backupFile = fopen(backupPath, "wb");
        if (!backupFile) {
            printf("Failed to create backup file: %s\n", backupPath);
            free(backupData);
            return context->config->skipOnError ? 0 : -1;
        }
        
        fprintf(backupFile, "TSPF_BACKUP\n");
        fprintf(backupFile, "Address: 0x%llx\n", targetAddr);
        fprintf(backupFile, "Offset: 0x%llx\n", fileOffset);
        fprintf(backupFile, "Size: %zu\n", context->config->patchSize);
        fprintf(backupFile, "DATA:\n");
        // actual backup data
        fwrite(backupData, 1, context->config->patchSize, backupFile);
        fclose(backupFile);
        free(backupData);
        
        printf("Created backup at: %s\n", backupPath);
    }
    
    if (macho_write_at_offset(macho, fileOffset, context->config->patchSize, context->config->patchBytes) != 0) {
        printf("Failed to write patch\n");
        return context->config->skipOnError ? 0 : -1;
    }
    
    if (context->config->flags & TEMPLATE_FLAG_VERIFY_AFTER) {
        uint8_t verifyBytes[context->config->patchSize];
        if (macho_read_at_offset(macho, fileOffset, context->config->patchSize, verifyBytes) != 0 ||
            memcmp(verifyBytes, context->config->patchBytes, context->config->patchSize) != 0) {
            printf("Patch verification failed\n");
            return context->config->skipOnError ? 0 : -1;
        }
    }
    
    printf("Successfully applied patch at address 0x%llx\n", targetAddr);
    return 0;
}

static void template_cleanup(void) {
    if (context) {
        free(context);
        context = NULL;
    }
}

TSPFPlugin *tspf_template_plugin_create(TemplateConfig *config) {
    TSPFPlugin *plugin = (TSPFPlugin *)calloc(1, sizeof(TSPFPlugin));
    if (!plugin) return NULL;
    
    context = (PluginContext *)calloc(1, sizeof(PluginContext));
    if (!context) {
        free(plugin);
        return NULL;
    }
    
    context->config = config;
    
    plugin->name = "Template Plugin";
    plugin->version = "1.0.0";
    plugin->initialize = template_initialize;
    plugin->patch = template_patch;
    plugin->cleanup = template_cleanup;
    
    return plugin;
}