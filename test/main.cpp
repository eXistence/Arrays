#include <johl/SoA.h>
#include <iostream>
#include <string>
#include <memory>

using namespace johl;
using std::cout;
using std::endl;

using TestSoA = SoA<int, std::string, double>;

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


void test(const TestSoA& soa)
{

  auto p = soa.data<1>();
  for(size_t i=0;i<soa.size();++i)
    cout << "s=" << p[i] << endl;

  for (const auto& s : soa.array<1>())
  {
    cout << "s=" << s << endl;
  }

  for (size_t i = 0; i < soa.size(); ++i)
  {
    cout << "(";
    cout << soa.at<0>(i);
    cout << ",";
    cout << soa.at<1>(i);
    cout << ")";
    cout << endl;
  }
}

int main(int argc, char** argv)
{
  cout << "foo: " << SumAlignment<int, aligned<float, 16>, aligned<double, 8>>::value << endl;
  cout << "int : " << AlignedType<int>::align << endl;
  cout << "aligned<float, 16> : " << AlignedType<aligned<float, 16>>::align << endl;

  TestSoA soa;
  TestSoA soa2;

  soa.append(1, "one", 100);
  soa.append(2, "two", 200);
  soa.append(3, "three", 300);
  soa.append(4, "four", 400);

  soa.removeAt(2);
  soa.insertAt(2, 5, "five", 500);

  soa.swapAt(2, 3);
  
  test(soa);

//  soa.swap(soa2);

  cout << __TIME__ << " hello world" << endl;
  cout << "size: " << sizeof(detail::Get<1, int, char, double>::Type) << endl;
  cout << "Size::value: " << detail::Size<int, char, double>::value << endl;
}