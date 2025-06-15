for i in cmake ninja-build clang qt6-base-dev qt6-wayland; do
    apt list $i 2>/dev/null | grep -qw installed || sudo apt install -y $i
done

rm -f CMakeCache.txt
cmake . -G Ninja --fresh -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang
