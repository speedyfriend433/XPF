#ifndef TSPF_ENTITLEMENT_PLUGIN_H
#define TSPF_ENTITLEMENT_PLUGIN_H

#include "../TSPFManager.h"

typedef struct {
    const char *name;
    const char *value;
} EntitlementPair;

TSPFPlugin *tspf_entitlement_plugin_create(EntitlementPair *entitlements, int count);

#endif