READ
-----------------------------------------------------

  Read data from file etc.

  ```
    int read(int fildes, char *buf, off_t count);
  ```

### DESCRIPTION

  The read() function shall attempt to read nbyte bytes from the file 
  associated with the open file descriptor, fildes, into the buffer pointed 
  to by buf. The behavior of multiple concurrent reads on the same pipe, 
  FIFO, or terminal device is unspecified.

  Before any action described below is taken, and if nbyte is zero, the 
  read() function may detect and return errors as described below. In the 
  absence of errors, or if error detection is not performed, the read() 
  function shall return zero and have no other results.

  On files that support seeking (for example, a regular file), the read() 
  shall start at a position in the file given by the file offset associated 
  with fildes. The file offset shall be incremented by the number of bytes 
  actually read.

  Files that do not support seeking-for example, terminals-always read from
  the current position. The value of a file offset associated with such a 
  file is undefined.

  No data transfer shall occur past the current end-of-file. If the starting 
  position is at or after the end-of-file, 0 shall be returned. If the file 
  refers to a device special file, the result of subsequent read() requests 
  is implementation-defined.

  If the value of nbyte is greater than {SSIZE_MAX}, the result is 
  implementation-defined.

  When attempting to read from an empty pipe or FIFO:

    * If no process has the pipe open for writing, read() shall return 0 to 
      indicate end-of-file.

    * If some process has the pipe open for writing and O_NONBLOCK is set, 
      read() shall return -1 and set errno to [EAGAIN].

    * If some process has the pipe open for writing and O_NONBLOCK is clear, 
      read() shall block the calling thread until some data is written or 
      the pipe is closed by all processes that had the pipe open for writing.

  When attempting to read a file (other than a pipe or FIFO) that supports 
  non-blocking reads and has no data currently available:

    * If O_NONBLOCK is set, read() shall return -1 and set errno to [EAGAIN].

    * If O_NONBLOCK is clear, read() shall block the calling thread until 
      some data becomes available.

  The use of the O_NONBLOCK flag has no effect if there is some data available.

  The read() function reads data previously written to a file. If any portion
  of a regular file prior to the end-of-file has not been written, read() 
  shall return bytes with value 0. For example, lseek() allows the file offset
  to be set beyond the end of existing data in the file. If data is later 
  written at this point, subsequent reads in the gap between the previous 
  end of data and the newly written data shall return bytes with value 0 
  until data is written into the gap.

  Upon successful completion, where nbyte is greater than 0, read() shall 
  mark for update the st_atime field of the file, and shall return the number
  of bytes read. This number shall never be greater than nbyte. The value 
  returned may be less than nbyte if the number of bytes left in the file is 
  less than nbyte, if the read() request was interrupted by a signal, or if 
  the file is a pipe or FIFO or special file and has fewer than nbyte bytes i
  immediately available for reading. For example, a read() from a file 
  associated with a terminal may return one typed line of data.

  If a read() is interrupted by a signal before it reads any data, it shall 
  return -1 with errno set to [EINTR].

  If a read() is interrupted by a signal after it has successfully read some
  data, it shall return the number of bytes read.

  For regular files, no data transfer shall occur past the offset maximum 
  established in the open file description associated with fildes.

  If fildes refers to a socket, read() shall be equivalent to recv() with no 
  flags set.

  If the O_DSYNC and O_RSYNC bits have been set, read I/O operations on the
  file descriptor shall complete as defined by synchronized I/O data integrity
  completion. If the O_SYNC and O_RSYNC bits have been set, read I/O 
  operations on the file descriptor shall complete as defined by synchronized 
  I/O file integrity completion.

  If fildes refers to a shared memory object, the result of the read() 
  function is unspecified.

  If fildes refers to a typed memory object, the result of the read() 
  function is unspecified.

  A read() from a STREAMS file can read data in three different modes: 
  byte-stream mode, message-nondiscard mode, and message-discard mode. The 
  default shall be byte-stream mode. This can be changed using the I_SRDOPT 
  ioctl() request, and can be tested with I_GRDOPT ioctl(). In byte-stream 
  mode, read() shall retrieve data from the STREAM until as many bytes as 
  were requested are transferred, or until there is no more data to be 
  retrieved. Byte-stream mode ignores message boundaries.

  In STREAMS message-nondiscard mode, read() shall retrieve data until as 
  many bytes as were requested are transferred, or until a message boundary 
  is reached. If read() does not retrieve all the data in a message, the 
  remaining data shall be left on the STREAM, and can be retrieved by the 
  next read() call. Message-discard mode also retrieves data until as many
  bytes as were requested are transferred, or a message boundary is reached.
  However, unread data remaining in a message after the read() returns shall 
  be discarded, and shall not be available for a subsequent read(), getmsg(), 
  or getpmsg() call.

  How read() handles zero-byte STREAMS messages is determined by the current
  read mode setting. In byte-stream mode, read() shall accept data until it 
  has read nbyte bytes, or until there is no more data to read, or until a 
  zero-byte message block is encountered. The read() function shall then 
  return the number of bytes read, and place the zero-byte message back on 
  the STREAM to be retrieved by the next read(), getmsg(), or getpmsg(). In 
  message-nondiscard mode or message-discard mode, a zero-byte message shall 
  return 0 and the message shall be removed from the STREAM. When a zero-byte
  message is read as the first message on a STREAM, the message shall be 
  removed from the STREAM and 0 shall be returned, regardless of the read mode.

  A read() from a STREAMS file shall return the data in the message at the 
  front of the STREAM head read queue, regardless of the priority band of the 
  message.

  By default, STREAMs are in control-normal mode, in which a read() from a 
  STREAMS file can only process messages that contain a data part but do not
  contain a control part. The read() shall fail if a message containing a 
  control part is encountered at the STREAM head. This default action can be 
  changed by placing the STREAM in either control-data mode or control-discard
  mode with the I_SRDOPT ioctl() command. In control-data mode, read() shall 
  convert any control part to data and pass it to the application before 
  passing any data part originally present in the same message. In 
  control-discard mode, read() shall discard message control parts but return
  to the process any data part in the message.

  In addition, read() shall fail if the STREAM head had processed an 
  asynchronous error before the call. In this case, the value of errno shall
  not reflect the result of read(), but reflect the prior error. If a hangup
  occurs on the STREAM being read, read() shall continue to operate normally
  until the STREAM head read queue is empty. Thereafter, it shall return 0.

  The pread() function shall be equivalent to read(), except that it shall 
  read from a given position in the file without changing the file pointer.
  The first three arguments to pread() are the same as read() with the 
  addition of a fourth argument offset for the desired position inside the 
  file. An attempt to perform a pread() on a file that is incapable of seeking
  shall result in an error.

### Return value

  Upon successful completion, read() shall return a non-negative integer 
  indicating the number of bytes actually read. Otherwise, the functions 
  shall return -1 and set errno to indicate the error.

### ERRORS

  The read() and pread() functions shall fail if:

  * EAGAIN

    The O_NONBLOCK flag is set for the file descriptor and the thread would 
    be delayed.

  * EBADF

    The fildes argument is not a valid file descriptor open for reading.

  * EBADMSG

    The file is a STREAM file that is set to control-normal mode and the 
    message waiting to be read includes a control part.

  * EINTR

    The read operation was terminated due to the receipt of a signal, and 
    no data was transferred.

  * EINVAL

    The STREAM or multiplexer referenced by fildes is linked (directly or 
    indirectly) downstream from a multiplexer.

  * EIO

    The process is a member of a background process attempting to read from
    its controlling terminal, the process is ignoring or blocking the SIGTTIN
    signal, or the process group is orphaned. This error may also be generated
    for implementation-defined reasons.

  * EISDIR

    The fildes argument refers to a directory and the implementation does not
    allow the directory to be read using read() or pread(). The readdir() 
    function should be used instead.

  * EOVERFLOW

    The file is a regular file, nbyte is greater than 0, the starting 
    position is before the end-of-file, and the starting position is greater
    than or equal to the offset maximum established in the open file 
    description associated with fildes.

  The read() function shall fail if:

  * EAGAIN or EWOULDBLOCK

    The file descriptor is for a socket, is marked O_NONBLOCK, and no data 
    is waiting to be received.

  * ECONNRESET

    A read was attempted on a socket and the connection was forcibly closed 
    by its peer.

  * ENOTCONN

    A read was attempted on a socket that is not connected.

  * ETIMEDOUT

    A read was attempted on a socket and a transmission timeout occurred.

  The read() and pread() functions may fail if:

  * EIO

    A physical I/O error has occurred.

  * ENOBUFS

    Insufficient resources were available in the system to perform the 
    operation.

  * ENOMEM

    Insufficient memory was available to fulfill the request.

  * ENXIO

    A request was made of a nonexistent device, or the request was outside 
    the capabilities of the device.

  The pread() function shall fail, and the file pointer shall remain 
  unchanged, if:

  * EINVAL

    The offset argument is invalid. The value is negative.

  * EOVERFLOW

    The file is a regular file and an attempt was made to read at or beyond
    the offset maximum associated with the file.

  * ENXIO

    A request was outside the capabilities of the device.

  * ESPIPE

    fildes is associated with a pipe or FIFO.

### External link

  [opengroup](http://pubs.opengroup.org/onlinepubs/009695399/functions/close.html "read - opengroup.org")  
