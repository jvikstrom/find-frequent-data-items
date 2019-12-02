#include "RevisionStreamer.h"

QueueStreamer::QueueStreamer(const std::string &path) : RevisionStreamer(path) {}

void QueueStreamer::onRevision(Revision rev) {
  q.push(rev);
}

std::optional<Revision> QueueStreamer::pop() {
  while(q.size() < 10)
    readNextBlock();
  if(q.empty()) {
    std::cout << "q is emptyu" << std::endl;
    return {};
  }
  Revision rev = q.front();
  q.pop();
  return std::make_optional(rev);
}

LockQueueStreamer::LockQueueStreamer(const std::string &path, moodycamel::BlockingReaderWriterQueue<Revision> &q) : RevisionStreamer(path), q(q) {}
void LockQueueStreamer::streamTo() {
  while(!shouldStop)
    readNextBlock();
}

void LockQueueStreamer::stop() {
  shouldStop = true;
}

void LockQueueStreamer::onRevision(Revision rev) {
  q.enqueue(rev);
}