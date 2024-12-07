rm -rf tarfs
mkdir tarfs

echo "[1;34m===== basic case manually test =====[m"
cp tar/basic.tar test.tar
g++ 111550093.cpp -o 111550093.out `pkg-config fuse --cflags --libs`
./111550093.out -f tarfs
rm -f test.tar
echo "[1;34m===== basic case manually test done =====[m"

echo "[1;35m===== softlink case manually test =====[m"
cp tar/softlink.tar test.tar
./111550093.out -f tarfs
rm -f test.tar
echo "[1;35m===== softlink case manually test done =====[m"
