#include "r2dbe_vdif.h"

uint64_t psn64(vdif_r2dbe_header_t *hdr) {
	uint64_t psn;
	psn =          (uint64_t)hdr->psn_lo & (uint64_t)0x00000000FFFFFFFF;
	psn |= ((uint64_t)hdr->psn_hi << 32) & (uint64_t)0xFFFFFFFF00000000;
	return psn;
}

float pps_offset_time(vdif_r2dbe_header_t *hdr) {
	float pps;
	pps = (float)hdr->pps_offset / R2DBE_FPGA_CLOCK;
	return pps;
}

