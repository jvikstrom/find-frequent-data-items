This contains an implementation of the algorithm descirbed by Charikar, Moses and Chen, Kevin and Farach-Colton, Martin in Finding frequent items in data streams. It takes data as input and prints the estiamted occurences for the k most frequently occuring elements.

# Build

```
mkdir build && cd build && ../gen_cmake.sh && ninja
```

This will create three executables, "bstreamer", "hashbstreamer" and "zipfbstreamer".
## Running

"zipfbstreamer" generates data in a zipf distribution with z=1.

"bstreamer" expects the Stanford SNAP bz2 file in "../enwiki-20080103.main.bz2" relative to the source directory. It will unzip every revision's (article id, revision id) to "out.data" in the build directory. This takes a very long time as there is a lot of data to unzip and it's done completely sequentially.

"hashbstreamer" expects the unzipped contents in "build/out.data". It will then run the naive hash-map algorithm and the approximate algorithm, output some stats and that's it.

## Data
Data that has been pre-unzipped and is in the correct format can be found [here](https://drive.google.com/open?id=1HTetf3tjaXv5RbKdZJRlvsFQPJ_76zuJ).
