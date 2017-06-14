#ifndef VDIF_FRAMES_H
#define VDIF_FRAMES_H

#include <stdint.h>

typedef struct vdif_header {
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
	uint32_t eud0:24;
	uint32_t edv:8;
	// word5
	uint32_t eud1;
	// word6
	uint32_t eud2;
	// word7
	uint32_t eud3;
} vdif_header_t;

/* Extract samples from the given VDIF frame.
 * Arguments:
 *  frm -- Pointer to vdif_header_t that is the VDIF header of a full
 *         frame
 *  out -- Pointer address, memory will be allocated for sample storage
 *  nch -- Address where number of channels will be stored
 *  bps -- Address where number of bits per sample will be stored
 *  cmp -- Address where number of sample components (re, or re+im)
 *         stored
 * Returns:
 *  cnt -- Total number of samples read.
 * Notes:
 *  Upon successful extraction of samples, integer values will be stored
 *  in the memory allocated to *out. The number of channels, bits per
 *  sample, and number of components (1 for real, or 2 for real +
 *  imaginary) are stored in *nch, *bps, and *cmp, respectively. The
 *  total number of samples read is returned.
 */
int get_samples(vdif_header_t *frm, uint32_t **out, int *nch, int *bps, int *cmp);

#endif // VDIF_FRAMES_H
