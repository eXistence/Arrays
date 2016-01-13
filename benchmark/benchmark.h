#pragma once

#include <string>
#include <johl/Arrays.h>
#include <random>
#include <iostream>

using johl::Arrays;
using johl::aligned;
using johl::detail::unused;

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
  char value[64];
};

struct Transform
{
  Vec4 position;
  Vec4 velocity;
};

struct Entity
{
  bool active;
  unsigned id;  
  Vec4 position;  
  Vec4 velocity;
  Name debugname;  
};


inline Entity createEntity(std::mt19937& generator, float threshold)
{
  std::uniform_real_distribution<float> dis(0.0, 1.0);

  Entity e;
  e.active = dis(generator) < threshold;
  e.position = Vec4{dis(generator),dis(generator),dis(generator),dis(generator)};
  e.velocity = Vec4{dis(generator),dis(generator),dis(generator),dis(generator)};

  return e;
}

//=============================================================================
// EntityVector
//=============================================================================

using EntityVector = std::vector<Entity>;

void setup(benchmark::State& state, EntityVector& container);
void update(EntityVector& container);
void verify(EntityVector& container);

inline void setup(benchmark::State& state, EntityVector& container)
{
  container.reserve(state.range_x());

  std::mt19937 generator(0);
  
  for(int i=0;i<state.range_x(); ++i)
  {
    Entity e = createEntity(generator, state.range_y()/100.0f );
    container.push_back(e);
  }  
}

inline void update(EntityVector& container)
{
  const size_t size = container.size();

  for(size_t i=0;i<size; ++i)
  {
    Entity& e = container[i];

    if(e.active)
    {
      e.position += e.velocity * 0.1f;
    }
  }
}

inline void verify(EntityVector& container)
{  
  unused(container);
}


//=============================================================================
// EntityVector
//=============================================================================

using EntityArrays = johl::Arrays<bool, unsigned, johl::aligned<Vec4, 16>, aligned<Vec4, 16>, Name>;

void setup(benchmark::State& state, EntityArrays& container);
void update(EntityArrays& container);
void verify(EntityArrays& container);

inline void setup(benchmark::State& state, EntityArrays& container)
{
  container.reserve(state.range_x());

  std::mt19937 generator(0);

  for(int i=0;i<state.range_x(); ++i)
  {
    Entity e = createEntity(generator,  state.range_y()/100.0f );
    container.append(e.active, e.id, e.position, e.velocity, e.debugname);
  }  
}

inline void update(EntityArrays& container)
{
  const size_t size = container.size();
  const bool* active = container.data<0>();
  const Vec4* velocity = container.data<3>();    
  Vec4* position = container.data<2>();  
 
  for(size_t i=0;i<size; ++i)
  {
    if(active[i])
    {
      position[i] += velocity[i] * 0.1f;
    }
  }
}

inline void verify(EntityArrays& container)
{  
  unused(container);
}


//=============================================================================
// EntityArrays2
//=============================================================================

using EntityArrays2 = johl::Arrays<bool, unsigned, Transform, Name>;

void setup(benchmark::State& state, EntityArrays2& container);
void update(EntityArrays2& container);
void verify(EntityArrays2& container);

inline void setup(benchmark::State& state, EntityArrays2& container)
{
  container.reserve(state.range_x());

  std::mt19937 generator(0);

  for(int i=0;i<state.range_x(); ++i)
  {
    Entity e = createEntity(generator,  state.range_y()/100.0f );
    container.append(e.active, e.id, Transform{e.position, e.velocity}, e.debugname);
  }  
}

inline void update(EntityArrays2& container)
{
  const size_t size = container.size();
  const bool* active = container.data<0>();
  Transform* transform = container.data<2>();    
 
  for(size_t i=0;i<size; ++i)
  {
    if(active[i])
    {
      Transform& t = transform[i];
      t.position += t.velocity * 0.1f;
    }
  }
}

inline void verify(EntityArrays2& container)
{  
  unused(container);
}
