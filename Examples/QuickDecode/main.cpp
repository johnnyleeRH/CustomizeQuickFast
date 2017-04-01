// Copyright (c) 2009, 2010, 2011, Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
//

#include <Examples/ExamplesPch.h>
#include <QuickDecode/QuickDecode.h>

using namespace QuickFAST;
using namespace Examples;

int main(int argc, char* argv[])
{
  int result = -1;
  QuickDecode decoder("../../SSE_GP_MarketData_Templates.xml", 4096);
  if(decoder.init(std::cout))
  {
    result = decoder.run();
    decoder.fini();
  }
  return result;
}
