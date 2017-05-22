#!/bin/bash
# ----------------------------------------------------------------------
# Coding Style - remove table and blank
# Maintainer: Buddy <buddy.zhang@aliyun.com>
#
# Copyright (C) 2017 BiscuitOS
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

sed 's/[ \t]*$//g' $1 > tmpfile
rm $1 
mv tmpfile $1
