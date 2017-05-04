#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ioutils.h"
#include "vdif_files.h"

/////////////////////////////////////////////////// INTERNAL DEFINITIONS

/* Test if header is a valid scatter-gather file header.
 * 
 * Returns 1 if valid, 0 if invalid
 */
int is_valid_sgfile_header(sgf_header_t *header) {
	if (header->sync_word == SGF_HEADER_SYNC_WORD &&
	  header->version == SGF_HEADER_VERSION &&
	  header->packet_format == SGF_HEADER_PACKET_FORMAT) {
		return 1;
	}
	return 0;
}

/* Open a scatter-gather file and initialize the SGFile_t struct.
 * 
 * Returns 1 on success, -1 on failure.
 */
int open_file_sg(const char *filename, SGFile_t *sgfile) {
	size_t len;
	sgb_header_t sgb_hdr;
	
	// initialize filename
	len = strlen(filename) + 1; // plus one for trailing '\0'
	sgfile->filename = (char *)malloc(len);
	strcpy(sgfile->filename,filename);
	// open file and store descriptor
	sgfile->fd = open(filename,O_RDONLY);
	if (sgfile->fd == -1) {
		fprintf(stderr,
		  "%s.%s(%d): unable to open '%s'\n",
		  __FILE__,__FUNCTION__,__LINE__,
		  filename);
		return -1;
	}
	// initialize raw header and check if valid
	if (ioutils_read(sgfile->fd,(void *)&sgfile->header,sizeof(sgf_header_t),&len) <= 0) {
		fprintf(stderr,
		  "%s.%s(%d): failed to read scatter-gather header bytes from '%s'\n",
		  __FILE__,__FUNCTION__,__LINE__,
		  filename);
		return -1;
	}
	if (!is_valid_sgfile_header(&sgfile->header)) {
		fprintf(stderr,
		  "%s.%s(%d): invalid scatter-gather header in file '%s'\n",
		  __FILE__,__FUNCTION__,__LINE__,
		  filename);
		return -1;
	}
	// reset index
	sgfile->index = 0;
	// get next block number
	if (ioutils_read(sgfile->fd,(void *)&sgb_hdr,sizeof(sgb_header_t),&len) == -1) {
		fprintf(stderr,
		  "%s.%s(%d): error reading first block in '%s'\n",
		  __FILE__,__FUNCTION__,__LINE__,
		  filename);
		return -1;
	}
	if (len == 0) {
		sgfile->next_block_num = -1;
		sgfile->next_block_size = -1;
	} else {
		sgfile->next_block_num = sgb_hdr.block_num;
		sgfile->next_block_size = sgb_hdr.block_size;
	}
	return 1;
}

/* Close scatter-gather file.
 */
void close_file_sg(SGFile_t *sgfile) {
	// reset parameters
	sgfile->index = -1;
	sgfile->next_block_num = -1;
	sgfile->next_block_size = -1;
	// close file
	if (close(sgfile->fd) == -1) {
		fprintf(stderr,
		  "%s.%s(%d): unable to close '%s'\n",
		  __FILE__,__FUNCTION__,__LINE__,
		  sgfile->filename);
	}
	// free filename memory
	if (sgfile->filename != NULL) {
		free(sgfile->filename);
		sgfile->filename = NULL;
	}
}

/* Compare two SGFile_t struct, which can be used for sorting. The
 * comparison is done on the number of the next block in each file.
 * 
 * Returns a value less than, equal to, or greater than zero if a is
 * less than, equal to, or greater than b, respectively, except for the
 * case where end-of-file is reached in which case EoF always returns
 * greater than (or equal to, in case of EoF in both files).
 */
int compare_files_sg(const void *a, const void *b) {
	SGFile_t *sgf_a = (SGFile_t *)a;
	SGFile_t *sgf_b = (SGFile_t *)b;
	//~ fprintf(stderr,"a = %d, b = %d\n",sgf_a->next_block_num,sgf_b->next_block_num);
	if (sgf_a->next_block_num == -1) {
		if (sgf_b->next_block_num == -1) {
			return 0;
		} else {
			return 1;
		}
	} else if (sgf_b->next_block_num == -1) {
		return -1;
	} else {
		return sgf_a->next_block_num - sgf_b->next_block_num;
	}
}

/* Read the entire next block of data from the file, and store it in
 * the data buffer in the referenced SGBlock_t struct. Buffer memory is
 * allocated and should be freed by a call to destroy_block_sg afterward.
 * 
 * Returns 1 on success, and -1 on failure.
 */
int read_block_from_file_sg(SGFile_t *sgfile, SGBlock_t *sgblock) {
	size_t len;
	sgb_header_t sgb_hdr;
	
	if (sgfile->next_block_size > 0) {
		// read block data
		sgblock->block_num = sgfile->next_block_num;
		sgblock->packet_size = sgfile->header.packet_size;
		sgblock->packet_count = sgfile->next_block_size / sgblock->packet_size;
		sgblock->data = malloc(sgfile->next_block_size);
		ioutils_read(sgfile->fd,(void *)sgblock->data,sgfile->next_block_size-sizeof(sgb_header_t),&len);
		// get next block number and update sgfile
		if (ioutils_read(sgfile->fd,(void *)&sgb_hdr,sizeof(sgb_header_t),&len) == -1) {
			fprintf(stderr,
			  "%s.%s(%d): error reading next block in '%s'\n",
			  __FILE__,__FUNCTION__,__LINE__,
			  sgfile->filename);
			return -1;
		}
		//~ fprintf(stdout,"len = %d\n",(int)len);
		if (len == 0) {
			sgfile->next_block_num = -1;
			sgfile->next_block_size = -1;
		} else {
			sgfile->next_block_num = sgb_hdr.block_num;
			sgfile->next_block_size = sgb_hdr.block_size;
			sgblock->index = sgfile->index;
			sgfile->index++;
		}
	} else {
		sgblock->block_num = -1;
		sgblock->packet_size = -1;
		sgblock->packet_count = 0;
		sgblock->data = NULL;
	}
	return 1;
}

/* Read the entire next block of data from the group, and store it in
 * the data buffer in the referenced SGBlock_t struct. Buffer memory is
 * allocated and should be freed by a call to destroy_block_sg afterward.
 * 
 * Returns 1 on success, and -1 on failure.
 */
int read_block_from_group_sg(SGGroup_t *sggroup, SGBlock_t *sgblock) {
	// sort files according to block number
	qsort(sggroup->files,sggroup->file_count,sizeof(SGFile_t),compare_files_sg);
	// read block from first file (after sort)
	if (read_block_from_file_sg(&sggroup->files[0],sgblock) == -1) {
		fprintf(stderr,
		  "%s.%s(%d): unable to read next block from '%s'\n",
		  __FILE__,__FUNCTION__,__LINE__,
		  sggroup->files[0].filename);
		return -1;
	}
	
	return 1;
}

/* Call this method when block data is no longer needed. All parameters
 * are reset and the data buffer is freed.
 */
void destroy_block_sg(SGBlock_t *sgblock) {
	sgblock->block_num = -1;
	sgblock->index = -1;
	sgblock->packet_size = -1;
	sgblock->packet_count = -1;
	if (sgblock->data != NULL) {
		free(sgblock->data);
		sgblock->data = NULL;
	}
}

/* Print string representation of SGFile_t struct to stdout, with the
 * given lead string at the start of each line.
 */
void print_file_sg(const char *ldr, const SGFile_t *sgfile) {
	fprintf(stdout,
	  "%s{filename: '%s', fd: %d, index: %d, next_block_num: %d, next_block_size: %d}\n",
	  ldr,sgfile->filename,sgfile->fd,sgfile->index,sgfile->next_block_num,sgfile->next_block_size);
}

/* Print string representation of SGBlock_t struct to stdout, with the
 * given lead string at the start of each line.
 */
void print_block_sg(const char *ldr, const SGBlock_t *sgblock) {
	fprintf(stdout,
	  "%s{block_number: %d, index: %d, packet_size: %d, packet_count: %d}\n",
	  ldr,sgblock->block_num,sgblock->index,sgblock->packet_size,sgblock->packet_count);
}

/////////////////////////////////////////////////// SCATTER-GATHER FILES

int open_group_sg(int num_files, const char *filenames[], SGGroup_t *sggroup) {
	int ii;
	
	// initialize sub-block parameters
	sggroup->subblock_packet_count = 0;
	sggroup->subblock_data = NULL;
	// initialize the array of SGFile_t objects
	sggroup->files = (SGFile_t *)malloc(num_files*sizeof(SGFile_t));
	for (ii=0; ii<num_files; ii++) {
		if (open_file_sg(filenames[ii],&sggroup->files[ii]) == -1) {
			fprintf(stderr,
			  "%s.%s(%d): failed to initialize scatter-gather file '%s'\n",
			  __FILE__,__FUNCTION__,__LINE__,
			  filenames[ii]);
			return -1;
		}
		// set packet size...
		if (ii==0) {
			sggroup->packet_size = sggroup->files[0].header.packet_size;
		} else {
			// ...or compare to packet size
			if (sggroup->files[ii].header.packet_size != sggroup->packet_size) {
				fprintf(stderr,
				"%s.%s(%d): scatter-gather file '%s' packet size %d does not match SGGroup_t packet size %d\n",
				__FILE__,__FUNCTION__,__LINE__,
				filenames[ii],sggroup->files[ii].header.packet_size,sggroup->packet_size);
				close_file_sg(&sggroup->files[ii]);
				return -1;
			}
		}
	}
	// set the number of files in group
	sggroup->file_count = num_files;
	return 1;
}

void close_group_sg(SGGroup_t *sggroup) {
	int ii;
	
	// destroy files
	for (ii=0; ii<sggroup->file_count; ii++) {
		close_file_sg(&sggroup->files[ii]);
	}
	// free local memory
	if (sggroup->files != NULL) {
		free(sggroup->files);
		sggroup->files = NULL;
	}
	if (sggroup->subblock_data != NULL) {
		free(sggroup->subblock_data);
		sggroup->subblock_data = NULL;
	}
	// reset parameters
	sggroup->file_count = -1;
	sggroup->packet_size = -1;
	sggroup->subblock_packet_count = -1;
}

int read_packets_from_group_sg(SGGroup_t *sggroup, int num_packets, void **buf) {
	int packet_size;
	int read_packets;
	int subblock_read;
	int subblock_save;
	SGBlock_t block;
	
	read_packets = 0;
	packet_size = sggroup->packet_size;
	*buf = malloc(num_packets*packet_size);
	// first copy data from sub-block buffer
	if (sggroup->subblock_packet_count > 0) {
		if (num_packets >= sggroup->subblock_packet_count) {
			// copy all the sub-block data
			memcpy(*buf,sggroup->subblock_data,
			  sggroup->subblock_packet_count*packet_size);
			read_packets += sggroup->subblock_packet_count;
			// clear sub-block buffer
			free(sggroup->subblock_data);
			sggroup->subblock_data = NULL;
			sggroup->subblock_packet_count = 0;
		} else {
			// copy only part of the data
			memcpy(*buf,sggroup->subblock_data,num_packets*packet_size);
			read_packets += num_packets;
			// shift data in sub-block data, and reduce packet count
			memmove(sggroup->subblock_data,sggroup->subblock_data+num_packets*packet_size,
			  (sggroup->subblock_packet_count-num_packets)*packet_size);
			sggroup->subblock_packet_count = sggroup->subblock_packet_count - num_packets;
		}
	}
	while (read_packets < num_packets) {
		if (!read_block_from_group_sg(sggroup,&block)) {
			fprintf(stderr,
			  "%s.%s(%d): failed to read next block\n",
			  __FILE__,__FUNCTION__,__LINE__);
			return -1;
		}
		if (block.packet_count == 0) {
			break;
		}
		if (read_packets + block.packet_count > num_packets) {
			// count packets to read / save
			subblock_read = (num_packets-read_packets);
			subblock_save = block.packet_count-subblock_read;
			// copy "read" packets
			memcpy(*buf+read_packets*packet_size,block.data,
			  subblock_read*packet_size);
			read_packets += subblock_read;
			// copy "save" packets
			sggroup->subblock_packet_count = subblock_save;
			sggroup->subblock_data = malloc(subblock_save*packet_size);
			memcpy(sggroup->subblock_data,block.data+(subblock_read)*packet_size,
			  subblock_save*sggroup->packet_size);
		} else {
			// copy all packet from block
			memcpy(*buf+read_packets*packet_size,block.data,
			  block.packet_count*packet_size);
			read_packets += block.packet_count;
		}
		destroy_block_sg(&block);
	}
	return read_packets;
}

void print_group_sg(const char *ldr, const SGGroup_t *sggroup) {
	int ii;
	SGFile_t *sgfile;
	
	for (ii=0; ii<sggroup->file_count; ii++) {
		sgfile = &sggroup->files[ii];
		print_file_sg(ldr, sgfile);
	}
}

///////////////////////////////////////////////////////////// FLAT FILES

int open_file_f(const char *filename, FFile_t *ffile) {
	size_t len;
	vdif_header_t hdr;
	
	// initialize filename
	len = strlen(filename) + 1; // plus one for trailing '\0'
	ffile->filename = (char *)malloc(len);
	strcpy(ffile->filename,filename);
	// open file and store descriptor
	ffile->fd = open(filename,O_RDONLY);
	if (ffile->fd == -1) {
		fprintf(stderr,
		  "%s.%s(%d): unable to open '%s'\n",
		  __FILE__,__FUNCTION__,__LINE__,
		  filename);
		return -1;
	}
	// initialize raw header and check if valid
	if (ioutils_read(ffile->fd,(void *)&hdr,sizeof(vdif_header_t),&len) <= 0) {
		fprintf(stderr,
		  "%s.%s(%d): failed to read vdif header bytes from '%s'\n",
		  __FILE__,__FUNCTION__,__LINE__,
		  filename);
		return -1;
	}
	// reset file offset to start of file
	if (lseek(ffile->fd, 0, SEEK_SET) == -1) {
		perror("vdif_files.c.open_file_f(): ");
	}
	if (hdr.invalid_data) {
		fprintf(stderr,
		  "%s.%s(%d): invalid packet at start of flat file '%s'\n",
		  __FILE__,__FUNCTION__,__LINE__,
		  filename);
		return -1;
	}
	// reset index
	ffile->index = 0;
	// set packet size
	ffile->packet_size = hdr.frame_length*8; // given in 8-byte words
	return 1;
}

void close_file_f(FFile_t *ffile) {
	// reset parameters
	ffile->index = -1;
	ffile->packet_size = -1;
	// close file
	if (close(ffile->fd) == -1) {
		fprintf(stderr,
		  "%s.%s(%d): unable to close '%s'\n",
		  __FILE__,__FUNCTION__,__LINE__,
		  ffile->filename);
	}
	// free filename memory
	if (ffile->filename != NULL) {
		free(ffile->filename);
		ffile->filename = NULL;
	}
}

int read_packets_from_file_f(FFile_t *ffile, int num_packets, void **buf) {
	int packet_size;
	size_t len;
	
	packet_size = ffile->packet_size;
	*buf = malloc(num_packets*packet_size);
	if (ioutils_read(ffile->fd,*buf,num_packets*packet_size,&len) == -1) {
		fprintf(stderr,
		  "%s.%s(%d): error reading packets from '%s'\n",
		  __FILE__,__FUNCTION__,__LINE__,
		  ffile->filename);
		return -1;
	};
	return len/packet_size;
}
