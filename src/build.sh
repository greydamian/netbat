#! /usr/bin/env bash

CC="cc"; # c compiler command (can be hard-coded to "gcc" or "clang")

SOURCE="netbat.c";
OUTPUT="../bin/netbat";

# compiler & linker flags for gcc
GCC_CFLAGS="";
GCC_LDFLAGS="-pthread";

# compiler & linker flags for clang
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

