#include <benchmark/benchmark.h>
#include <string>
#include <johl/Arrays.h>
#include <random>
#include <iostream>

using johl::Arrays;
using johl::aligned;



struct Vec4
{
	float x;
	float y;
	float z;
	float w;

	const Vec4& operator+=(const Vec4& v) 
  {
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;

		return *this;
	}

	Vec4 operator*(float f) const
	{
		return Vec4{x*f, y*f, z*f, w*f};
	}
};

struct Name
{
	char value[128];
};

struct Entity
{
	bool active;
	unsigned id;	
	Vec4 position;
	Vec4 velocity;
	Name debugname;
};

static int activeEntities = 0;

static Entity createEntity(std::mt19937& generator, float threshold)
{
  std::uniform_real_distribution<double> dis(0.0, 1.0);

  Entity e;
  e.active = dis(generator) < threshold;

  if(e.active)
  	activeEntities++;

  return e;
}

static void UpdateEntities_AOS(benchmark::State& state) {  
  std::vector<Entity> entities;
  entities.reserve(state.range_x());

  std::mt19937 generator (0);
  activeEntities = 0;
  
  for(size_t i=0;i<entities.capacity(); ++i)
  {
  	Entity e = createEntity(generator, state.range_y()/100.0f );

  	entities.push_back(e);
  }  

  //std::cout << "active: " << activeEntities << std::endl;
  const size_t size = entities.size();

  while (state.KeepRunning()) 
  {    
  	for(size_t i=0;i<size; ++i)
  	{
  		if(entities[i].active)
  		{
  			entities[i].position += entities[i].velocity * 0.1f;
  		}
  	}
  }    
}
// Register the function as a benchmark
BENCHMARK(UpdateEntities_AOS)->RangePair(100, 100000, 0, 100);

static void UpdateEntities_SOA(benchmark::State& state) {  
  johl::Arrays<bool, unsigned, johl::aligned<Vec4, 16>, aligned<Vec4, 16>, Name> entities;
  entities.reserve(state.range_x());

  std::mt19937 generator (0);
  activeEntities = 0;

  for(size_t i=0;i<entities.capacity(); ++i)
  {
  	Entity e = createEntity(generator,  state.range_y()/100.0f );
  	entities.append(e.active, e.id, e.position, e.velocity, e.debugname);
  }  

  const bool* active = entities.data<0>();
  Vec4* position = entities.data<2>();
  Vec4* velocity = entities.data<3>();
  const size_t size = entities.size();

  while (state.KeepRunning()) 
  {    
  	for(size_t i=0;i<size; ++i)
  	{
  		if(active[i])
  		{
  			position[i] += (velocity[i] * 0.1f);
  		}
  	}
  }    
}
// Register the function as a benchmark
BENCHMARK(UpdateEntities_SOA)->RangePair(100, 100000, 0, 100);

struct Transform
{
	Vec4 position;
	Vec4 velocity;
};

static void UpdateEntities_SOA2(benchmark::State& state) {  
  johl::Arrays<bool, unsigned, johl::aligned<Transform, 16>, Name> entities;
  entities.reserve(state.range_x());

  std::mt19937 generator (0);
  activeEntities = 0;

  for(size_t i=0;i<entities.capacity(); ++i)
  {
  	Entity e = createEntity(generator,  state.range_y()/100.0f );
  	entities.append(e.active, e.id, Transform{e.position, e.velocity}, e.debugname);
  }  

  const bool* active = entities.data<0>();
  Transform* transform = entities.data<2>();    
  const size_t size = entities.size();

  while (state.KeepRunning()) 
  {
  	for(size_t i=0;i<size; ++i)
  	{
  		if(active[i])
  		{
  			transform[i].position += transform[i].velocity * 0.1f;
  		}
  	}
  }    
}
// Register the function as a benchmark
BENCHMARK(UpdateEntities_SOA2)->RangePair(100, 100000, 0, 100);

BENCHMARK_MAIN()
