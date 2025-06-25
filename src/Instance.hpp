#pragma once

struct InstanceImpl;

struct Instance {
  Instance(int& argc, char** argv);
  InstanceImpl* impl;
};
