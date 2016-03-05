set -x
cd "`dirname "$0"`"

rm -rf .obj
mkdir .obj || exit 1
for i in src/*.c; do
  gcc -Wall -c -o .obj/`basename $i`.o -I h/jfdt $i || exit 1
done

ar rc .obj/jfdt.a .obj/*.o || exit 1

gcc -Wall -o .obj/tcproxy -I h examples/simple/tcproxy.c .obj/jfdt.a
