/* SPDX-License-Identifier: ((GPL-2.0 WITH Linux-syscall-note)) OR BSD-3-Clause) */
/*
 * This file is provided under a dual BSD/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 *
 * Copyright(c) 2018 Intel Corporation. All rights reserved.
 *
 * Author: Seppo Ingalsuo <seppo.ingalsuo@linux.intel.com>
 */

#ifndef EQ_H
#define EQ_H

/* FIR EQ type */

/* Component will reject non-matching configuration. The version number need
 * to be incremented with any ABI changes in function fir_cmd().
 */
#define SOF_EQ_FIR_ABI_VERSION  1

#define SOF_EQ_FIR_IDX_SWITCH	0

#define SOF_EQ_FIR_MAX_SIZE 4096 /* Max size allowed for coef data in bytes */

#define SOF_EQ_FIR_MAX_LENGTH 192 /* Max length for individual filter */

/*
 * eq_fir_configuration data structure contains this information
 *     uint32_t size
 *	   This is the number of bytes need to store the received EQ
 *	   configuration.
 *     uint16_t channels_in_config
 *         This describes the number of channels in this EQ config data. It
 *         can be different from PLATFORM_MAX_CHANNELS.
 *     uint16_t number_of_responses
 *         0=no responses, 1=one response defined, 2=two responses defined, etc.
 *     int16_t data[]
 *         assign_response[channels_in_config]
 *             0 = use first response, 1 = use 2nd response, etc.
 *             E.g. {0, 0, 0, 0, 1, 1, 1, 1} would apply to channels 0-3 the
 *	       same first defined response and for to channels 4-7 the second.
 *         coef_data[]
 *             Repeated data
 *             { filter_length, output_shift, h[] }
 *	       for every EQ response defined where vector h has filter_length
 *             number of coefficients. Coefficients in h[] are in Q1.15 format.
 *             E.g. 16384 (Q1.15) = 0.5. The shifts are number of right shifts.
 *
 * NOTE: The channels_in_config must be even to have coef_data aligned to
 * 32 bit word in RAM. Therefore a mono EQ assign must be duplicated to 2ch
 * even if it would never used. Similarly a 5ch EQ assign must be increased
 * to 6ch. EQ init will return an error if this is not met.
 *
 * NOTE: The filter_length must be multiple of four. Therefore the filter must
 * be padded from the end with zeros have this condition met.
 */

struct sof_eq_fir_config {
	uint32_t size;
	uint16_t channels_in_config;
	uint16_t number_of_responses;
	int16_t data[];
};

struct sof_eq_fir_coef_data {
	int16_t length; /* Number of FIR taps */
	int16_t out_shift; /* Amount of right shifts at output */
	int16_t coef[]; /* FIR coefficients */
};

/* In the struct above there's two words (length, shift) before the actual
 * FIR coefficients. This information is used in parsing of the config blob.
 */
#define SOF_EQ_FIR_COEF_NHEADER 2

/* IIR EQ type */

/* Component will reject non-matching configuration. The version number need
 * to be incremented with any ABI changes in function fir_cmd().
 */
#define SOF_EQ_IIR_ABI_VERSION  1

#define SOF_EQ_IIR_IDX_SWITCH   0

#define SOF_EQ_IIR_MAX_SIZE 1024 /* Max size allowed for coef data in bytes */

#define SOF_EQ_IIR_MAX_RESPONSES 8 /* A blob can define max 8 IIR EQs */

/* eq_iir_configuration
 *     uint32_t channels_in_config
 *         This describes the number of channels in this EQ config data. It
 *         can be different from PLATFORM_MAX_CHANNELS.
 *     uint32_t number_of_responses_defined
 *         0=no responses, 1=one response defined, 2=two responses defined, etc.
 *     int32_t data[]
 *         Data consist of two parts. First is the response assign vector that
 *	   has length of channels_in_config. The latter part is coefficient
 *         data.
 *         uint32_t assign_response[channels_in_config]
 *             -1 = not defined, 0 = use first response, 1 = use 2nd, etc.
 *             E.g. {0, 0, 0, 0, -1, -1, -1, -1} would apply to channels 0-3 the
 *             same first defined response and leave channels 4-7 unequalized.
 *         coefficient_data[]
 *             <1st EQ>
 *             uint32_t num_biquads
 *             uint32_t num_biquads_in_series
 *             <1st biquad>
 *             int32_t coef_a2       Q2.30 format
 *             int32_t coef_a1       Q2.30 format
 *             int32_t coef_b2       Q2.30 format
 *             int32_t coef_b1       Q2.30 format
 *             int32_t coef_b0       Q2.30 format
 *             int32_t output_shift  number of shifts right, shift left is negative
 *             int32_t output_gain   Q2.14 format
 *             <2nd biquad>
 *             ...
 *             <2nd EQ>
 *
 *         Note: A flat response biquad can be made with a section set to
 *         b0 = 1.0, gain = 1.0, and other parameters set to 0
 *         {0, 0, 0, 0, 1073741824, 0, 16484}
 */

struct sof_eq_iir_config {
	uint32_t size;
	uint32_t channels_in_config;
	uint32_t number_of_responses;
	int32_t data[]; /* eq_assign[channels], eq 0, eq 1, ... */
};

struct sof_eq_iir_header_df2t {
	uint32_t num_sections;
	uint32_t num_sections_in_series;
	int32_t biquads[]; /* Repeated biquad coefficients */
};

struct sof_eq_iir_biquad_df2t {
	int32_t a2; /* Q2.30 */
	int32_t a1; /* Q2.30 */
	int32_t b2; /* Q2.30 */
	int32_t b1; /* Q2.30 */
	int32_t b0; /* Q2.30 */
	int32_t output_shift; /* Number of right shifts */
	int32_t output_gain;  /* Q2.14 */
};

/* A full 22th order equalizer with 11 biquads cover octave bands 1-11 in
 * in the 0 - 20 kHz bandwidth.
 */
#define SOF_EQ_IIR_DF2T_BIQUADS_MAX 11

/* The number of int32_t words in sof_eq_iir_header_df2t */
#define SOF_EQ_IIR_NHEADER_DF2T 2

/* The number of int32_t words in sof_eq_iir_biquad_df2t */
#define SOF_EQ_IIR_NBIQUAD_DF2T 7

#endif /* EQ_H */
