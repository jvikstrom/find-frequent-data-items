#pragma once
#include <iostream>
#include <vector>
#include <bzlib.h>
#include <stdio.h>
#include <queue>
#include <optional>
#include "lib/readerwriterqueue.h"

struct CouldNotOpenFile {};
struct CouldNotOpenBZip2File {};
struct CouldNotReadNextBlock {};

struct Revision {
  int articleId;
  int revisionId;
};
template<class T>
class RevisionStreamer {
  FILE *pFile;
  BZFILE* bzFile;

  std::string lastBlockLine;
protected:
  void readNextBlock() {
    std::vector<unsigned char> data(4096);
    int bzerror = 0;
    int nRead = BZ2_bzRead(&bzerror, bzFile, &data[0], data.size());
    if(bzerror != BZ_OK) {
      throw CouldNotReadNextBlock();
    }
    std::string str(data.begin(), data.end());
    str = lastBlockLine + str;
    std::vector<std::string> lines;
    int lastIdx = 0;
    for(std::size_t i = 0; i < str.size(); i++) {
      if(str[i] == '\n') {
        std::string line = str.substr(lastIdx, i-lastIdx);
        if(line.substr(0, 9) == "REVISION ") {
          int i = 8;
          int istart = i+1;
          while(line[++i] != ' ');
          int start = i + 1;
          while(line[++i] != ' ');
          static_cast<T*>(this)->onRevision(Revision{std::atoi(line.substr(istart, start-istart).c_str()), std::atoi(line.substr(start, i-start).c_str())});;
        }
        lastIdx = i+1;
      }
    }
    lastBlockLine = str.substr(lastIdx);
  }
public:
  RevisionStreamer(const std::string &path) {
    pFile = fopen(path.c_str(), "r");
    if(!pFile) {
      throw CouldNotOpenFile();
    }
    int bzerror = 0;
    bzFile = BZ2_bzReadOpen(&bzerror, pFile, 0, 0, 0, 0);
    if(bzerror != BZ_OK) {
      throw CouldNotOpenBZip2File();
    }
    //readNextBlock();
  }
  ~RevisionStreamer() {
    fclose(pFile);
  }
  void onRevision(Revision rev);
};

class QueueStreamer : public RevisionStreamer<QueueStreamer> {
  std::queue<Revision> q;

public:
  QueueStreamer(const std::string &path);
  std::optional<Revision> pop();
  void onRevision(Revision rev);
};

class LockQueueStreamer : public RevisionStreamer<LockQueueStreamer> {
  bool shouldStop = false;
  moodycamel::BlockingReaderWriterQueue<Revision> &q;
public:
  LockQueueStreamer(const std::string &path, moodycamel::BlockingReaderWriterQueue<Revision> &q);
  void streamTo();
  void stop();
  void onRevision(Revision rev);
};
