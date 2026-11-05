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

uint32_t __aeabi_uidiv(uint32_t n, uint32_t d)
{
	return (uint32_t)udivmod32(n, d);
}

uint64_t __aeabi_uidivmod(uint32_t n, uint32_t d)
{
	return udivmod32(n, d);
}

int32_t __aeabi_idiv(int32_t n, int32_t d)
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
