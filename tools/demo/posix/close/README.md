Close
--------------------------------------------------

  For most file systems, a program terminates access to a file in a filesystem
  using the close system call. This flushes buffers, updates file metadata (
  which may include and end of file indicator in the data), de-allocates 
  resources associated with the file (including the file descriptor) and 
  updates the system wide table of files in use. Some languages maintain a 
  structure of files opened by its run-time library and may close when the 
  program terminates. Some operating systems will invoke the close if the 
  program terminates. Some operating systems will invoke the close as part 
  of an operating system recovery as a result of a system failure.


### DESCRIPTION

  The close() function shall deallocate the file descriptor indicated by 
  fildes. To deallocate means to make the file descriptor available for return 
  by subsequent calls to open() or other functions that allocate file 
  descriptors. All outstanding record locks owned by the process on the file 
  associated with the file descriptor shall be removed (that is, unlocked).

  If close() is interrupted by a signal that is to be caught, it shall return 
  -1 with errno set to [EINTR] and the state of fildes is unspecified. If an 
  I/O error occurred while reading from or writing to the file system during 
  close(), it may return -1 with errno set to [EIO]; if this error is returned,
  the state of fildes is unspecified.

  When all file descriptors associated with a pipe or FIFO special file are
  closed, any data remaining in the pipe or FIFO shall be discarded.

  When all file descriptors associated with an open file description have 
  been closed, the open file description shall be freed.

  If the link count of the file is 0, when all file descriptors associated
  with the file are closed, the space occupied by the file shall be freed 
  and the file shall no longer be accessible.

  If a STREAMS-based fildes is closed and the calling process was previously
  registered to receive a SIGPOLL signal for events associated with that 
  STREAM, the calling process shall be unregistered for events associated 
  with the STREAM. The last close() for a STREAM shall cause the STREAM 
  associated with fildes to be dismantled. If O_NONBLOCK is not set and 
  there have been no signals posted for the STREAM, and if there is data 
  on the module's write queue, close() shall wait for an unspecified 
  time (for each module and driver) for any output to drain before dismantling
  the STREAM. The time delay can be changed via an I_SETCLTIME ioctl() request.
  If the O_NONBLOCK flag is set, or if there are any pending signals, close() 
  shall not wait for output to drain, and shall dismantle the STREAM 
  immediately.

  If the implementation supports STREAMS-based pipes, and fildes is associated 
  with one end of a pipe, the last close() shall cause a hangup to occur on 
  the other end of the pipe. In addition, if the other end of the pipe has 
  been named by fattach(), then the last close() shall force the named end to 
  be detached by fdetach(). If the named end has no open file descriptors 
  associated with it and gets detached, the STREAM associated with that end 
  shall also be dismantled.

  If fildes refers to the master side of a pseudo-terminal, and this is the 
  last close, a SIGHUP signal shall be sent to the controlling process, if 
  any, for which the slave side of the pseudo-terminal is the controlling 
  terminal. It is unspecified whether closing the master side of the 
  pseudo-terminal flushes all queued input and output.

  If fildes refers to the slave side of a STREAMS-based pseudo-terminal, a 
  zero-length message may be sent to the master. 

  When there is an outstanding cancelable asynchronous I/O operation against 
  fildes when close() is called, that I/O operation may be canceled. An I/O 
  operation that is not canceled completes as if the close() operation had 
  not yet occurred. All operations that are not canceled shall complete as 
  if the close() blocked until the operations completed. The close() operation
  itself need not block awaiting such I/O completion. Whether any I/O 
  operation is canceled, and which I/O operation may be canceled upon close(),
  is implementation-defined.

  If a shared memory object or a memory mapped file remains referenced at 
  the last close (that is, a process has it mapped), then the entire contents
  of the memory object shall persist until the memory object becomes 
  unreferenced. If this is the last close of a shared memory object or a 
  memory mapped file and the close results in the memory object becoming 
  unreferenced, and the memory object has been unlinked, then the memory 
  object shall be removed.

  If fildes refers to a socket, close() shall cause the socket to be destroyed.
  If the socket is in connection-mode, and the SO_LINGER option is set for the
  socket with non-zero linger time, and the socket has untransmitted data, 
  then close() shall block for up to the current linger interval until all 
  data is transmitted.


  The close call is standardized by the POSIX specification

  ```
    int close (int filedes);
  ```

  The function returns zero to indicate the file was closed successfully. 
  If any error occurs, a value of -1 is returned and errno is appropriately 
  set.

  The errors that can occur include:

  ```
    EBADF    The argument supplied was not a valid file descriptor
    EINTR    The function call was interrupted by a signal
    EIO      An I/O error occurred
  ```

### External Link

  [close() - opengroup](http://pubs.opengroup.org/onlinepubs/009695399/functions/close.html "close - opengroup.org")
