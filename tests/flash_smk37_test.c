/* SPDX-License-Identifier: GPL-3.0-only */
/* The SMK-37's flash guard through fm1_flash_policy.h (BOARD_SMK37): what the RAM driver's test (FL_WRITE_OK) lets an
 * app, and the update loader (-DLOADER: its own FL_RANGE_OK), erase or program. Every 256-byte page start and every
 * length a caller uses, over the whole part and past it: allowed exactly inside the windows, never touching a
 * protected range (the head, BTIF + USRTRIM, the stock USR resources, key_mac). */
#include <stdio.h>
#define BOARD_SMK37 1
#ifdef LOADER
#define FL_RANGE_OK(off, n) (FL_IN(off, n, FL_APP_LO, FL_APP_HI) || FL_IN(off, n, FL_REC_LO, FL_REC_HI))
#endif
#include "fm1_flash_policy.h"
static int in(uint32_t o, uint32_t n, uint32_t lo, uint32_t hi) { return o >= lo && o < hi && n <= hi - o; }
static int want(uint32_t o, uint32_t n)
{
#ifdef LOADER
    return in(o, n, 0x4000u, 0x9D000u) || in(o, n, 0x9D000u, 0xC0000u);
#else
    return in(o, n, 0x9D000u, 0xBB000u) || in(o, n, 0xBB000u, 0xC0000u) || in(o, n, 0xC2000u, 0xEA000u) ||
           in(o, n, 0xEB000u, 0xF4000u) || (o == 0xEA000u && n == 0x1000u);
#endif
}
static int prot(uint32_t o, uint32_t n)                  /* [o, o + n) meets a protected byte (64-bit: no wrap) */
{
    static const uint32_t P[4][2] = {{0, 0x4000u}, {0xC0000u, 0xC2000u}, {0xF4000u, 0xFE000u}, {0xFF000u, 0xFFFFFFFFu}};
    uint64_t e = (uint64_t)o + n;
    int i;
    for (i = 0; i < 4; i++)
        if (o < P[i][1] && e > P[i][0])
            return 1;
    return e > 0x100000u;
}
int main(void)
{
    static const uint32_t N[] = {1, 80, 255, 256, 0x1000, 0x1001, 0x2000, 0x10000, 0x100000, 0xFFFFFF00u, 0xFFFFFFFFu};
    uint32_t o, k, bad = 0, checks = 0, i;
    static const uint32_t far[] = {0x100000u, 0x0019D000u, 0x0109D000u, 0x800C2000u, 0xFFFFF000u, 0xFFFFFFFFu};
    for (o = 0; o < 0x102000u; o += 0x100u)
        for (k = 0; k < sizeof N / sizeof *N; k++, checks++) {
            int ok = FL_WRITE_OK(o, N[k]) != 0;
            if (N[k] && ok != want(o, N[k]))
                bad++, printf("FAIL: %06X + %X allowed %d, wanted %d\n", (unsigned)o, (unsigned)N[k], ok, want(o, N[k]));
            if (N[k] && ok && prot(o, N[k]))
                bad++, printf("FAIL: %06X + %X reaches a protected range\n", (unsigned)o, (unsigned)N[k]);
        }
    for (i = 0; i < sizeof far / sizeof *far; i++)       /* (beyond the part: the high address bits are ignored) */
        for (k = 0; k < sizeof N / sizeof *N; k++, checks++)
            if (FL_WRITE_OK(far[i], N[k]))
                bad++, printf("FAIL: %08X + %X allowed\n", (unsigned)far[i], (unsigned)N[k]);
    for (o = 0x9D000u - 0x300u; o < 0x9D000u + 0x300u; o++, checks++)   /* (unaligned, around a window's edge) */
        if ((FL_WRITE_OK(o, 0x200u) != 0) != want(o, 0x200u))
            bad++, printf("FAIL: %06X + 200\n", (unsigned)o);
    _Static_assert(FM1_FLASH_PLAIN_LO == 0x9D000u && FM1_FLASH_PLAIN_HI == 0x100000u, "the SMK-37's plain window");
    printf("SMK-37 flash guard (%s): %u checks, %u failures\n",
#ifdef LOADER
           "the update loader",
#else
           "an app",
#endif
           (unsigned)checks, (unsigned)bad);
    return bad != 0;
}
