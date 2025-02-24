#include "../TSPFManager.h"
#include "../plugins/TSPFEntitlementPlugin.h"
#include "../plugins/TSPFBinaryPatchPlugin.h"
#include "../plugins/TSPFFunctionPatchPlugin.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <path_to_app>\n", argv[0]);
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

    // Example
    uint8_t returnTrue[] = {
        0x20, 0x00, 0x80, 0xD2,  // mov x0, #1
        0xC0, 0x03, 0x5F, 0xD6   // ret
    };

    FunctionPatch functionPatches[] = {
        {
            .symbolName = "isJailbroken",
            .patchBytes = returnTrue,
            .patchSize = sizeof(returnTrue),
            .useSymbol = true
        },
        {
            .address = 0x100008000,
            .patchBytes = returnTrue,
            .patchSize = sizeof(returnTrue),
            .useSymbol = false
        }
    };

    TSPFPlugin *entitlementPlugin = tspf_entitlement_plugin_create(
        entitlements, 
        sizeof(entitlements) / sizeof(EntitlementPair)
    );

    TSPFPlugin *functionPatchPlugin = tspf_function_patch_plugin_create(
        functionPatches,
        sizeof(functionPatches) / sizeof(FunctionPatch)
    );

    if (!entitlementPlugin || !functionPatchPlugin ||
        tspf_manager_register_plugin(manager, entitlementPlugin) != 0 ||
        tspf_manager_register_plugin(manager, functionPatchPlugin) != 0) {
        printf("Failed to register plugins\n");
        tspf_manager_free(manager);
        return 1;
    }

    if (tspf_manager_load_target(manager, argv[1]) != 0 ||
        tspf_manager_apply_patches(manager) != 0) {
        printf("Failed to apply patches\n");
        tspf_manager_free(manager);
        return 1;
    }

    printf("Successfully applied all patches!\n");

    tspf_manager_free(manager);
    return 0;
}