#include "TSPFEntitlementPlugin.h"
#include <string.h>
#include <stdio.h>
#include "CSBlob.h"

typedef struct {
    EntitlementPair *entitlements;
    int count;
} PluginContext;

static PluginContext *context = NULL;

static int entitlement_initialize(void) {
    printf("Initializing entitlement plugin\n");
    return 0;
}

static int entitlement_patch(MachO *macho) {
    if (!context || !context->entitlements) return -1;

    printf("Applying entitlement patches...\n");
    
    __block uint64_t csOffset = 0;
    __block uint32_t csSize = 0;
    
    macho_enumerate_load_commands(macho, ^(struct load_command loadCommand, uint64_t offset, void *cmd, bool *stop) {
        if (loadCommand.cmd == LC_CODE_SIGNATURE) {
            struct linkedit_data_command *csCmd = (struct linkedit_data_command *)cmd;
            csOffset = csCmd->dataoff;
            csSize = csCmd->datasize;
            *stop = true;
        }
    });
    
    if (csOffset == 0 || csSize == 0) {
        printf("No code signature found\n");
        return -1;
    }
    
    CS_SuperBlob superBlob;
    if (macho_read_at_offset(macho, csOffset, sizeof(CS_SuperBlob), &superBlob) != 0) {
        printf("Failed to read SuperBlob\n");
        return -1;
    }
    
    __block uint32_t entOffset = 0;
    for (uint32_t i = 0; i < superBlob.count; i++) {
        CS_BlobIndex index;
        uint64_t indexOffset = csOffset + sizeof(CS_SuperBlob) + (i * sizeof(CS_BlobIndex));
        
        if (macho_read_at_offset(macho, indexOffset, sizeof(CS_BlobIndex), &index) != 0) {
            continue;
        }
        
        if (index.type == CSSLOT_ENTITLEMENTS) {
            entOffset = index.offset;
            break;
        }
    }
    
    char *entPlist = NULL;
    size_t entPlistSize = 0;
    FILE *tmp = tmpfile();
    if (!tmp) {
        printf("Failed to create temporary file\n");
        return -1;
    }
    
    fprintf(tmp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(tmp, "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n");
    fprintf(tmp, "<plist version=\"1.0\">\n");
    fprintf(tmp, "<dict>\n");
    
    for (int i = 0; i < context->count; i++) {
        fprintf(tmp, "\t<key>%s</key>\n", context->entitlements[i].name);
        fprintf(tmp, "\t<string>%s</string>\n", context->entitlements[i].value);
    }
    
    fprintf(tmp, "</dict>\n");
    fprintf(tmp, "</plist>\n");
    
    fseek(tmp, 0, SEEK_END);
    entPlistSize = ftell(tmp);
    fseek(tmp, 0, SEEK_SET);
    
    entPlist = (char *)malloc(entPlistSize);
    if (!entPlist) {
        fclose(tmp);
        return -1;
    }
    
    fread(entPlist, 1, entPlistSize, tmp);
    fclose(tmp);
    
    size_t blobSize = sizeof(CS_GenericBlob) + entPlistSize;
    CS_GenericBlob *newBlob = (CS_GenericBlob *)calloc(1, blobSize);
    if (!newBlob) {
        free(entPlist);
        return -1;
    }
    
    newBlob->magic = CSMAGIC_EMBEDDED_ENTITLEMENTS;
    newBlob->length = (uint32_t)blobSize;
    memcpy(newBlob->data, entPlist, entPlistSize);
    
    if (macho_write_at_offset(macho, csOffset + entOffset, blobSize, newBlob) != 0) {
        printf("Failed to write new entitlements blob\n");
        free(entPlist);
        free(newBlob);
        return -1;
    }
    
    printf("Successfully updated entitlements\n");
    
    free(entPlist);
    free(newBlob);
    return 0;
}

static void entitlement_cleanup(void) {
    if (context) {
        free(context);
        context = NULL;
    }
}

TSPFPlugin *tspf_entitlement_plugin_create(EntitlementPair *entitlements, int count) {
    TSPFPlugin *plugin = (TSPFPlugin *)calloc(1, sizeof(TSPFPlugin));
    if (!plugin) return NULL;
    
    context = (PluginContext *)calloc(1, sizeof(PluginContext));
    if (!context) {
        free(plugin);
        return NULL;
    }
    
    context->entitlements = entitlements;
    context->count = count;
    
    plugin->name = "Entitlement Plugin";
    plugin->version = "1.0.0";
    plugin->initialize = entitlement_initialize;
    plugin->patch = entitlement_patch;
    plugin->cleanup = entitlement_cleanup;
    
    return plugin;
}