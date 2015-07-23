# FSB Vorbis Extractor

Extracts Vorbis samples from FMOD Soundbank files version 5 (FSB5).

## Building and running

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make

./src/extractor container.fsb --destination existing_directory
```

## Dependencies

On Ubuntu required dependencies can be installed with command:

```
sudo aptitude install \
  cmake \
  g++ \
  libboost-dev \
  libboost-filesystem-dev \
  libboost-iostreams-dev \
  libboost-system-dev \
  libgflags-dev \
  libgoogle-glog-dev \
  libgtest-dev \
  libogg-dev \
  libvorbis-dev \
  make \

```

## License

GPL3, see LICENSE file for details.
