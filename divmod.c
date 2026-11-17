/*
 * divmod.c - software integer division for ALTROM
 *
 * ARMv5TE has no divide instruction and the link uses -nostdlib, so clang's
 * calls to the AEABI helpers (emitted for / and % at -Oz) have nothing to
 * resolve to.  Include this FIRST in entry.c, before any code that divides.
 *
 * AEABI convention: *divmod helpers return quotient in r0, remainder in r1,
 * which is what returning a uint64_t (low = quotient, high = remainder) does.
 */

typedef unsigned int size_t;

static uint64_t udivmod32(uint32_t n, uint32_t d)
{
	uint32_t q = 0, r = 0;

	if (d == 0)
		return 0;
	for (int i = 31; i >= 0; i--) {
		uint32_t carry = r >> 31;
		r = (r << 1) | ((n >> i) & 1);
		if (carry || r >= d) {
			r -= d;
			q |= 1u << i;
		}
	}
	return ((uint64_t)r << 32) | q;
}

__attribute__((used)) uint32_t __aeabi_uidiv(uint32_t n, uint32_t d)
{
	return (uint32_t)udivmod32(n, d);
}

__attribute__((used)) uint64_t __aeabi_uidivmod(uint32_t n, uint32_t d)
{
	return udivmod32(n, d);
}

__attribute__((used)) int32_t __aeabi_idiv(int32_t n, int32_t d)
{
	uint32_t un = n < 0 ? 0u - (uint32_t)n : (uint32_t)n;
	uint32_t ud = d < 0 ? 0u - (uint32_t)d : (uint32_t)d;
	uint32_t q  = (uint32_t)udivmod32(un, ud);

	return ((n < 0) != (d < 0)) ? -(int32_t)q : (int32_t)q;
}

uint64_t __aeabi_idivmod(int32_t n, int32_t d)
{
	uint32_t un = n < 0 ? 0u - (uint32_t)n : (uint32_t)n;
	uint32_t ud = d < 0 ? 0u - (uint32_t)d : (uint32_t)d;
	uint64_t qr = udivmod32(un, ud);
	int32_t  q  = (int32_t)(uint32_t)qr;
	int32_t  r  = (int32_t)(uint32_t)(qr >> 32);

	if ((n < 0) != (d < 0)) q = -q;
	if (n < 0)              r = -r;
	return ((uint64_t)(uint32_t)r << 32) | (uint32_t)q;
}

void* memcpy(void* dest, const void* src, size_t n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

// EABI aliases that the compiler expects
void __aeabi_memcpy(void* dest, const void* src, size_t n) {
    memcpy(dest, src, n);
}
void __aeabi_memcpy4(void* dest, const void* src, size_t n) {
    memcpy(dest, src, n);
}

// Software implementation of 64-bit multiplication
int64_t __aeabi_lmul(int64_t a, int64_t b) {
    uint32_t al = (uint32_t)a;
    int32_t ah = (int32_t)(a >> 32);
    uint32_t bl = (uint32_t)b;
    int32_t bh = (int32_t)(b >> 32);

    uint64_t low = (uint64_t)al * bl;
    int64_t mid1 = (int64_t)al * bh;
    int64_t mid2 = (int64_t)ah * bl;

    return (int64_t)low + ((mid1 + mid2) << 32);
}

uint64_t __aeabi_ldivmod(uint64_t numerator, uint64_t denominator) {
    uint64_t result = 0;
    uint64_t current = 1;
    
    // Catch division by zero
    if (denominator == 0) return 0;

    // Shift denominator up until it matches or exceeds numerator
    while (denominator < numerator && (denominator & (1ULL << 63)) == 0) {
        denominator <<= 1;
        current <<= 1;
    }

    // Perform long division bit-by-bit
    while (current > 0) {
        if (numerator >= denominator) {
            numerator -= denominator;
            result |= current;
        }
        denominator >>= 1;
        current >>= 1;
    }

    // According to EABI, the quotient is returned in r0-r1 (lower 64-bits of return value)
    // and the remainder is returned in r2-r3. A standard C 64-bit return implicitly packs this.
    return result; 
}
