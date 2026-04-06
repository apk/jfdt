set -x
cd "`dirname "$0"`"

# run-on-change src/*.c h/jfdt/*.h the.sh -- sh the.sh

rm -rf .obj
mkdir -p .obj/o || exit 1
for i in src/*.c; do
  gcc -Wall -g -c -o .obj/o/`basename $i .c`.o -I h/jfdt $i || exit 1
done

ar rc .obj/jfdt.a .obj/o/*.o || exit 1

for i in examples/simple/*.c; do
    b="`basename $i .c`"
    gcc -Wall -g -o .obj/"$b" -I h "$i" .obj/jfdt.a || exit 1
done
