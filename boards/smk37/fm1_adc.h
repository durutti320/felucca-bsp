/* SPDX-License-Identifier: GPL-3.0-only
 * SMK-37 port: Copyright (C) 2026 M. Geisler (genkigenki/makkuro) */
/* The SMK-37 has no MASTER pot and reads its analog inputs (pads, faders, wheels, pedal, battery) in the input scan:
 * fm1_adc_init and fm1_adc_read (FM1_ADC_MASTER: K1 as a virtual pot, FM1_ADC_BATT) are in smk_input.h */
#pragma once
#include "fm1_input.h"
