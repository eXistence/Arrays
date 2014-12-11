#include <johl/SoA.h>
#include <iostream>
#include <string>
#include <memory>

using namespace johl;
using std::cout;
using std::endl;

using TestSoA = SoA<int, std::string, double>;

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

  soa.swap(soa2);

  cout << __TIME__ << " hello world" << endl;
  cout << "size: " << sizeof(detail::Get<1, int, char, double>::Type) << endl;
  cout << "Size::value: " << detail::Size<int, char, double>::value << endl;
}