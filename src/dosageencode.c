#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>
#include "bgenformat.h"
#include "agentformat.h"
#include "memman.h"
// dosageencode.c

void dosage_encode(
        AgentFseHeader* agentFseHeader,
        AgentProbabilityData* agentProbDataBlock,
        UncompressedProbabilityData *probDataBlock) {
    // TODO: Add asserts here and/or elsewhere for diploid and v1.2
    assert(!probDataBlock->phased);
    assert(probDataBlock->b == 16);
    uint8_t* data = probDataBlock->data;
    uint32_t n = probDataBlock->n;
    uint32_t i = 0;
    // Encode 16 samples at a time
    // E(m) = 1 - p(MM) - 0.5p(Mm) = (2^b-1) - M[0...b-1] - 0.5M[b...2b-1]
    if (n >= 16) { // Fix: Unsigned integer underflow: e.g. n = 6 => n-16 = 4294967286
        for (; i <= n - 16; i += 16) {
            // Load registers
            short *p = (short *) &data[i * 2 * sizeof(short)];
            // x: the 16 (2^b-1)s == all ones == -1 in two's complement, 256-bit register
            __m256i x = _mm256_set1_epi16(-1);
            // 16 samples = 16 p(MM) + 16 p(Mm) = 32 + 32 bytes = two 256-bit registers
            // y: the 16 p(MM)s
            __m256i y = _mm256_set_epi16(
                    p[30], p[28], p[26], p[24],
                    p[22], p[20], p[18], p[16],
                    p[14], p[12], p[10], p[8],
                    p[6], p[4], p[2], p[0]);
            // z: the 16 p(Mm)s
            __m256i z = _mm256_set_epi16(
                    p[31], p[29], p[27], p[25],
                    p[23], p[21], p[19], p[17],
                    p[15], p[13], p[11], p[9],
                    p[7], p[5], p[3], p[1]);
            // Divide heterozygous by 2
            z = _mm256_srli_epi16(z, 1);
            // Do subtractions
            x = _mm256_sub_epi16(x, y);
            x = _mm256_sub_epi16(x, z);
            // Store register
            __m256i *q = (__m256i *) &data[i * sizeof(short)];
            _mm256_storeu_si256(q, x);
        }
    }
    // Encode the rest
    for (; i < n; i++) {
        uint16_t * p = (uint16_t *) &data[i * 2 * sizeof(short)];
        uint16_t * q = (uint16_t *) &data[i * sizeof(short)];
        *q = UINT16_MAX - p[0] - (p[1] >> 1);
    }
    // Shrink the data buffer
    // n samples = n E(m)s = 2n bytes (each E(m) is b == 16 bits)
    probDataBlock->data = resize_buf(data, sizeof(short), n);
    // Original size (4n bytes) -> halved to new size (2n bytes)
    probDataBlock->c -= 2 * n;
}
