#include <benchmark/benchmark.h>
#include "benchmark.h"

static const int minPercentage = 0;
static const int maxPercentage = 256;
static const int minEntities = 1<<6;
static const int maxEntities = 1<<15;

template <class Q, int i> 
void BM_Sequential(benchmark::State& state) {   

  const int num = state.range_x();
  const float active = static_cast<float>(state.range_y())/256.0f;

  Q entities;
  setup(num, active, entities);
  
  while (state.KeepRunning()) 
  {    
    update(entities);
  }    
}

BENCHMARK_TEMPLATE2(BM_Sequential, EntityVector, 16)->RangePair(minEntities, maxEntities, minPercentage, maxPercentage);
BENCHMARK_TEMPLATE2(BM_Sequential, EntityArrays, 16)->RangePair(minEntities, maxEntities, minPercentage, maxPercentage);
BENCHMARK_TEMPLATE2(BM_Sequential, EntityArrays2, 16)->RangePair(minEntities, maxEntities, minPercentage, maxPercentage);

bool verify()
{
  int num = 100;
  float active = 0.5f;
  EntityVector entityVector;
  EntityArrays entityArrays;
  EntityArrays2 entityArrays2;

  setup(num, active, entityVector);
  setup(num, active, entityArrays);
  setup(num, active, entityArrays2);

  update(entityVector);
  update(entityArrays);
  update(entityArrays2);

  for(int i=0; i<num; ++i)
  {
    Vec4 posVector = entityVector[i].position;
    Vec4 posArrays = entityArrays.data<2>()[i];
    Vec4 posArrays2 = entityArrays2.data<2>()[i].position;

    if(abs(posVector.x - posArrays.x) > 0.001)
      return false;

    if(abs(posVector.x - posArrays2.x) > 0.001)
      return false;    

    if(abs(posVector.y - posArrays.y) > 0.001)
      return false;

    if(abs(posVector.y - posArrays2.y) > 0.001)
      return false;    

    if(abs(posVector.z - posArrays.z) > 0.001)
      return false;

    if(abs(posVector.z - posArrays2.z) > 0.001)
      return false;    
  }

  return true;
}

int main(int argc, char** argv) {
    ::benchmark::Initialize(&argc, argv);

    if(!verify())
    {
      std::cout << "verify failed" << std::endl;
      return 1;
    }

    ::benchmark::RunSpecifiedBenchmarks();
}
