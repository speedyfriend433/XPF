#ifndef TSPF_PLUGIN_TEMPLATE_H
#define TSPF_PLUGIN_TEMPLATE_H

#include "../TSPFManager.h"

// example config, replace with your configuration
typedef struct {
    // target config
    char *targetSymbol;           // symbol/function to patch
    uint64_t targetAddress;      // direct memory address to patch
    bool useSymbolLookup;       // whether to use symbol or direct address
    
    // patch config
    uint8_t *patchBytes;         // bytes to write
    size_t patchSize;           // size of patch
    bool verifyOriginalBytes;  // whether to verify original bytes before patching
    uint8_t *originalBytes;   // original bytes to verify (optional)
    
    // plugin
    bool skipOnError;            // continue with other patches if this one fails
    char *logFile;              // optional log file path
    uint32_t flags;            // custom flags for plugin
    
    // custom parameters
    void *userData;             // user-defined data
    void (*callback)(void *);  // optional callback function
} TemplateConfig;

TSPFPlugin *tspf_template_plugin_create(TemplateConfig *config);

// example flags
#define TEMPLATE_FLAG_VERBOSE           (1 << 0)
#define TEMPLATE_FLAG_BACKUP           (1 << 1)
#define TEMPLATE_FLAG_FORCE_PATCH      (1 << 2)
#define TEMPLATE_FLAG_VERIFY_AFTER     (1 << 3)

#endif