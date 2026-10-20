/* SPDX-License-Identifier: GPL-3.0-only
 * SMK-37 port: Copyright (C) 2026 M. Geisler (genkigenki/makkuro); felucca-bsp board: LOOM-8 SMK port */
/* The SMK-37's board (BOARD_SMK37; the build puts boards/smk37 on the include path in place of boards/fm1): the
 * input HAL keeps fm1_input.h's API (fm1_in, fm1_input_init, fm1_input_tick, fm1_enc_take, fm1_input_edges, fm1_led)
 * and adds the SMK's extras (velocity, pads, faders, wheels): smk_input.h, from genkigenki's SMK-37 layer for Felucca
 * (genkigenki/Felucca branch smk37). What the SMK has no hardware for (the FM-1's LED glow, its power-on sweep and
 * idle animation) are no-ops: smk_compat.h. GPL-3.0-only like that layer, not felucca-bsp's MPL-2.0. */
#pragma once
#include "smk_input.h"
#include "smk_compat.h"
static uint8_t fm1_led_dim[FM1_NCOL];   /* (the FM-1's dim layer: none on the SMK, its button LEDs are on / off) */
