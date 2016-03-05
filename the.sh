set -x
cd "`dirname "$0"`"

rm -rf .obj
mkdir -p .obj/o || exit 1
for i in src/*.c; do
  gcc -Wall -g -c -o .obj/o/`basename $i .c`.o -I h/jfdt $i || exit 1
done

ar rc .obj/jfdt.a .obj/o/*.o || exit 1

gcc -Wall -g -o .obj/tcproxy -I h examples/simple/tcproxy.c .obj/jfdt.a
