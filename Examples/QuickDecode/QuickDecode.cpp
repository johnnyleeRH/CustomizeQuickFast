// Copyright (c) 2009, 2010, 2011 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
//
#include <Examples/ExamplesPch.h>
#include "QuickDecode.h"
#include <Codecs/XMLTemplateParser.h>
#include <Codecs/GenericMessageBuilder.h>

#include <Communication/Receiver.h>
#include <Application/DecoderConfiguration.h>
#include <Examples/MessageInterpreter.h>
#include <Examples/ValueToFix.h>

namespace {
  template< typename T>
  void ignore_returnvalue(const T&) { }
}

using namespace QuickFAST;
using namespace Examples;

///////////////////////
//

QuickDecode::QuickDecode(std::string templatefile, int bufcount)
: configuration_(new Application::DecoderConfiguration(templatefile,
                                                       Application::DecoderConfiguration::RAWFILE_RECEIVER,
                                                       bufcount))
,fixOutput_(true)
{
}

QuickDecode::~QuickDecode()
{
}

bool
QuickDecode::init(std::ostream &out_)
{
  try
  {
    if(fixOutput_)
    {
      builder_.reset(new ValueToFix(out_));
    }
    else
    {
      MessageInterpreter handler(out_, false);
      builder_.reset(new Codecs::GenericMessageBuilder(handler));
    }

    pConnection_.reset(new Application::CustomizeConnection);
    pConnection_->configure(*builder_, *configuration_);
  }
  catch (std::exception & e)
  {
    std::cout << e.what() << std::endl;
    std::cerr << e.what() << std::endl;
    return false;
  }
  return true;
}

int
QuickDecode::run()
{
  int result = 0;
  try
  {
    pConnection_->receiver().runThreads(1, false);
  }
  catch (std::exception & e)
  {
    std::cout << e.what() << std::endl;
    std::cerr << e.what() << std::endl;
    result = -1;
  }
  return result;
}

void
QuickDecode::addSource(const unsigned char* buf, size_t bufSize)
{
  pConnection_->addReceiveBuf(buf, bufSize);
}

bool
QuickDecode::decodefinished()
{
  return pConnection_->decodefinished();
}

void
QuickDecode::fini()
{
  pConnection_->receiver().stop();
  pConnection_->receiver().joinThreads();
}
