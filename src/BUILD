echo "Complie defs.c to defs.o"
gcc -Wall -fpic -c defs.c -o defs.o
echo "Create so from defs.o to libtracer.so"
gcc -shared defs.o -o libtracer.so
sudo ln -sf `pwd`/libtracer.so /opt/lib/libtracehitter.so
echo "Compile main"
gcc -Wall -L/opt/lib -ltracehitter main.c -o main
