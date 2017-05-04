#ifndef VDIF_FILES_H
#define VDIF_FILES_H

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

/////////////////////////////////////////////////// SCATTER-GATHER FILES
/* Defines scatter-gather file header. The #define statements give the
 * expected values for the scatter-gather file format that can be handled
 * in this toolset.
 */
#define SGF_HEADER_SYNC_WORD 0xFEED6666
#define SGF_HEADER_VERSION 2
#define SGF_HEADER_PACKET_FORMAT 0
typedef struct sgf_header {
	// should match #define above
	unsigned int sync_word;
	// should match #define above
	int version;
	// "default" block size
	int block_size;
	// should match #define above
	int packet_format;
	// size of VDIF packets
	int packet_size;
} sgf_header_t;

/* Defines the block header within a scatter-gather file.
 */
typedef struct sgb_header {
	// increments across all blocks written simultaneously (even across
	// multiple input_stream
	int block_num;
	// includes this header and all underlying VDIF packets
	int block_size;
} sgb_header_t;

/* Encapsulates a scatter-gather block read from file
 */
typedef struct SGBlock {
	// raw block number
	int block_num;
	// index in stream
	int index;
	// packet size in bytes
	int packet_size;
	// number of packets in block
	int packet_count;
	// buffer filled with block data
	void *data;
} SGBlock_t;

/* Encapsulates a scatter-gather file, keeps bookkeeping information on
 * status of read data.
 */
typedef struct SGFile {
	// raw file header
	sgf_header_t header;
	// filename
	char *filename;
	// file descriptor
	int fd;
	// counts number of blocks already read
	int index;
	// block_num of the next block
	int next_block_num;
	// block_size of next block
	int next_block_size;
} SGFile_t;

/* Encapsulates a scatter-gather group of files, keeps collection of
 * SGFile_t structs and bookkeeping information on status of read data.
 */
typedef struct SGGroup {
	// list of files that comprise the group
	SGFile_t *files;
	// number of files in group
	int file_count;
	// packet size
	int packet_size;
	// number of sub-block packets
	int subblock_packet_count;
	// sub-block packet data
	void *subblock_data;
} SGGroup_t;

/* Open a group of scatter-gather files. The referenced SGGroup_t struct
 * is initialized and should be used in subsequent calls to *_group_sg
 * functions.
 * 
 * Returns 1 on success and -1 on failure.
 */
int open_group_sg(int num_files, const char *filenames[], SGGroup_t *sggroup);

/* Close a group of scatter-gather files. All dynamically allocated
 * memory associated with the SGGroup_t struct is freed.
 */
void close_group_sg(SGGroup_t *sggroup);

/* Read a number of packets from scatter-gather group and store them in
 * a buffer. Buffer memory is allocated dynamically and should be freed
 * manually after use. The scatter-gather group should be open, and
 * block indecies are advanced by the read.
 * 
 * Returns number of packets read (can be less than requested number of
 * packets if end-of-file reached), 0 when no more packets could be
 * read, and -1 when an error occurs.
 * 
 * Data is guaranteed to be in scatter-gather block order, but packets
 * may be out of order.
 */
int read_packets_from_group_sg(SGGroup_t *sggroup, int num_packets, void **buf);

/* Print string representation of scatter-gather group to stdout.
 */
void print_group_sg(const char *ldr, const SGGroup_t *sggroup);

///////////////////////////////////////////////////////////// FLAT FILES
/* Encapsulates a flat VDIF file.
 */
typedef struct FFile {
	// filename
	char *filename;
	// file descriptor
	int fd;
	// counts number of packets already read
	int index;
	// byte size of packets
	int packet_size;
} FFile_t;

/* Open a flat file. The referenced FFile_t struct is initialized and
 * should be used in subsequent calls to *_file_f functions.
 * 
 * Returns 1 on success and -1 on failure.
 */
int open_file_f(const char *filename, FFile_t *ffile);

/* Close a flat file. All dynamically allocated memory associated with
 * the FFile_t struct is freed.
 */
void close_file_f(FFile_t *ffile);

/* Read number of packets from flat file and store store them in a
 * buffer. Buffer memory is allocated dynamically and should be freed
 * manually after use. The flat file should be open, and its index is
 * advanced by the read.
 * 
 * Returns number of packets read (can be less than requested number of
 * packets if end-of-file reached), 0 when no more packets could be
 * read, and -1 when an error occurs.
 * 
 * Data is guaranteed to be in order read from file, but packets may
 * be out of order.
 */
int read_packets_from_file_f(FFile_t *ffile, int num_packets, void **buf);

#endif // VDIF_FILES_H
