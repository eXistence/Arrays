#include <johl/Arrays.h>

//std stuff
#include <string>
#include <vector>
#include <memory>

//unit test framework
#include <gtest/gtest.h>

using namespace johl;

template<int N>
struct is_power_of_two
{
  static const int value = N && !(N & (N - 1));
};

template<typename TType, size_t TAlign>
struct aligned
{
  static_assert(is_power_of_two<TAlign>::value, "TAlign must be power two");
  //  static_assert( is_power_of_two<TAlign>::val != 0, "what?"):
  using Type = TType;
  static const size_t align = TAlign;
};




template<typename T>
struct AlignedType
{
  using Type = T;
  static const size_t align = 4;
};

template<typename T, size_t TAlign>
struct AlignedType<aligned<T, TAlign>>
{
  using Type = T;
  static const size_t align = TAlign;
};





template<typename... Types>
struct SumAlignment;

template<typename T>
struct SumAlignment<T>
{
  static const size_t value = AlignedType<T>::align;
};

template<typename TFirst, typename... TRest>
struct SumAlignment<TFirst, TRest...>
{
  static const size_t value = AlignedType<TFirst>::align + SumAlignment<TRest...>::value;
};


TEST(ArraysTest, EmptyInstance)
{
  //allocate on the heap to explicitly construct and destruct the container
  std::unique_ptr<Arrays<float,int,double>> instance(new Arrays<float,int,double>());
  ASSERT_TRUE(instance);
  EXPECT_EQ(instance->size(), (size_t)0);
  EXPECT_EQ(instance->capacity(), (size_t)0);
  instance.reset();
  EXPECT_FALSE(instance);
}

TEST(ArraysTest, ReserveEmpty)
{
  Arrays<int, double, bool> arrays;

  arrays.reserve(100);
  EXPECT_EQ(arrays.size(), (size_t)0);
  EXPECT_EQ(arrays.capacity(), (size_t)100);
}

TEST(ArraysTest, FillTrivialTypes)
{
  Arrays<int, double, bool> arrays;

  arrays.append(1, 1.1, true);
  arrays.append(2, 2.2, false);
  arrays.append(3, 3.3, true);

  EXPECT_EQ(arrays.size(), (size_t)3);

  EXPECT_EQ(arrays.at<0>(0), 1);
  EXPECT_EQ(arrays.at<0>(1), 2);
  EXPECT_EQ(arrays.at<0>(2), 3);

  EXPECT_DOUBLE_EQ(arrays.at<1>(0), 1.1);
  EXPECT_DOUBLE_EQ(arrays.at<1>(1), 2.2);
  EXPECT_DOUBLE_EQ(arrays.at<1>(2), 3.3);

  EXPECT_EQ(arrays.at<2>(0), true);
  EXPECT_EQ(arrays.at<2>(1), false);
  EXPECT_EQ(arrays.at<2>(2), true);
}

TEST(ArraysTest, Sequential)
{
  Arrays<int, double, bool> arrays;

  arrays.append(1, 1.1, true);
  arrays.append(2, 2.2, false);
  arrays.append(3, 3.3, true);

  int* ints = arrays.data<0>();
  EXPECT_EQ(1, ints[0]);
  EXPECT_EQ(2, ints[1]);
  EXPECT_EQ(3, ints[2]);

  double* doubles = arrays.data<1>();
  EXPECT_DOUBLE_EQ(1.1, doubles[0]);
  EXPECT_DOUBLE_EQ(2.2, doubles[1]);
  EXPECT_DOUBLE_EQ(3.3, doubles[2]);

  bool* bools = arrays.data<2>();
  EXPECT_TRUE(bools[0]);
  EXPECT_FALSE(bools[1]);
  EXPECT_TRUE(bools[2]);
}

TEST(ArraysTest, FillNonTrivialTypes)
{
  Arrays<int, std::string, std::vector<int>> arrays;

  const std::vector<int> zeros;
  const std::vector<int> ones = { 1 };
  const std::vector<int> twos = { 2, 2 };

  arrays.append(0, "zero", zeros);
  arrays.append(1, "one", ones);
  arrays.append(2, "two", twos);

  const int* ints = arrays.data<0>();
  EXPECT_EQ(0, ints[0]);
  EXPECT_EQ(1, ints[1]);
  EXPECT_EQ(2, ints[2]);

  const std::string* strings = arrays.data<1>();
  EXPECT_EQ("zero", strings[0]);
  EXPECT_EQ("one", strings[1]);
  EXPECT_EQ("two", strings[2]);

  const std::vector<int>* vectors = arrays.data<2>();
  EXPECT_EQ(zeros, vectors[0]);
  EXPECT_EQ(ones, vectors[1]);
  EXPECT_EQ(twos, vectors[2]);
}

TEST(ArraysTest, SwapAt)
{
  Arrays<int, std::string, bool> arrays;

  arrays.append(1, "one", true);
  arrays.append(2, "two", false);
  arrays.append(3, "three", true);

  arrays.swapAt(1,2);

  const int* ints = arrays.data<0>();
  EXPECT_EQ(1, ints[0]);
  EXPECT_EQ(3, ints[1]);
  EXPECT_EQ(2, ints[2]);

  const std::string* strings = arrays.data<1>();
  EXPECT_EQ("one", strings[0]);
  EXPECT_EQ("three", strings[1]);
  EXPECT_EQ("two", strings[2]);

  const bool* bools = arrays.data<2>();
  EXPECT_TRUE(bools[0]);
  EXPECT_TRUE(bools[1]);
  EXPECT_FALSE(bools[2]);
}

namespace 
{
  class TestAllocator : public Allocator
  {
  public:
    TestAllocator()
      : allocations(0)
      , allocator()
    {}

    virtual void* allocate(size_t size) override
    {      
      void* p = allocator.allocate(size);      
      Alloc a = { p, size };
      allocations.push_back(a);
      return p;
    }

    virtual void deallocate(void* p) override
    {
      allocator.deallocate(p);
      
      if(p) 
      { 
        auto pred = [=](const Alloc& a) {return a.p == p; };
        auto i = std::find_if(allocations.begin(), allocations.end(), pred);
        if(i != allocations.end())
        {
          allocations.erase(i);
        }
      }
    }

    struct Alloc    
    {
      void* p;
      size_t size;
    };
    
    std::vector<Alloc> allocations;
    MallocAllocator allocator;
  };
}

TEST(ArraysTest, CustomAllocator)
{
  TestAllocator allocator;

  {
    Arrays<int, double> arrays(&allocator);

    ASSERT_EQ(0, allocator.allocations.size());

    arrays.append(1, 1.1);

    ASSERT_EQ(1, allocator.allocations.size());
    ASSERT_EQ(12, allocator.allocations[0].size); //sizeof(int) + sizeof(double) = 12
  }

  ASSERT_EQ(0, allocator.allocations.size());
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}