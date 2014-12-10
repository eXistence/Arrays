#include "SoA.h"

int main(int argc, char** argv)
{
  using std::cout;
  using std::endl;

  SoA<int, char, double> soa;
  soa.reserve(10);

  int* is = soa.ptr<0>();
  is[0] = 42;
  is[1] = 43;

  char* cs = soa.ptr<1>();
  cs[0] = 'F';
  cs[1] = 'Y';

  double* ds = soa.ptr<2>();
  ds[0] = 0.123;
  ds[1] = 0.666;

  cout << "is[0]: " << is[0] << endl;
  cout << "is[1]: " << is[1] << endl;
  cout << "cs[0]: " << cs[0] << endl;
  cout << "cs[1]: " << cs[1] << endl;
  cout << "ds[0]: " << ds[0] << endl;
  cout << "ds[1]: " << ds[1] << endl;


  cout << __TIME__ << " hello world" << endl;
  cout << "size: " << sizeof(Get<1, int, char, double>::Type) << endl;
  cout << "Size::value: " << Size<int, char, double>::value << endl;
}