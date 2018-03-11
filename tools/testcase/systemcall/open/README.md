Open
---------------------------------------------------

  For most file systems, a program initializes access to a file in a file 
  system using the open system call. This allocates resources associated 
  to the file (the file descriptor), and returns a handle that the process 
  will use to refer to that file. In some cases the open is performed by 
  the first access.

  The same file may be opened simultaneously by several processes, and even
  by the same process (resulting in several file descriptors for the same
  file) depending on the file organization and filesystem. Operations on the
  descriptors such as moving the file pointer or closing it are independent
  (they do not affect other descriptors for the same file). Operations on the
  file (like a write) can be seen by operations on the other descriptors (a
  posterior read can read the written data).

  During the open, the filesystem may allocate memory for buffers (or it may
  wait until the first operation).

  The absolute filename is resolved. This may include connecting to a remote
  host and notifying an operator that a removable medium is required. It may
  include the initialization of a communication device. At this point an 
  error may be returned if the host or medium is not available. The first
  access to at least the directory within the filesystem is performed. An 
  error will usually be returned if the higher level components of the path
  (directories) cannot be located or accessed. An error will be returned if 
  the file is expected to exist and it does not or if the file should not 
  already exist and it does.

  If the file is expected to exist and it does, the file access, as restricted
  by permission flags within the file meta data or access control list, is 
  validated against the requested type of operations. This usually requires 
  an additional filesystem access although in some filesystems meta-flags may
  be part of the directory structure.

  If the file is being created, the filesystem may allocate the default 
  initial amount of storage or a specified amount depending on the file system 
  capabilities. If this fails an error will be returned. Updating the directory
  with the new entry may be performed or it may be delayed until the close is 
  performed.

  Various other errors which may occur during the open include directory update
  failures, un-permitted multiple connections, media failures, communication 
  link failures and device failures. The return value must always be examined 
  and an error specific action taken.

  In many cases programming language-specific run-time library opens may 
  perform additional actions including initializing a run-time library 
  structure related to the file.

  As soon as a file is no longer needed, the program should close it. This 
  will cause run-time library and filesystem buffers to be updated to the 
  physical media and permit other processes to access the data if exclusive 
  use had been required. Some run-time libraries may close a file if the 
  program calls the run-time exit. Some filesystems may perform the necessary
  operations if the program terminates. Neither of these is likely to take 
  place in the event of a kernel or power failure. This can cause damaged 
  filesystem structures requiring the running of privileged and lengthy 
  filesystem utilities during which the entire filesystem may be inaccessible.




