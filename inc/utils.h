#ifndef _COMFUNC_H_
#define _COMFUNC_H_

#include <stdarg.h>
#include <types.h>

#if 0
#define _U  0x01    /* upper */
#define _L  0x02    /* lower */
#define _D  0x04    /* digit */
#define _C  0x08    /* cntrl */
#define _P  0x10    /* punct */
#define _S  0x20    /* white space (space/lf/tab) */
#define _X  0x40    /* hex digit */
#define _SP 0x80    /* hard space (0x20) */
#endif

#define set_bit(x, bit) ((x) |= 1 << (bit))
#define clr_bit(x, bit) ((x) &= ~(1 << (bit)))
#define tst_bit(x, bit) ((x) & (1 << (bit)))
#define get_bits(val,x1,x2) (((val)>>(x1))&((1<<((x2)-(x1)+1))-1))

#ifndef min
#define min(a, b) ((a)<(b) ? (a):(b))
#endif
#ifndef max
#define max(a, b) ((a)>(b) ? (a):(b))
#endif

#define array_size(array) (sizeof(array)/sizeof(*array))

#define FIELD_SIZEOF(t, f) (sizeof(((t*)0)->f))
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#ifndef offset_of
# define offset_of(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)
#endif
/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:    the pointer to the member.
 * @type:   the type of the container struct this is embedded in.
 * @member: the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) \
        ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/*
 * This looks more complex than it should be. But we need to
 * get the type for the ~ right in round_down (it needs to be
 * as wide as the result!), and we want to evaluate the macro
 * arguments just once each.
 */
#define __round_mask(x, y) ((uint32_t)((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))

#define _swab16(x) ((uint16_t)(             \
    (((uint16_t)(x) & (uint16_t)0x00ffU) << 8) |    \
    (((uint16_t)(x) & (uint16_t)0xff00U) >> 8)))

#define _swab32(x) ((uint32_t)(             \
    (((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) |\
    (((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) |\
    (((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) |\
    (((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24)))

#ifdef BIG_ENDIAN
#define cpu_to_le32(x) ((uint32_t)_swab32((x)))
#define le32_to_cpu(x) ((uint32_t)_swab32((uint32_t)(x)))
#define cpu_to_le16(x) ((uint16_t)_swab16((x)))
#define le16_to_cpu(x) ((uint16_t)_swab16((uint16_t)(x)))
#define cpu_to_be32(x) ((uint32_t)(x))
#define be32_to_cpu(x) ((uint32_t)(x))
#define cpu_to_be16(x) ((uint16_t)(x))
#define be16_to_cpu(x) ((uint16_t)(x))
#else
#define cpu_to_le32(x) ((uint32_t)(x))
#define le32_to_cpu(x) ((uint32_t)(x))
#define cpu_to_le16(x) ((uint16_t)(x))
#define le16_to_cpu(x) ((uint16_t)(x))
#define cpu_to_be32(x) ((uint32_t)_swab32((x)))
#define be32_to_cpu(x) ((uint32_t)_swab32((uint32_t)(x)))
#define cpu_to_be16(x) ((uint16_t)_swab16((x)))
#define be16_to_cpu(x) ((uint16_t)_swab16((uint16_t)(x)))
#endif

#define bcd2bin(val) (((val) & 0x0f) + ((val) >> 4) * 10)
#define bin2bcd(val) ((((val) / 10) << 4) + (val) % 10)

#define _abs(a) ((a) < 0 ? -(a) : (a))
#define swap(a, b) do {a ^= b; b ^= a; a ^= b;} while (0)

uint8_t checksum(const void *data, int len);

/* byte order */
uint32_t get_le_val(const uint8_t *p, int bytes);
uint32_t get_be_val(const uint8_t *p, int bytes);
void put_le_val(uint32_t val, uint8_t *p, int bytes);
void put_be_val(uint32_t val, uint8_t *p, int bytes);

int split(char *str, char **arr, const char *del);
int is_all_xx(const void *s1, uint8_t val, int n);

const char *_arr2str(const void *arr, int len, void *dest, int maxlen, int del);
const char *arr2str(const void *arr, int len, void *dest, int maxlen);
int str2arr(const char *str, void *out, int maxlen);
char *i2str(uint8_t val, char *destStr);

void reverse(void *buff, size_t len);

uint16_t crc16(uint16_t crc, const void *buf, int size);
int count_bit_in_long(unsigned long x);

void *memmem(const void *buf1, int len1, const void *buf2, int len2);
void memadd(const void *data, int add, size_t n);

bool is_all_bcd(const uint8_t *data, size_t n);
void hex2bcd(uint32_t value, uint8_t *bcd, uint8_t bytes);
uint32_t bcd2hex(uint8_t *bcd, uint8_t bytes);

void float_to_bcd(float f, ubase_t bytes, ubase_t frabits, u8 *bcd);
float bcd_to_float(u8 *bcd, ubase_t bytes, ubase_t frabits, bool withsign);

bool is_bcd_time_valid(const uint8_t *bcdtime);
uint8_t week(uint8_t y, uint8_t m, uint8_t d);

unsigned int jiffies_to_msecs(const unsigned long j);


void *os_memset(void *src, int c, size_t n);
void *os_memcpy(void *dest, const void *src, size_t n);

int32_t os_strncmp(const char *cs, const char *ct, size_t count);
int32_t os_strcmp(const char *cs, const char *ct);
size_t  os_strlen(const char *src);
char   *os_strdup(const char *s);

char   *os_strchr(char *s, char c);
char   *os_strstr(const char *str1, const char *str2);
int32_t os_sscanf(const char *buf, const char *fmt, ...);
char   *os_strncpy(char *dest, const char *src, size_t n);
char   *os_strcpy(char *des, const char *src);
void   *os_memmove(void *dest, const void *src, size_t n);
int32_t os_memcmp(const void *cs, const void *ct, size_t count);
uint32_t os_strcasecmp(const char *a, const char *b);

#define memset    os_memset
#define memcpy    os_memcpy

#define strncmp   os_strncmp
#define strcmp    os_strcmp
#define strlen    os_strlen
#define strdup    os_strdup

#define strchr    os_strchr
#define strstr    os_strstr
#define sscanf    os_sscanf
#define strncpy   os_strncpy
#define strcpy    os_strcpy
#define memmove   os_memmove
#define memcmp    os_memcmp
#define strcasecmp  os_strcasecmp

long strtol(const char *str, char **endptr, int base);
long long strtoll(const char *str, char **endptr, int base);
unsigned long strtoul(const char *str, char **endptr, int base);

#endif
