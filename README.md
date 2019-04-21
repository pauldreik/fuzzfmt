This is for fuzzing libfmt.

It has found a bug:
[fmt github #1124](https://github.com/fmtlib/fmt/issues/1124)

Unfortunately one has to limit the maximum memory allocation, otherwise
the fuzzing will soon interrupt after trying to allocate many GB of memory.

With afl, reaches about 3000 iterations per second on a single core.
With libFuzzer, about 200000.

# AFL
Building with afl and undefined behaviour sanitizer:
```sh
mkdir build-afl-ubsan
cd build-afl-ubsan
CXX=afl-g++ CXXFLAGS="-fsanitize=undefined" cmake .. -Dreproduce_mode=on
make
```

corpus minimization:
```sh
afl-cmin  -i ../build-afl/aflout/ -o corpus/ -- ./reproducer_fuzz_two_args @@
```

fuzzing:
```sh
export UBSAN_OPTIONS=abort_on_error=1
afl-fuzz -i corpus -o out -- ./reproducer_fuzz_two_args @@
```

# libFuzzer

```sh
mkdir build-libfuzzer-sanitizers
cd build-libfuzzer-sanitizers/
CXX=clang++ CXXFLAGS="-fsanitize=address,undefined -O3" cmake .. -Dreproduce_mode=off
make
./fuzzer_fuzz_two_args
```

