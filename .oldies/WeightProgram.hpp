#pragma once

struct Instance;

class WeightProgram {
 public:
  WeightProgram();
  ~WeightProgram();
  void callback(Instance*);

 private:
  long long int pid = 0;
};
