#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include <bzlib.h>
#include <stdio.h>
#include <cmath>
#include "lib/readerwriterqueue.h"
#include "RevisionStreamer.h"


struct Zipf {
  int m, s;
  int n;
  double divisor = 0;
  Zipf(int n, int m, int s) : m(m), n(n), s(s) {
    for(int i = 1; i <= m; ++i) {
      divisor += 1 / std::pow((double)i, (double)s);
    }
  }
  double zipf(int rank) {
    return (1 / std::pow((double)rank, (double)s)) / divisor;
  }
  int freq(int rank) {
    return std::max(zipf(rank) * n, 1.0);
  }
};

int main() {
  std::string outPath = "zipfout.data";
  std::ofstream ofs(outPath.c_str(), std::ios::out | std::ios::binary);
  if(!ofs.is_open()) {
    return -1;
  }

  int unique = 10000000;
  Zipf zipf(500000, unique, 1);
  long long count = 0;
  std::vector<Revision> revs;
  for(int i = 1; i <= unique; ++i) {
    if(i < 100)
      std::cout << i << ": " << zipf.freq(i) << std::endl;
    int freq = zipf.freq(i);
    for(int j = 0; j < freq; ++j) {
      revs.push_back({i, 1});
      ++count;
    }
  }
  ofs.write((char*)&revs[0], revs.size() * sizeof(Revision));
  std::cout << "Total count: " << count << std::endl;
}