
# Build

```
mkdir build && cd build && ../gen_cmake.sh && ninja
```

This will create two executables, "bstreamer" and "hashbstreamer".
## Running

"bstreamer" expects the Stanford SNAP bz2 file in "../enwiki-20080103.main.bz2" relative to the source directory. It will unzip every revision's (article id, revision id) to "out.data" in the build directory.

"hashbstreamer" expects the unzipped contents in "build/out.data". It will then run the naive hash-map algorithm and the approximate algorithm, output some stats and that's it.

