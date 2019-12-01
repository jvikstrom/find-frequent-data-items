#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <fstream>
#include <bzlib.h>
#include <stdio.h>

#include "lib/readerwriterqueue.h"
#include "RevisionStreamer.h"

int main() {
  std::string outPath = "./out.data";

  moodycamel::BlockingReaderWriterQueue<Revision> q(4096);
  // Main Queue.

  int i = 0;
  int nRead = 100000;
  int lastArticleId = -1;
  int nUnique = 0;
  auto start = std::chrono::system_clock::now();
  std::thread writeThread([&](){
    std::ofstream ofs(outPath.c_str(), std::ios::out | std::ios::binary);
    if(!ofs.is_open()) {
      std::cerr << "Could not open out file" << std::endl;
      throw "How awful";
    }
    Revision rev;
    while(true) {
      std::vector<Revision> revs;
      revs.reserve(512000);
      // Batch as much as possible into one write.
      while(revs.size() < 512000) {
        // FIXME: We could get stuck here when we stop.
        q.wait_dequeue(rev);
        if(i % nRead == 0) {
          auto end = std::chrono::system_clock::now();
          std::chrono::duration<double> elapsedSeconds = end-start;
          start = end;
          std::cout << "Read # unique: " << nUnique << ", total read: " << i << ", per second: " << nRead / elapsedSeconds.count() << "               \r" << std::flush;
        }
        i++;
        if(rev.articleId != lastArticleId) {
          lastArticleId = rev.articleId;
          nUnique++;
        }
        revs.push_back(rev);
      }
      // Append to outfile
      if(!revs.empty()) {
        if(revs.size() >1)
          std::cout << "Writing: " << revs.size() << ":: " << std::endl;
        ofs.write((char*)&revs[0], revs.size() * sizeof(Revision));
      }
    }
    ofs.close();
  });

  LockQueueStreamer streamer("../../enwiki-20080103.main.bz2", q);
  std::thread readThread([&](){
    streamer.streamTo();
  });

  readThread.join();
  writeThread.join();
}