#include <utils.h>
#include "bitmap.h"
#include "bitops.h"

/*
 * ffs - find first bit in word.
 * @word: The word to search
 *
 * Undefined if no bit exists, so code should check against 0 first.
 */
unsigned long __ffs(unsigned long word)
{
    int num = 0;

#if BITS_PER_LONG == 64
    if ((word & 0xffffffff) == 0)
    {
        num += 32;
        word >>= 32;
    }
#endif
    if ((word & 0xffff) == 0)
    {
        num += 16;
        word >>= 16;
    }
    if ((word & 0xff) == 0)
    {
        num += 8;
        word >>= 8;
    }
    if ((word & 0xf) == 0)
    {
        num += 4;
        word >>= 4;
    }
    if ((word & 0x3) == 0)
    {
        num += 2;
        word >>= 2;
    }
    if ((word & 0x1) == 0)
        num += 1;
    return num;
}

/*
 * fls - find last (most-significant) bit set
 * @x: the word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */
unsigned long __fls(unsigned long x)
{
    int r = 32;

    if (!x)
        return 0;
    if (!(x & 0xffff0000u))
    {
        x <<= 16;
        r -= 16;
    }
    if (!(x & 0xff000000u))
    {
        x <<= 8;
        r -= 8;
    }
    if (!(x & 0xf0000000u))
    {
        x <<= 4;
        r -= 4;
    }
    if (!(x & 0xc0000000u))
    {
        x <<= 2;
        r -= 2;
    }
    if (!(x & 0x80000000u))
    {
        x <<= 1;
        r -= 1;
    }
    return r;
}


/*
 * This is a common helper function for find_next_bit and
 * find_next_zero_bit.  The difference is the "invert" argument, which
 * is XORed with each fetched word before searching it for one bits.
 */
static unsigned long _find_next_bit(const unsigned long *addr,
                                    unsigned long nbits,
                                    unsigned long start,
                                    unsigned long invert)
{
    unsigned long tmp;

    if (!nbits || start >= nbits) return nbits;

    tmp = addr[start / BITS_PER_LONG] ^ invert;

    /* Handle 1st word. */
    tmp &= BITMAP_FIRST_WORD_MASK(start);
    start = round_down(start, BITS_PER_LONG);

    while (!tmp)
    {
        start += BITS_PER_LONG;
        if (start >= nbits) return nbits;

        tmp = addr[start / BITS_PER_LONG] ^ invert;
    }

    return min(start + __ffs(tmp), nbits);
}

/*
 * Find the next set bit in a memory region.
 */
unsigned long find_next_bit(const unsigned long *addr, unsigned long size,
                            unsigned long offset)
{
    return _find_next_bit(addr, size, offset, 0UL);
}


unsigned long find_next_zero_bit(const unsigned long *addr, unsigned long size,
                                 unsigned long offset)
{
    return _find_next_bit(addr, size, offset, ~0UL);
}


/*
 * Find the first set bit in a memory region.
 */
unsigned long find_first_bit(const unsigned long *addr, unsigned long size)
{
    unsigned long idx;

    for (idx = 0; idx * BITS_PER_LONG < size; idx++)
    {
        if (addr[idx])
            return min(idx * BITS_PER_LONG + __ffs(addr[idx]), size);
    }

    return size;
}

/*
 * Find the first cleared bit in a memory region.
 */
unsigned long find_first_zero_bit(const unsigned long *addr, unsigned long size)
{
    unsigned long idx;

    for (idx = 0; idx * BITS_PER_LONG < size; idx++)
    {
        if (addr[idx] != ~0UL)
            return min(idx * BITS_PER_LONG + __ffz(addr[idx]), size);
    }

    return size;
}


unsigned long find_last_bit(const unsigned long *addr, unsigned long size)
{
    if (size)
    {
        unsigned long val = BITMAP_LAST_WORD_MASK(size);
        unsigned long idx = (size - 1) / BITS_PER_LONG;

        do
        {
            val &= addr[idx];
            if (val)
                return idx * BITS_PER_LONG + __fls(val);

            val = ~0ul;
        }
        while (idx--);
    }
    return size;
}


#ifdef __BIG_ENDIAN

/* include/linux/byteorder does not support "unsigned long" type */
static inline unsigned long ext2_swab(const unsigned long y)
{
#if BITS_PER_LONG == 64
    return (unsigned long) __swab64((u64) y);
#elif BITS_PER_LONG == 32
    return (unsigned long) __swab32((u32) y);
#else
#error BITS_PER_LONG not defined
#endif
}

#if !defined(find_next_bit_le) || !defined(find_next_zero_bit_le)
static unsigned long _find_next_bit_le(const unsigned long *addr,
                                       unsigned long nbits, unsigned long start, unsigned long invert)
{
    unsigned long tmp;

    if (!nbits || start >= nbits)
        return nbits;

    tmp = addr[start / BITS_PER_LONG] ^ invert;

    /* Handle 1st word. */
    tmp &= ext2_swab(BITMAP_FIRST_WORD_MASK(start));
    start = round_down(start, BITS_PER_LONG);

    while (!tmp)
    {
        start += BITS_PER_LONG;
        if (start >= nbits)
            return nbits;

        tmp = addr[start / BITS_PER_LONG] ^ invert;
    }

    return min(start + __ffs(ext2_swab(tmp)), nbits);
}
#endif

#ifndef find_next_zero_bit_le
unsigned long find_next_zero_bit_le(const void *addr, unsigned
                                    long size, unsigned long offset)
{
    return _find_next_bit_le(addr, size, offset, ~0UL);
}
#endif

#ifndef find_next_bit_le
unsigned long find_next_bit_le(const void *addr, unsigned
                               long size, unsigned long offset)
{
    return _find_next_bit_le(addr, size, offset, 0UL);
}
#endif

#endif /* __BIG_ENDIAN */
