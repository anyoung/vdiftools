#include <stdio.h>
#include <stdlib.h>

#include "vdif_files.h"
#include "vdif_frames.h"

int main(int argc, const char **argv) {
	int ii;
	int num_packets;
	int packets_read;
	int pkt_size_8byte;
	FFile_t file;
	vdif_header_t *pkt1, *pkt2;
	void *buf;
	
	if (argc < 3) {
		fprintf(stdout,"Usage: %s NUMPACKETS FLATFILE\n",&argv[0][2]);
		return 1;
	}
	num_packets = atoi(argv[1]);
	if (open_file_f(argv[2],&file) != -1) {
		packets_read = 0;
		while (packets_read<num_packets) {
			if (num_packets-packets_read<2000) {
				ii = read_packets_from_file_f(&file,num_packets-packets_read,&buf);
				packets_read += ii;
			} else {
				ii = read_packets_from_file_f(&file,2000,&buf);
				packets_read += ii;
			}
			if (ii==0) {
				break;
			}
			pkt1 = (vdif_header_t *)buf;
			pkt_size_8byte = pkt1->frame_length;
			pkt2 = (vdif_header_t *)(buf + 8*pkt_size_8byte*(ii-1));
			fprintf(stdout,"VDIF time is %d@%d+%d .. %d@%d+%d\n",
			  pkt1->ref_epoch,pkt1->secs_since_epoch,pkt1->data_frame,
			  pkt2->ref_epoch,pkt2->secs_since_epoch,pkt2->data_frame);
			free(buf);
		}
		close_file_f(&file);
	}
	return 0;
}
