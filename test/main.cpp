#include "SoA.h"
#include <iostream>

using namespace johl;

int main(int argc, char** argv)
{
  using std::cout;
  using std::endl;

  SoA<int, char, double> soa;
//  soa.reserve(10);

  int* is = soa.ptr<0>();
  char* cs = soa.ptr<1>();
  double* ds = soa.ptr<2>();

  soa.append(1,'2',3);
  soa.append(4,'5',6);
  soa.append2(1,2,3);

  cout << "is[0]: " << is[0] << endl;
  cout << "is[1]: " << is[1] << endl;
  cout << "cs[0]: " << cs[0] << endl;
  cout << "cs[1]: " << cs[1] << endl;
  cout << "ds[0]: " << ds[0] << endl;
  cout << "ds[1]: " << ds[1] << endl;


  cout << __TIME__ << " hello world" << endl;
  cout << "size: " << sizeof(detail::Get<1, int, char, double>::Type) << endl;
  cout << "Size::value: " << detail::Size<int, char, double>::value << endl;
}