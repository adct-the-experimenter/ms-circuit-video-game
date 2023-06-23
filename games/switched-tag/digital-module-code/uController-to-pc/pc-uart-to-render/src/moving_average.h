#include <queue>

class MovingAverage {
 
  // queue used to store list so that we get the average
  std::queue<float> Dataset;
  int period;
  float sum;
 
  public:
  // constructor to initialize period
  MovingAverage(int per) { period = per; }
 
  // function to add new data in the
  // list and update the sum so that
  // we get the new mean
  void addData(float num)
  {
    sum += num;
    Dataset.push(num);
 
    // Updating size so that length
    // of data set should be equal
    // to period as a normal mean has
    if (Dataset.size() > period) {
      sum -= Dataset.front();
      Dataset.pop();
    }
  }
 
  // function to calculate mean
  float getMean() { return sum / period; }
};
