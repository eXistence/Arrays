#include <benchmark/benchmark.h>
#include "benchmark.h"

static const int minPercentage = 0;
static const int maxPercentage = 100;
static const int minEntities = 1<<6;
static const int maxEntities = 1<<15;

template <class Q, int i> 
void BM_Sequential(benchmark::State& state) {  
  Q entities;

  setup(state, entities);
  
  while (state.KeepRunning()) 
  {    
    update(entities);
  }    

  verify(entities);      
}

BENCHMARK_TEMPLATE2(BM_Sequential, EntityVector, 16)->RangePair(minEntities, maxEntities, minPercentage, maxPercentage);
BENCHMARK_TEMPLATE2(BM_Sequential, EntityArrays, 16)->RangePair(minEntities, maxEntities, minPercentage, maxPercentage);
BENCHMARK_TEMPLATE2(BM_Sequential, EntityArrays2, 16)->RangePair(minEntities, maxEntities, minPercentage, maxPercentage);

BENCHMARK_MAIN()
