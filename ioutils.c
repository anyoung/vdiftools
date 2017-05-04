#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ioutils.h"

int ioutils_read(int fd, void *buf, size_t count, size_t *bytes_read) {
	size_t bytes;
	
	*bytes_read = 0;
	while (*bytes_read < count) {
		bytes = (size_t)read(fd,buf+*bytes_read,count-*bytes_read);
		if (bytes == 0) {
			//~ fprintf(stderr,
			  //~ "%s.%s(%d): End-of-file reached\n",
			  //~ __FILE__,__FUNCTION__,__LINE__);
			return 0;
		} else if (bytes < 0) {
			perror("ioutils_read");
			return -1;
		}
		*bytes_read += bytes;
	}
	return 1;
}
