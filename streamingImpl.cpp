#include <random>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <map>
#include "BinaryStreamer.h"
#include "RevisionStreamer.h"
#include "Timer.h"

class StatStreamer : public BinaryStreamer<StatStreamer> {
public:
  int i = 0;
  int nArticle = 0;
  int lastArticleId = -1;

  StatStreamer(const std::string &path) : BinaryStreamer<StatStreamer>(path) {}
  void onRevision(Revision rev) {
    if(rev.articleId != lastArticleId) {
      lastArticleId = rev.articleId;
      nArticle++;
    }
    i++;
  }
  void onStart() {}
  void onDone() {}
};

class NaiveStreamer : public BinaryStreamer<NaiveStreamer> {
public:
  int i = 0;
  int nArticle = 0;
  int lastArticleId = -1;

  std::unordered_map<int,int> articleOccurences;
  NaiveStreamer(const std::string &path) : BinaryStreamer<NaiveStreamer>(path) {}
  void onRevision(Revision rev) {
    if(rev.articleId != lastArticleId) {
      lastArticleId = rev.articleId;
      nArticle++;
    }
    i++;

    articleOccurences[rev.articleId]++;
  }
  void onStart() {
    std::cout << "Naive brute force algorithm" << std::endl;
  }
  void onDone() {
    std::vector<std::pair<int,int>> occArticles;
    for(auto &p : articleOccurences) {
      occArticles.push_back({p.second, p.first});
    }
    std::sort(occArticles.begin(), occArticles.end(), std::greater<>());
    int k = 10;
    for(int i = 0; i < k; i++) {
      std::cout << "  " << (i+1) << "th most common: " << occArticles[i].second << " with " << occArticles[i].first << " occurences" << std::endl;
    }
    std::cout << "Used: " << (articleOccurences.size() * sizeof(std::pair<int,int>)) / (1024*1024) << " MB of memory." << std::endl;
  }
};

/***********************************************
*                                              *
* This is where the streaming algorithm starts *
*                                              *
************************************************/

// Cache the primes that have been generated previously for performance.
std::vector<long long> primes;
long long primeGt(long long x) {
  // Find a prime greater than x
  long long i = 2;
  if(primes.size() > 0)
    i = primes.back();
  if(i > x)
    return i; // Easy performance optimization.

  for(; i <= x; ++i) {
    bool divisable = false;
    for(long long y : primes) {
      if(i % y == 0)
        divisable = true;
        break;
    }
    if(!divisable)
      primes.push_back(i);
  }

  std::cout << "Done with initial prime gen..." << std::endl;
  while(true) {
    ++x;
    bool divisable = false;
    for(long long y : primes) {
      if(x % y == 0) {
        divisable = true;
        break;
      }
        
    }
    if(!divisable) {
      primes.push_back(x);
      return x;
    }
      
  }
}

// From: Carter, Larry; Wegman, Mark N. (1979). "Universal Classes of Hash Functions"
class Hash {
  long long a;
  long long b;
  long long p;
  long long m;
public:
  // a and b should be randomly chosen.
  Hash(long long a, long long b, long long m) {
    long long p = primeGt(m);
    // Find a p. Where p > m.
    a = a % p;
    b = b % p;

    this->a = a;
    this->b = b;
    this->p = p;
    this->m = m;
  }
  long long hash(long long x) const {
    return ((a * x + b) % p) % m; // FIXME: This can apparently be implemented with shifts instead, but I'm to lazy for that. Performance is not that crazy important.
  }  
};

// Hash function from objects to {-1,1}.
class SHash {
  Hash hash;
public:
  SHash(long long a, long long b) : hash(a, b, 2){}
  int get(long long x) {
    if(hash.hash(x) == 0) {
      return -1;
    }
    return 1;
  }
  int bytesize() const {
    return sizeof(hash);
  }
  // For debugging purposes.
  long long thehash(long long x) const {
    return hash.hash(x);
  }
};

class HHash {
  std::vector<int> values;
  Hash hash;
public:
  HHash(long long a, long long c, long long b) : hash(a, c, b), values(b) {}
  int& get(long long x) {
    return values[hash.hash(x)];
  }
  int get(long long x) const {
    return values[hash.hash(x)];
  }
  int bytesize() const {
    return values.size() * sizeof(int) + sizeof(hash); // Is this correct?
  }
  long long thehash(long long x) const {
    return hash.hash(x);
  }
};
std::vector<std::pair<HHash, SHash>> getHashes(unsigned seed, long long t, long long b) {
  std::default_random_engine gen(seed);
  std::uniform_int_distribution<int> dist(0, 1e9);
  std::vector<std::pair<HHash, SHash>> hashes;
  std::cout << "Creating hashes with b: " << b << std::endl;
  for(long long i = 0; i < t; i++) {
    std::cout << i << "/" << t << "             \r" << std::flush;
    HHash hhash(dist(gen), dist(gen), b);
    SHash shash(dist(gen), dist(gen));
    hashes.emplace_back(hhash, shash);
  }
  std::cout << "Done creating hashes" << std::endl;
  return hashes;
}
struct KMaxHeap {
  int k;
  struct ValueKey {
    int value;
    int key;
    bool operator<(const ValueKey &rhs){
      return value < rhs.value;
    }
    bool operator() (const ValueKey &lhs, const ValueKey &rhs) const {
        return lhs.value > rhs.value;
    }
  };

  std::vector<int> keys;
  std::vector<int> values;
  int minValue = 1e6;
  void updateMinValue() {
    minValue = 1e6;
    for(int i = 0; i < keys.size(); ++i)
      minValue = std::min(minValue, values[i]);
  }
  void addGte(int q, int v) {
    int i = 0;
    for(; i < keys.size(); ++i) {
      if(keys[i] == q) {
        values[i] += v;
        if(minValue == values[i] - v)
          updateMinValue();
        return;
      }
    }
  }
  void addEstimated(int q, int v) {
    if(keys.size() < k) {
      keys.push_back(q);
      values.push_back(v);
      updateMinValue();
      return;
    }

    if(minValue > v)
      return;

    // Replace minvalue with v.
    for(int i = 0; i < values.size(); ++i) {
      if(values[i] == minValue) {
        values[i] = v;
        keys[i] = q;
        break;
      }
    }
    updateMinValue();

  }
  KMaxHeap(int k) : k(k) {}
  bool in(int q) {
    for(auto &p : keys) {
      if(p == q)
        return true;
    }
    return false;
  }
  int bytesize() {
    return sizeof(int) * keys.size() + sizeof(int) * values.size() + sizeof(minValue) + sizeof(k);
  }
};

class StreamingAlgo {
  long long b;
  long long t;
  std::vector<std::pair<HHash, SHash>> hashes;
  KMaxHeap maxHeap;
public:
  StreamingAlgo(int k, unsigned seed, long long t, long long b) : b(b), t(t), hashes(getHashes(seed, t, b)), maxHeap(k) {}
  int bytesize() {
    long long sz = 0;
    for(const std::pair<HHash, SHash> &hs : hashes) {
      sz += hs.first.bytesize() + hs.second.bytesize();
    }
    return sizeof(b) + sizeof(t) + sz + maxHeap.bytesize();
  }
  void addHash(int x) {
    for(long long i = 0; i < t; ++i) {
      hashes[i].first.get(x) += hashes[i].second.get(x);
    }
  }
  void checkAdd(int x) {
    if(maxHeap.in(x)) {
      // Just increment.
      maxHeap.addGte(x, 1);
    } else {
      // Just try to set the estimate to the heap.
      maxHeap.addEstimated(x, estimate(x));
    }
  }
  void add(int x) {
    addHash(x);
    checkAdd(x);
  }
  int estimate(int q) {
    std::vector<int> vals(t);
    for(long long i = 0; i < t; ++i) {
      vals[i] = hashes[i].first.get(q) * hashes[i].second.get(q);
    }
    std::sort(vals.begin(), vals.end());
    if(vals.size() % 2 == 1) {
      return (vals[vals.size() / 2] + vals[1 + vals.size() / 2]) / 2;
    }
    return vals[vals.size() / 2];
  }
  const KMaxHeap &getMaxHeap() const {
    return maxHeap;
  }
};

class RandomSortedStreamer : public BinaryStreamer<RandomSortedStreamer> {
  RandomSortedStreamer(const std::string &path) : BinaryStreamer<RandomSortedStreamer>(path) {}
  void onRevision(Revision rev) {}
  void onStart() {
    std::cout << "Random sorted streamer" << std::endl;
  }
  void onDone() {

  }
};

template<class T>
constexpr int lg2(T t) {
  int iter = 0;
  long long lt = t;
  while(lt >>= 1) iter++;
  return iter;
}

constexpr int approxN() {
  return 115*1e6;
}

constexpr double getDelta() {
  return 0.001;
}

constexpr double getT(double epsilon) {
  return 22;
}

class RandomShuffledStreamer : public BinaryStreamer<RandomShuffledStreamer> {
  StreamingAlgo algo;
  int k;
  long long t;
  long long b;
  int i = 0;
  Timer timer;
  Timer globalTimer;

public:
  std::unordered_map<int,int> actualOccurenceCounts;
  RandomShuffledStreamer(const std::string &path, unsigned seed, long long t, long long b, int k) : BinaryStreamer<RandomShuffledStreamer>(path), algo(k, seed, t, b), k(k), t(t), b(b) {}
  void onRevision(Revision rev) {
    if(i % 500000 == 0) {
      std::cout << i << ", " << timer.toString() << "         \r" << std::flush;
      timer.restart();
    }
    algo.add(rev.articleId);
    ++i;
  }
  void onStart() {
    std::cout << "Random shuffled streamer" << std::endl;
    std::cout << "  Using t: " << t << ", k: " << k << ", b: " << b << std::endl;
    timer.restart();
    globalTimer.restart();
  }
  void onDone() {
    std::cout << "Total running time was: " << globalTimer.toString() << std::endl;
    std::cout << "Done random shuffled" << std::endl;
    auto &values = algo.getMaxHeap().values;
    auto &keys = algo.getMaxHeap().keys;
    std::vector<std::pair<int,int>> kvp(values.size());
    for(int ii = 0; ii < values.size(); ++ii)
      kvp[ii] = {values[ii], keys[ii]};
    std::sort(kvp.rbegin(), kvp.rend());
    int i = 0;
    for(int ii = 0; ii < kvp.size(); ++ii) {
      int key = kvp[ii].second;
      int value = kvp[ii].first;
      std::cout << "  " << (i+1) << "th most common: " << key << " with " << value << " occurences, actual: " << actualOccurenceCounts[key] << std::endl;
      i++;
    }
    std::cout << "Used: " << algo.bytesize() / (1024 * 1024) << " MB of memory" << std::endl;
  }
};

std::vector<int> getOccurences(const NaiveStreamer &streamer) {
  std::unordered_map<int,int> occs = streamer.articleOccurences;
  std::vector<int> occN;
  for(auto p : occs) {
    occN.push_back(p.second);
  }
  std::sort(occN.rbegin(), occN.rend());
  return occN;
}

// Calculates B given data from a streamer.
int calcB(const std::vector<int> &occN, int k, double epsilon) {
  double sum = 0;
  double knsquare = (occN[k-1] * occN[k-1]);
  for(int i = k; i < occN.size(); ++i)
    sum += occN[i] * occN[i] / knsquare;
  sum *= 256 / (epsilon * epsilon);
  return sum;
}

// Prints statistics in files that can later be processed into graphs.
void datastats(const NaiveStreamer &streamer) {
  std::ofstream occfd("occurences.data");
  if(!occfd.is_open())
    return;
  std::ofstream kbpairfd("k-d-pair.data");
  if(!kbpairfd.is_open())
    return;
  std::vector<int> occs = getOccurences(streamer);
  std::cout << "Writing occurences to file" << std::endl;
  for(int i : occs)
    occfd << i << "\n";
  std::cout << "Writing k - b pairs to file" << std::endl;
  long long t = getT(getDelta());
  for(int i = 1; i < occs.size() - 1; ++i) {
    if(i > 5000)
      i += 10;
    if(i > 25000)
      i+=100;
    if(i > 5000*20)
      i+=1000;
    
    if(i % 1000)
      std::cout << "Wrote " << i << "/" << occs.size() << ": " << occs[i] << "         \r" << std::flush;
    long long b = calcB(occs, i, 0.1);
    kbpairfd << i << "," << b << "," << t << "," << b*t << "\n";
  }
  std::cout << "Done writing to file        " << std::endl;
  std::cout << "m is: " << occs.size() << std::endl;
}

int main(int argc, char* argv[]) {
  {
    /*
      Write to test file
    */
    std::ofstream ofs("./test.data");
    if(!ofs.is_open())
      std::cout << "Could not open test.data file" << std::endl;
    std::vector<Revision> revs;
    revs.push_back(Revision{0,0});
    revs.push_back(Revision{0,1});
    revs.push_back(Revision{1,0});
    revs.push_back(Revision{0,0});
    revs.push_back(Revision{0,1});
    revs.push_back(Revision{3,1});
    revs.push_back(Revision{2,1});
    revs.push_back(Revision{0,1});

    ofs.write((char*)&revs[0], revs.size() * sizeof(Revision));
  }
  std::string dataPath = "./out.data";
  NaiveStreamer streamer(dataPath);
  streamer.startStream();
  std::cout << "N: " << streamer.i << ", unique: " << streamer.nArticle << std::endl;

  int k = 40;
  int t = getT(getDelta());
  int b = calcB(getOccurences(streamer), k, 0.1) ;
  std::cout << "Should have used: " << 2779191 / (1024*1024) << " MB for naive implementation." << std::endl;
  std::cout << "Complete sum: " << b << ", should use: " << getT(getDelta()) * b / (1024*1024) << " MB of memory." << std::endl;
  if(argc > 1)
    datastats(streamer);

  {
    RandomShuffledStreamer rstreamer(dataPath, /*seed*/101230, t, b, k);
    rstreamer.actualOccurenceCounts = streamer.articleOccurences;
    rstreamer.startStream();
  }

}
