# FMT Fuzzer
This is for fuzzing libfmt which is proposed for standardization, so it's extra
important that bugs are smoked out.

It has found a bug:
[fmt github #1124](https://github.com/fmtlib/fmt/issues/1124)

Unfortunately one has to limit the maximum memory allocation, otherwise
the fuzzing will soon interrupt after trying to allocate many GB of memory. That is why the submodule
does not point to upstream fmt, but instead to a branch which introduces the nice blocks like:
```cpp
#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
if(spec.precision>100000) {
 throw std::runtime_error("fuzz mode - avoiding large precision");
}
#endif
```
This macro is the defacto standard for making fuzzing practically possible, see [the libFuzzer documentation](https://llvm.org/docs/LibFuzzer.html#fuzzer-friendly-build-mode).


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
afl-cmin  -i lots/of/files/ -o corpus/ -- ./reproducer_fuzz_two_args @@
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
mkdir out
./fuzzer_fuzz_two_args out corpus
```

