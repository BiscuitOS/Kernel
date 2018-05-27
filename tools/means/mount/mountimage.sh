#!/bin/bash
#
# Mount special Image.
#

# $1 Image Name

if [ ! -d .tmp ]; then
  mkdir -p .tmp
fi

if [ $2 == "mount" ]; then
  losetup -o 1048576 /dev/loop3 $1
  mount /dev/loop4 .tmp
fi

if [ $2 == "umount" ]; then
  umount .tmp
  losetup -d /dev/loop4
  rm -rf .tmp
fi
