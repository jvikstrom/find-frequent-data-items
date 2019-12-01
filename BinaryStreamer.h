#pragma once
#include "RevisionStreamer.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <iterator>
#include <random>
#include <algorithm>

// BinaryStreamer fakes a streaming of the file.
template<class T>
class BinaryStreamer {
  std::ifstream ifs;
protected:
  std::vector<Revision> block;
public:
  BinaryStreamer(const std::string &path) : ifs(path, std::ios::binary | std::ios::in) {
    if(!ifs.is_open()) {
      throw CouldNotOpenFile();
    }
    while(!ifs.eof()) {
      Revision rev;
      ifs.read((char*)&rev, sizeof(rev));
      block.push_back(rev);
    }
  }
  void startStream() {
    static_cast<T*>(this)->onStart();

    for(Revision rev : block) {
      static_cast<T*>(this)->onRevision(rev);
    }
    static_cast<T*>(this)->onDone();
  }
  void onRevision(Revision rev) {}
  void onDone() {}
  void onStart() {}
};

// This shuffles all the revisions in the file.
template<class T>
class RandomBinaryStreamer : BinaryStreamer<T> {
public:
  RandomBinaryStreamer(const std::string &path, unsigned seed) : BinaryStreamer<T>(path) {
    // The blocks are loaded by BinaryStreamer's constructor so simply shuffle them here.
    std::shuffle(this->block.begin(), this->block.end(), std::default_random_engine(seed));
  }
};
