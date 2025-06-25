#ifndef WIEGANT_FMT_H_
#define WIEGAND_FMT_H_

#include <stdint.h>

#define WIEG_26BIT_TOTAL_BITS      26U
#define WIEG_26BIT_HIGH_MASK       0x01FFE000
#define WIEG_26BIT_LOW_MASK        0x00001FFE
#define WIEG_26BIT_FAC_MASK        0x01FE0000
#define WIEG_26BIT_FAC_OFFSET      17U
#define WIEG_26BIT_UID_MASK        0x0001FFFE
#define WIEG_26BIT_UID_OFFSET      1U
#define WIEG_26BIT_HIGH_PARITY_IDX 25U
#define WIEG_26BIT_LOW_PARITY_IDX  0U

#define WIEG_34BIT_TOTAL_BITS      34U
#define WIEG_34BIT_HIGH_MASK       0xFFFE0000 
#define WIEG_34BIT_LOW_MASK        0x0001FFFE
#define WIEG_34BIT_FAC_MASK        0xFFFE0000
#define WIEG_34BIT_FAC_OFFSET      17U
#define WIEG_34BIT_UID_MASK        0x0001FFFE
#define WIEG_34BIT_UID_OFFSET      1U
#define WIEG_34BIT_HIGH_PARITY_IDX 33U
#define WIEG_34BIT_LOW_PARITY_IDX  0U

#define WIEG_HIGH_PARITY_BIT(fmt, x)   ((x >> fmt->high_parity_idx) & 1)
#define WIEG_LOW_PARITY_BIT(fmt, x)    ((x >> fmt->low_parity_idx)  & 1)

typedef struct {
    uint32_t high_mask;
    uint32_t low_mask;
    uint32_t fac_mask;
    uint32_t uid_mask;
    int total_bits;
    int fac_offset;
    int uid_offset;
    int high_parity_idx;
    int low_parity_idx;
} wieg_fmt_t;

extern const wieg_fmt_t wieg_fmt_26bit;
extern const wieg_fmt_t wieg_fmt_34bit;

#endif /*WIEGAND_FMT_H_*/