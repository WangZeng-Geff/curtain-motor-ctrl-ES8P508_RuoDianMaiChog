#include <utils.h>

/*
 * lib/bitmap.c
 * Helper functions for bitmap.h.
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2.  See the file COPYING for more details.
 */
#include "bitmap.h"

/*
 * bitmaps provide an array of bits, implemented using an an
 * array of unsigned longs.  The number of valid bits in a
 * given bitmap does _not_ need to be an exact multiple of
 * BITS_PER_LONG.
 *
 * The possible unused bits in the last, partially used word
 * of a bitmap are 'don't care'.  The implementation makes
 * no particular effort to keep them zero.  It ensures that
 * their value will not affect the results of any operation.
 * The bitmap operations that return Boolean (bitmap_empty,
 * for example) or scalar (bitmap_weight, for example) results
 * carefully filter out these unused bits from impacting their
 * results.
 *
 * These operations actually hold to a slightly stronger rule:
 * if you don't input any bitmaps to these ops that have some
 * unused bits set, then they won't output any set unused bits
 * in output bitmaps.
 *
 * The byte ordering of bitmaps is more natural on little
 * endian architectures.  See the big-endian headers
 * include/asm-ppc64/bitops.h and include/asm-s390/bitops.h
 * for the best explanations of this ordering.
 */

/**
 * __bitmap_shift_right - logical right shift of the bits in a
 * bitmap
 *   @dst : destination bitmap
 *   @src : source bitmap
 *   @shift : shift by this many bits
 *   @nbits : bitmap size, in bits
 *
 * Shifting right (dividing) means moving bits in the MS -> LS bit
 * direction.  Zeros are fed into the vacated MS positions and the
 * LS bits shifted off the bottom are lost.
 */
void __bitmap_shift_right(unsigned long *dst, const unsigned long *src,
                          unsigned shift, unsigned nbits)
{
    unsigned k, lim = BITS_TO_LONGS(nbits);
    unsigned off = shift / BITS_PER_LONG, rem = shift % BITS_PER_LONG;
    unsigned long mask = BITMAP_LAST_WORD_MASK(nbits);
    for (k = 0; off + k < lim; ++k)
    {
        unsigned long upper, lower;

        /*
         * If shift is not word aligned, take lower rem bits of
         * word above and make them the top rem bits of result.
         */
        if (!rem || off + k + 1 >= lim)
            upper = 0;
        else
        {
            upper = src[off + k + 1];
            if (off + k + 1 == lim - 1)
                upper &= mask;
            upper <<= (BITS_PER_LONG - rem);
        }
        lower = src[off + k];
        if (off + k == lim - 1)
            lower &= mask;
        lower >>= rem;
        dst[k] = lower | upper;
    }
    if (off)
        memset(&dst[lim - off], 0, off * sizeof(unsigned long));
}


/**
 * __bitmap_shift_left - logical left shift of the bits in a
 * bitmap
 *   @dst : destination bitmap
 *   @src : source bitmap
 *   @shift : shift by this many bits
 *   @nbits : bitmap size, in bits
 *
 * Shifting left (multiplying) means moving bits in the LS -> MS
 * direction.  Zeros are fed into the vacated LS bit positions
 * and those MS bits shifted off the top are lost.
 */

void __bitmap_shift_left(unsigned long *dst, const unsigned long *src,
                         unsigned int shift, unsigned int nbits)
{
    int k;
    unsigned int lim = BITS_TO_LONGS(nbits);
    unsigned int off = shift / BITS_PER_LONG, rem = shift % BITS_PER_LONG;
    for (k = lim - off - 1; k >= 0; --k)
    {
        unsigned long upper, lower;

        /*
         * If shift is not word aligned, take upper rem bits of
         * word below and make them the bottom rem bits of result.
         */
        if (rem && k > 0)
            lower = src[k - 1] >> (BITS_PER_LONG - rem);
        else
            lower = 0;
        upper = src[k] << rem;
        dst[k + off] = lower | upper;
    }
    if (off)
        memset(dst, 0, off * sizeof(unsigned long));
}

int __bitmap_and(unsigned long *dst, const unsigned long *bitmap1,
                 const unsigned long *bitmap2, unsigned int bits)
{
    unsigned int k;
    unsigned int lim = bits / BITS_PER_LONG;
    unsigned long result = 0;

    for (k = 0; k < lim; k++)
        result |= (dst[k] = bitmap1[k] & bitmap2[k]);
    if (bits % BITS_PER_LONG)
        result |= (dst[k] = bitmap1[k] & bitmap2[k] &
                            BITMAP_LAST_WORD_MASK(bits));
    return result != 0;
}

void __bitmap_or(unsigned long *dst, const unsigned long *bitmap1,
                 const unsigned long *bitmap2, unsigned int bits)
{
    unsigned int k;
    unsigned int nr = BITS_TO_LONGS(bits);

    for (k = 0; k < nr; k++)
        dst[k] = bitmap1[k] | bitmap2[k];
}

void __bitmap_xor(unsigned long *dst, const unsigned long *bitmap1,
                  const unsigned long *bitmap2, unsigned int bits)
{
    unsigned int k;
    unsigned int nr = BITS_TO_LONGS(bits);

    for (k = 0; k < nr; k++)
        dst[k] = bitmap1[k] ^ bitmap2[k];
}

int __bitmap_andnot(unsigned long *dst, const unsigned long *bitmap1,
                    const unsigned long *bitmap2, unsigned int bits)
{
    unsigned int k;
    unsigned int lim = bits / BITS_PER_LONG;
    unsigned long result = 0;

    for (k = 0; k < lim; k++)
        result |= (dst[k] = bitmap1[k] & ~bitmap2[k]);
    if (bits % BITS_PER_LONG)
        result |= (dst[k] = bitmap1[k] & ~bitmap2[k] &
                            BITMAP_LAST_WORD_MASK(bits));
    return result != 0;
}

int __bitmap_intersects(const unsigned long *bitmap1,
                        const unsigned long *bitmap2, unsigned int bits)
{
    unsigned int k, lim = bits / BITS_PER_LONG;
    for (k = 0; k < lim; ++k)
        if (bitmap1[k] & bitmap2[k])
            return 1;

    if (bits % BITS_PER_LONG)
        if ((bitmap1[k] & bitmap2[k]) & BITMAP_LAST_WORD_MASK(bits))
            return 1;
    return 0;
}

int __bitmap_subset(const unsigned long *bitmap1,
                    const unsigned long *bitmap2, unsigned int bits)
{
    unsigned int k, lim = bits / BITS_PER_LONG;
    for (k = 0; k < lim; ++k)
        if (bitmap1[k] & ~bitmap2[k])
            return 0;

    if (bits % BITS_PER_LONG)
        if ((bitmap1[k] & ~bitmap2[k]) & BITMAP_LAST_WORD_MASK(bits))
            return 0;
    return 1;
}

void bitmap_set(unsigned long *map, unsigned int start, int len)
{
    unsigned long *p = map + BIT_WORD(start);
    const unsigned int size = start + len;
    int bits_to_set = BITS_PER_LONG - (start % BITS_PER_LONG);
    unsigned long mask_to_set = BITMAP_FIRST_WORD_MASK(start);

    while (len - bits_to_set >= 0)
    {
        *p |= mask_to_set;
        len -= bits_to_set;
        bits_to_set = BITS_PER_LONG;
        mask_to_set = ~0UL;
        p++;
    }
    if (len)
    {
        mask_to_set &= BITMAP_LAST_WORD_MASK(size);
        *p |= mask_to_set;
    }
}

void bitmap_clear(unsigned long *map, unsigned int start, int len)
{
    unsigned long *p = map + BIT_WORD(start);
    const unsigned int size = start + len;
    int bits_to_clear = BITS_PER_LONG - (start % BITS_PER_LONG);
    unsigned long mask_to_clear = BITMAP_FIRST_WORD_MASK(start);

    while (len - bits_to_clear >= 0)
    {
        *p &= ~mask_to_clear;
        len -= bits_to_clear;
        bits_to_clear = BITS_PER_LONG;
        mask_to_clear = ~0UL;
        p++;
    }
    if (len)
    {
        mask_to_clear &= BITMAP_LAST_WORD_MASK(size);
        *p &= ~mask_to_clear;
    }
}

/**
 * bitmap_find_next_zero_area_off - find a contiguous aligned zero area
 * @map: The address to base the search on
 * @size: The bitmap size in bits
 * @start: The bitnumber to start searching at
 * @nr: The number of zeroed bits we're looking for
 * @align_mask: Alignment mask for zero area
 * @align_offset: Alignment offset for zero area.
 *
 * The @align_mask should be one less than a power of 2; the effect is that
 * the bit offset of all zero areas this function finds plus @align_offset
 * is multiple of that power of 2.
 */
unsigned long bitmap_find_next_zero_area_off(unsigned long *map,
        unsigned long size,
        unsigned long start,
        unsigned int nr,
        unsigned long align_mask,
        unsigned long align_offset)
{
    unsigned long index, end, i;
again:
    index = find_next_zero_bit(map, size, start);

    /* Align allocation */
    index = __ALIGN_MASK(index + align_offset, align_mask) - align_offset;

    end = index + nr;
    if (end > size)
        return end;
    i = find_next_bit(map, end, index);
    if (i < end)
    {
        start = i + 1;
        goto again;
    }
    return index;
}

