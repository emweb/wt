# Build fuzzers

## ASAN

```bash
export CC=clang
export CXX=clang++
export CFLAGS="-O1 -fno-omit-frame-pointer -gline-tables-only -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=fuzzer-no-link"
export CXXFLAGS="-O1 -fno-omit-frame-pointer -gline-tables-only -DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION -fsanitize=address -fsanitize-address-use-after-scope -fsanitize=fuzzer-no-link"
export LIB_FUZZING_ENGINE=-fsanitize=fuzzer
```

## Compile

```bash
apt-get install -y libboost-all-dev cmake

mkdir -p mybuild && cd mybuild/
cmake -DSHARED_LIBS=OFF -DBUILD_FUZZ=ON -DBoost_USE_STATIC_LIBS=ON ../.
```

## Run

```bash
./fuzz-{target} fuzz-{target}_seed_corpus/
```
