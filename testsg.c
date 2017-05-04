#include <stdio.h>
#include <stdlib.h>

#include "vdif_files.h"

int main(int argc, const char **argv) {
	int ii;
	int num_packets;
	int num_files;
	int packets_read;
	int pkt_size_8byte;
	SGGroup_t group;
	vdif_header_t *pkt1, *pkt2;
	void *buf;
	
	if (argc < 3) {
		fprintf(stdout,"Usage: %s NUMPACKETS SGFILE [ SGFILE [ ... ] ]\n",&argv[0][2]);
		return 1;
	}
	num_packets = atoi(argv[1]);
	num_files = argc - 2;
	if (open_group_sg(num_files,&argv[2],&group) != -1) {
		packets_read = 0;
		while (packets_read<num_packets) {
			if (num_packets-packets_read<2000) {
				ii = read_packets_from_group_sg(&group,
				  num_packets-packets_read,&buf);
				packets_read += ii;
			} else {
				ii = read_packets_from_group_sg(&group,
				  2000,&buf);
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
		close_group_sg(&group);
	}
	return 0;
}
