#ifndef R2DBE_VDIF_H
#define R2DBE_VDIF_H

#include <stdint.h>

#define R2DBE_FPGA_CLOCK 256000000.0f
#define R2DBE_SAMPLE_RATE 4096000000.0f

typedef struct vdif_r2dbe_header {
	// word0
	uint32_t secs_since_epoch:30;
	uint32_t legacy_mode:1;
	uint32_t invalid_data:1;
	// word1
	uint32_t data_frame:24;
	uint32_t ref_epoch:6;
	uint32_t unassigned:2;
	// word2
	uint32_t frame_length:24;
	uint32_t log2_chans:5;
	uint32_t vdif_vers:3;
	// word3
	uint32_t station_id:16;
	uint32_t thread_id:10;
	uint32_t bits_per_sample:5;
	uint32_t complex:1;
	// word4
	uint32_t pol_rcp_not_lcp:1;
	uint32_t bdc_hi_not_lo:1;
	uint32_t rec_hi_not_lo:1;
	uint32_t eud0:21;
	uint32_t edv:8;
	// word5
	uint32_t pps_offset;
	// word6
	uint32_t psn_lo;
	// word7
	uint32_t psn_hi;
} vdif_r2dbe_header_t;

typedef struct vdif_r2dbe_packet {
	vdif_r2dbe_header_t header;
	void *data;
} vdif_r2dbe_packet_t;

/* Return PSN as an unsigned 64-bit integer.
 * Arguments:
 *  hdr -- Pointer to vdif_r2dbe_header_t from which to extract the PSN.
 * Returns:
 *  psn -- 64-bit integer PSN.
 */
uint64_t psn64(vdif_r2dbe_header_t *hdr);

/* Return PPS offset as seconds.
 * Arguments:
 *  hdr -- Pointer to vdif_r2dbe_header_t from which to extract PPS offset.
 * Returns:
 *  pps -- PPS offset in seconds.
 */
float pps_offset_time(vdif_r2dbe_header_t *hdr);

#endif // R2DBE_VDIF_H
