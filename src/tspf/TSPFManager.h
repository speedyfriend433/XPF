#ifndef TSPF_MANAGER_H
#define TSPF_MANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <choma/MemoryStream.h>
#include <choma/MachO.h>

typedef struct TSPFPlugin {
    char *name;
    char *version;
    int (*initialize)(void);
    int (*patch)(MachO *macho);
    void (*cleanup)(void);
} TSPFPlugin;

typedef struct TSPFManager {
    TSPFPlugin **plugins;
    int pluginCount;
    MachO *targetApp;
} TSPFManager;

TSPFManager *tspf_manager_create(void);
int tspf_manager_register_plugin(TSPFManager *manager, TSPFPlugin *plugin);
int tspf_manager_load_target(TSPFManager *manager, const char *appPath);
int tspf_manager_apply_patches(TSPFManager *manager);
void tspf_manager_free(TSPFManager *manager);

#endif 