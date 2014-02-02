#! /usr/bin/env bash

gcc_path="$(which gcc)";
clang_path="$(which clang)";

mkdir -p ../bin;

if [ "$clang_path" != "" ];
then
    clang -o ../bin/netbat ./netbat.c;
    exit;
fi

if [ "$gcc_path" != "" ];
then
    gcc -o ../bin/netbat ./netbat.c -pthread;
    exit;
fi

echo "build.sh: error: failed to detect compatible compiler";

