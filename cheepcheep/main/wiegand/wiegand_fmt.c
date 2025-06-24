#include "wiegand_fmt.h"

const wieg_fmt_t wieg_fmt_26bit = {
    .high_mask       = WIEG_26BIT_HIGH_MASK,
    .low_mask        = WIEG_26BIT_LOW_MASK,
    .fac_mask        = WIEG_26BIT_FAC_MASK,
    .uid_mask        = WIEG_26BIT_UID_MASK,
    .total_bits      = WIEG_26BIT_TOTAL_BITS,
    .fac_offset      = WIEG_26BIT_FAC_OFFSET,
    .uid_offset      = WIEG_26BIT_UID_OFFSET,
    .high_parity_idx = WIEG_26BIT_HIGH_PARITY_IDX,
    .low_parity_idx  = WIEG_26BIT_LOW_PARITY_IDX,
};

const wieg_fmt_t wieg_fmt_34bit = {
    .high_mask       = WIEG_34BIT_HIGH_MASK,
    .low_mask        = WIEG_34BIT_LOW_MASK,
    .fac_mask        = WIEG_34BIT_FAC_MASK,
    .uid_mask        = WIEG_34BIT_UID_MASK,
    .total_bits      = WIEG_34BIT_TOTAL_BITS,
    .fac_offset      = WIEG_34BIT_FAC_OFFSET,
    .uid_offset      = WIEG_34BIT_UID_OFFSET,
    .high_parity_idx = WIEG_34BIT_HIGH_PARITY_IDX,
    .low_parity_idx  = WIEG_34BIT_LOW_PARITY_IDX,
};