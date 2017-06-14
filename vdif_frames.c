#include <stdio.h>
#include <stdlib.h>

#include "vdif_frames.h"

int get_samples(vdif_header_t *frm, uint32_t **out, int *nch, int *bps, int *cmp) {
	int ii, jj;
	int mask;
	int num = 0;
	int samp_per_w32;
	int w32len_data;
	uint32_t *data_in, *data_out;
	
	if (frm->invalid_data) {
		return num;
	}
	*bps = frm->bits_per_sample+1;
	*nch = 0x01 << frm->log2_chans;
	*cmp = frm->complex ? 2 : 1;
	samp_per_w32 = 32 / (*bps);
	// payload length, frame_length is given as 8-byte words
	w32len_data = (frm->frame_length*8 - sizeof(vdif_header_t))/4;
	num = w32len_data*samp_per_w32;
	*out = (int32_t *)malloc(num*sizeof(int32_t));
	mask = (0x01 << (*bps)) - 1;
	data_out = *out;
	data_in = (int32_t *)((void *)frm + sizeof(vdif_header_t));
	for (ii=0; ii<w32len_data; ii++) {
		for (jj=0; jj<samp_per_w32; jj++) {
			*data_out = ((*data_in) >> jj*(*bps)) & mask;
			data_out++;
		}
		data_in++;
	}
	return num;
}
