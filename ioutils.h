#ifndef IOUTILS_H
#define IOUTILS_H

/* Read count bytes from file described by fd into buf.
 * Arguments:
 *  fd -- descriptor for file opened in read-mode
 *  buf -- pointer to memory location where data should be stored
 *  count -- number of bytes to read from file
 *  read -- pointer to memory where number of read bytes should be stored
 * Returns:
 *  rv -- 1 on success, 0 on end-of-file reached, -1 on error
 * Notes:
 *  
 */
int ioutils_read(int fd, void *buf, size_t count, size_t *read);

#endif // IOUTILS_H
