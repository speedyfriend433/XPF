#include "TSPFManager.h"

TSPFManager *tspf_manager_create(void) {
    TSPFManager *manager = (TSPFManager *)calloc(1, sizeof(TSPFManager));
    if (!manager) return NULL;
    
    manager->plugins = NULL;
    manager->pluginCount = 0;
    manager->targetApp = NULL;
    
    return manager;
}

int tspf_manager_register_plugin(TSPFManager *manager, TSPFPlugin *plugin) {
    if (!manager || !plugin) return -1;
    
    TSPFPlugin **newPlugins = realloc(manager->plugins, 
                                     (manager->pluginCount + 1) * sizeof(TSPFPlugin *));
    if (!newPlugins) return -1;
    
    manager->plugins = newPlugins;
    manager->plugins[manager->pluginCount++] = plugin;
    
    return 0;
}

int tspf_manager_load_target(TSPFManager *manager, const char *appPath) {
    if (!manager || !appPath) return -1;
    
    if (manager->targetApp) {
        macho_free(manager->targetApp);
    }
    
    manager->targetApp = macho_init_for_writing(appPath);
    if (!manager->targetApp) return -1;
    
    return 0;
}

int tspf_manager_apply_patches(TSPFManager *manager) {
    if (!manager || !manager->targetApp) return -1;
    
    for (int i = 0; i < manager->pluginCount; i++) {
        TSPFPlugin *plugin = manager->plugins[i];
        
        if (plugin->initialize) {
            int result = plugin->initialize();
            if (result != 0) continue;
        }
        
        if (plugin->patch) {
            plugin->patch(manager->targetApp);
        }
        
        if (plugin->cleanup) {
            plugin->cleanup();
        }
    }
    
    return 0;
}

void tspf_manager_free(TSPFManager *manager) {
    if (!manager) return;
    
    if (manager->targetApp) {
        macho_free(manager->targetApp);
    }
    
    if (manager->plugins) {
        free(manager->plugins);
    }
    
    free(manager);
}