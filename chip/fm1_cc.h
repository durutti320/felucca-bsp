/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* FM1_INLINE: register helpers that must inline into their caller, so the
 * SFR access sequence (and its timing, in ISRs and IRQ-off sections) is the
 * one written at the call site; -Os would otherwise out-line a helper with
 * several callers. RAM-resident code (.ram_text) may only use these, never
 * a called helper (build.py checks .ram_text for calls). */
#pragma once

#define FM1_INLINE static inline __attribute__((always_inline))

/* Target board: 0 = the FM-1 (default), 1 = the M-VAVE SMK-37 (Pro, Elite, MKE-P37: the same AC791N, SPL and update
 * loader as the FM-1; another panel). The build sets it (BOARD=smk37); the board's own headers are in boards/smk37. */
#ifndef BOARD_SMK37
#define BOARD_SMK37 0
#endif
/* SMK_MODEL: the model name an SMK build reports (its identity, its update loader's): the SMK-37 Pro, SMK-37 Elite
 * and MKE-P37 are one keyboard under three names, and the stock updater only takes a unit's own factory package */
#if BOARD_SMK37 && !defined(SMK_MODEL)
#define SMK_MODEL "SMK-37 Pro"
#endif
