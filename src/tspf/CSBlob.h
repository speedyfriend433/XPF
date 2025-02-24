#ifndef CS_BLOB_H
#define CS_BLOB_H

#include <stdint.h>

#define CSMAGIC_EMBEDDED_SIGNATURE   0xFADE0CC0
#define CSMAGIC_EMBEDDED_ENTITLEMENTS 0xFADE7171

typedef struct {
    uint32_t magic;
    uint32_t length;
    uint8_t data[];
} CS_GenericBlob;

typedef struct {
    uint32_t type;
    uint32_t offset;
} CS_BlobIndex;

typedef struct {
    uint32_t magic;
    uint32_t length;
    uint32_t count;
    CS_BlobIndex index[];
} CS_SuperBlob;

#define CSSLOT_CODEDIRECTORY 0
#define CSSLOT_REQUIREMENTS  2
#define CSSLOT_ENTITLEMENTS 5

#endif