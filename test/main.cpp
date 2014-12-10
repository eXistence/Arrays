#include "SoA.h"
#include <iostream>
#include <string>

using namespace johl;

int main(int argc, char** argv)
{
  using std::cout;
  using std::endl;

  SoA<int, std::string> soa;
//  soa.reserve(10);

  soa.append(1, "one");
  soa.append(2, "two");
  soa.append(3, "three");
  soa.append(4, "four");

  soa.removeAt(2);
  soa.insertAt(2, 5, "five");


  int* is = soa.ptr<0>();
  std::string* ss = soa.ptr<1>();

  for(size_t i=0; i<soa.size(); ++i)
  {
    cout << "(";
    cout << soa.ptr<0>()[i];
    cout << ",";
    cout << soa.ptr<1>()[i];
    cout << ")";
    cout << endl;
  }

  cout << __TIME__ << " hello world" << endl;
  cout << "size: " << sizeof(detail::Get<1, int, char, double>::Type) << endl;
  cout << "Size::value: " << detail::Size<int, char, double>::value << endl;
}