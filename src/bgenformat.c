#include <assert.h>
#include "bgenformat.h"
// bgenformat.c

void validate_bgen_header(BgenHeader* header) {
    assert(header->offset == 20); // No free data
    assert(header->lh == 20); // No free data
    assert(header->magic == 0x6E656762); // bgen
    assert(header->flags & 0x8); // layout v2
    assert((header->flags & 0x3 | 0x2) == 0x2); // zstd or uncomp
    assert(!(header->flags >> 31)); // no sample ids
}

int is_zstd(BgenHeader *header) {
    return (header->flags & 0x3) == 0x2;
}
