#pragma  once
#include <stdlib.h>

namespace johl
{
  class Allocator
  {
  public:
    virtual ~Allocator() {}

    Allocator() = default;
    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;

    virtual void* allocate(size_t size) = 0;
    virtual void deallocate(void* p) = 0;

    static Allocator* defaultAllocator();
  };

  class MallocAllocator : public Allocator
  {
  public:
    MallocAllocator() = default;

    virtual ~MallocAllocator() {}

    virtual void* allocate(size_t size) override
    {
      return ::malloc(size);
    }

    virtual void deallocate(void* p) override
    {
      ::free(p);
    }
  };

  inline Allocator* Allocator::defaultAllocator()
  {
    static MallocAllocator a; //threadsafe since C+11, thanks to 'magic static'
    return &a;
  }
}