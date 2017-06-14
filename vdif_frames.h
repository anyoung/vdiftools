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

#endif // VDIF_FRAMES_H
