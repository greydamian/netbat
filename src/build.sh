#! /usr/bin/env bash

# Copyright (c) 2014 Damian Jason Lapidge
#
# The contents of this file are subject to the terms and conditions defined 
# within the file LICENSE.txt, located within this project's root directory.

CC="cc"; # c compiler command (can be hard-coded to "gcc" or "clang")

SOURCE="netbat.c";
OUTPUT="../bin/netbat";

# gcc specific compiler & linker flags
GCC_CFLAGS="";
GCC_LDFLAGS="-pthread";

# clang specific compiler & linker flags
CLANG_CFLAGS="";
CLANG_LDFLAGS="";

# create output directory
mkdir -p ../bin;

if [ "$(which $CC)" == "" ];
then
    echo "error: failure to locate C compiler ($CC)" 1>&2;
    exit 1; # exit failure
fi

CC_VERSION="$($CC -v 2>&1)";

if [ "$(echo $CC_VERSION | grep -i 'gcc')" != "" ];
then
    # compile with gcc
    $CC $GCC_CFLAGS -o $OUTPUT $SOURCE $GCC_LDFLAGS;
    exit; # exit success
fi

if [ "$(echo $CC_VERSION | grep -i 'clang')" != "" ];
then
    # compile with clang
    $CC $CLANG_CFLAGS -o $OUTPUT $SOURCE $CLANG_LDFLAGS;
    exit; # exit success
fi

echo "error: failure to recognize a supported compiler" 1>&2;
exit 1; # exit failure

