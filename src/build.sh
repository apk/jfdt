#!/bin/sh

set -x
cd "`dirname "$0"`"

rm -rf .obj
mkdir .obj || exit 1
for i in *.c; do
  gcc -c -o .obj/`basename $i`.o -I ../h/jfdt $i || exit 1
done
exec ar rc .obj/jfdt.a .obj/*.o
