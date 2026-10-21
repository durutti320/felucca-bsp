/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
 * SMK-37 Pro port: Copyright (C) 2026 M. Geisler (genkigenki/makkuro); felucca-bsp's windows: Cam Durnsford (FLY) */
/* The SMK-37's flash map (BOARD_SMK37; fm1_flash.h includes it in place of the FM-1's): which ranges the RAM flash
 * driver may erase or program, in felucca-bsp's names (FL_DATA, FL_XDATA, FL_OTA). From genkigenki's SMK-37 layer for
 * Felucca (genkigenki/Felucca branch smk37, firmware/hal/fm1_flash_map.h; hardware notes:
 * github.com/genkigenki/fm1-to-smk37). GPL-3.0-only like that layer: this file is not under felucca-bsp's MPL-2.0.
 *
 *   0x00000-0x03FFF  head (SPL, isd_config)                                  NEVER
 *   0x04000-0x9CFFF  app area (the stock PRCT end)                           the update loader only
 *   0x9D000-0xBAFFF  VM: the store (FL_DATA, PLAT_DATA)                      the app
 *   0xBB000-0xBFFFF  VM: the update loader's staging + update record (ota.c) FL_OTA
 *   0xC0000-0xC0FFF  BTIF (Bluetooth pairing)                                NEVER
 *   0xC1000-0xC1FFF  USRTRIM (key, pad and wheel calibration, "trimdat")      NEVER
 *   0xC2000-0xE9FFF  USRFLASH: the app's big data (FL_XDATA0, PLAT_XDATA0)   the app
 *                    (the stock firmware's mapping until the app first writes there)
 *   0xEA000-0xEAFFF  the stock firmware's end marker "usrdata" at 0xEAFF8: erased once, before the first write into
 *                    FL_XDATA0 (FL_STOCKTAG), so a return to stock rebuilds its factory mapping
 *   0xEB000-0xF3FFF  free, not listed (unused)
 *   0xF4000-0xFDFFF  USR (stock resources; kept for a return to stock)       NEVER
 *   0xFE000-0xFEFFF  free, unused
 *   0xFF000-0xFFFFF  key_mac (the chip's key and MAC)                        NEVER
 * An explicit allow-list (FL_RANGE_OK); the asserts keep each window out of the protected ranges at compile time,
 * and FL_GUARD (the RAM driver's test) adds a second, independent net against them. Never widen a window or the
 * guard to make something fit: an object that does not fit moves. */
#pragma once
#include <stdint.h>

/* [off, off + n) inside [lo, hi), without wrapping: off + n can overflow, and the 1 MiB part ignores the high address
 * bits, so a wrapped range lands low */
#define FL_IN(off, n, lo, hi) ((uint32_t)(off) >= (lo) && (uint32_t)(off) <= (hi) && \
                               (uint32_t)(n) <= (hi) - (uint32_t)(off))
/* the flash part: 0xEB6014 (1 MiB, read from a unit over the mask-ROM UBOOT), or the FM-1's 0x856014. The driver uses
 * only generic SPI NOR commands (9F, 05/35, 06, 20, 02, 0B) */
#define FL_JEDEC_OK(id) ((id) == 0xEB6014u || (id) == 0x856014u)

#define FL_HEAD_HI      0x00004000u                /* protected: [0, FL_HEAD_HI) */
#define FL_CAL_LO       0x000C0000u                /* protected: BTIF + USRTRIM */
#define FL_CAL_HI       0x000C2000u
#define FL_USRRES_LO    0x000F4000u                /* protected: the stock USR resources */
#define FL_USRRES_HI    0x000FE000u
#define FL_KEYMAC_LO    0x000FF000u                /* protected: key_mac and above */
#define FL_APP_LO       0x00004000u                /* the update loader: the app area */
#define FL_APP_HI       0x0009D000u
#define FL_REC_LO       0x0009D000u                /* the update loader: where it may drop records (the VM only) */
#define FL_REC_HI       0x000C0000u
#define FL_PLAIN_LO     0x0009D000u                /* the plain XIP window, from the VM up (fl_plain_window_init) */
#define FL_DATA_LO      0x0009D000u                /* the store */
#define FL_DATA_HI      0x000BB000u
#define FL_OTA_LO       0x000BB000u                /* the update loader's staging, ota.c */
#define FL_OTA_HI       0x000C0000u
#define FL_XDATA0_LO    0x000C2000u                /* the app's big data (samples): USRFLASH */
#define FL_XDATA0_HI    0x000EA000u
#define FL_XDATA1_LO    0x000EA000u                /* (no second window on the SMK: empty) */
#define FL_XDATA1_HI    0x000EA000u
#define FL_STOCKTAG_LO  0x000EA000u                /* the stock end marker's sector: erased whole, once */
#define FL_STOCKTAG_HI  0x000EB000u
#define FL_STOCKTAG_OFF 0x000EAFF8u
#define FL_STOCKTAG_OK(off, n) ((uint32_t)(off) == FL_STOCKTAG_LO && (uint32_t)(n) == FL_STOCKTAG_HI - FL_STOCKTAG_LO)

/* a window [lo, hi) is allowed: sector aligned, and clear of every protected range */
#define FL_WIN_SAFE(lo, hi) ((lo) % 0x1000u == 0u && (hi) % 0x1000u == 0u && (lo) < (hi) && \
                             (lo) >= FL_HEAD_HI && (hi) <= FL_KEYMAC_LO && \
                             ((hi) <= FL_CAL_LO || (lo) >= FL_CAL_HI) && \
                             ((hi) <= FL_USRRES_LO || (lo) >= FL_USRRES_HI))
_Static_assert(FL_WIN_SAFE(FL_DATA_LO, FL_DATA_HI), "SMK: FL_DATA overlaps a protected range");
_Static_assert(FL_WIN_SAFE(FL_OTA_LO, FL_OTA_HI), "SMK: FL_OTA overlaps a protected range");
_Static_assert(FL_WIN_SAFE(FL_XDATA0_LO, FL_XDATA0_HI), "SMK: FL_XDATA0 overlaps a protected range");
_Static_assert(FL_XDATA1_LO == FL_XDATA1_HI, "SMK: no second data window");
_Static_assert(FL_WIN_SAFE(FL_STOCKTAG_LO, FL_STOCKTAG_HI) && FL_STOCKTAG_LO == FL_XDATA0_HI &&
               FL_STOCKTAG_OFF + 8u == FL_STOCKTAG_HI, "SMK: the stock marker sector");
_Static_assert(FL_WIN_SAFE(FL_APP_LO, FL_APP_HI) && FL_WIN_SAFE(FL_REC_LO, FL_REC_HI), "SMK: the loader's windows");
_Static_assert(FL_REC_HI == FL_CAL_LO && FL_OTA_HI <= FL_REC_HI && FL_OTA_LO >= FL_REC_LO,
               "SMK: the update record (the OTA area) lies in the loader's record window, below BTIF");
_Static_assert(FL_APP_HI <= FL_DATA_LO && FL_DATA_HI <= FL_OTA_LO, "SMK: windows overlap");

/* the store (the VM; an app's store objects may also lie in USRFLASH, its big-data window: both are the app's), and
 * the app's big data */
#define FL_STORE_OK(off, n) (FL_IN(off, n, FL_DATA_LO, FL_DATA_HI) || FL_IN(off, n, FL_XDATA0_LO, FL_XDATA0_HI))
#define FL_XDATA_OK(off, n) FL_IN(off, n, FL_XDATA0_LO, FL_XDATA0_HI)
/* where the RAM driver may erase / program: the app's windows, the staging, the stock marker's sector (whole); the
 * update loader defines its own (loader.c) */
#ifndef FL_RANGE_OK
#define FL_RANGE_OK(off, n) (FL_STORE_OK(off, n) || FL_XDATA_OK(off, n) || FL_IN(off, n, FL_OTA_LO, FL_OTA_HI) || \
                             FL_STOCKTAG_OK(off, n))
#endif
/* the second, independent net in the RAM driver: [off, off + n) touches a protected range (also when a caller
 * supplies its own FL_RANGE_OK, as the update loader does) */
#define FL_HITS(off, n, lo, hi) ((uint32_t)(off) < (hi) && \
                                 ((uint32_t)(off) >= (lo) || (uint32_t)(n) > (lo) - (uint32_t)(off)))
#define FL_PROTECTED(off, n) (FL_HITS(off, n, 0u, FL_HEAD_HI) || FL_HITS(off, n, FL_CAL_LO, FL_CAL_HI) || \
                              FL_HITS(off, n, FL_USRRES_LO, FL_USRRES_HI) || (uint32_t)(off) >= FL_KEYMAC_LO || \
                              (uint32_t)(n) > FL_KEYMAC_LO - (uint32_t)(off))
#define FL_GUARD(off, n) (FL_RANGE_OK(off, n) && !FL_PROTECTED(off, n))
