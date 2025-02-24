#include "../TSPFManager.h"
#include "../plugins/TSPFEntitlementPlugin.h"
#include "../plugins/TSPFExamplePlugin.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <path_to_app_binary>\n", argv[0]);
        return 1;
    }

    TSPFManager *manager = tspf_manager_create();
    if (!manager) {
        printf("Failed to create TSPF manager\n");
        return 1;
    }

    EntitlementPair entitlements[] = {
        {"com.apple.private.security.no-container", "true"},
        {"com.apple.security.exception.files.absolute-path.read-write", "*"},
        {"com.apple.private.security.container-required", "false"}
    };

    TSPFPlugin *entitlementPlugin = tspf_entitlement_plugin_create(
        entitlements, 
        sizeof(entitlements) / sizeof(EntitlementPair)
    );

    TSPFPlugin *examplePlugin = tspf_example_plugin_create();

    if (!entitlementPlugin || !examplePlugin ||
        tspf_manager_register_plugin(manager, entitlementPlugin) != 0 ||
        tspf_manager_register_plugin(manager, examplePlugin) != 0) {
        printf("Failed to register plugins\n");
        tspf_manager_free(manager);
        return 1;
    }

    if (tspf_manager_load_target(manager, argv[1]) != 0) {
        printf("Failed to load app\n");
        tspf_manager_free(manager);
        return 1;
    }

    if (tspf_manager_apply_patches(manager) != 0) {
        printf("Failed to apply patches\n");
        tspf_manager_free(manager);
        return 1;
    }

    printf("Successfully patched app!\n");
    tspf_manager_free(manager);
    return 0;
}