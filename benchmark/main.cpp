#include <benchmark/benchmark.h>
#include "benchmark.h"

static void AosSimple(benchmark::State& state) {  
  AosSimpleContainer32 entities;

  setup(state, entities);
  
  while (state.KeepRunning()) 
  {    
    update(entities);
  }    

  verify(entities);
}

static const int minPercentage = 0;
static const int maxPercentage = 100;
static const int minEntities = 100;
static const int maxEntities = 100000;

// Register the function as a benchmark
BENCHMARK(AosSimple)->RangePair(minEntities, maxEntities, minPercentage, maxPercentage);

static void SoaSimple(benchmark::State& state) {  
  SoaSimpleContainer32 entities;

  setup(state, entities);
  
  while (state.KeepRunning()) 
  {    
    update(entities);
  }    

  verify(entities);    
}
// Register the function as a benchmark
BENCHMARK(SoaSimple)->RangePair(minEntities, maxEntities, minPercentage, maxPercentage);

static void SoaTransform(benchmark::State& state) {  
  SoaTransformContainer32 entities;

  setup(state, entities);
  
  while (state.KeepRunning()) 
  {    
    update(entities);
  }    

  verify(entities);     
}
// Register the function as a benchmark
BENCHMARK(SoaTransform)->RangePair(minEntities, maxEntities, minPercentage, maxPercentage);

BENCHMARK_MAIN()
