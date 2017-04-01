// Copyright (c) 2009, 2010, 2011, Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
//
#include <Common/QuickFASTPch.h>
#include "CustomizeConnection.h"
#include <Application/DecoderConfiguration_fwd.h>
#include <Codecs/XMLTemplateParser.h>
#include <Codecs/MessagePerPacketAssembler.h>
#include <Codecs/StreamingAssembler.h>
#include <Codecs/NoHeaderAnalyzer.h>
#include <Codecs/FixedSizeHeaderAnalyzer.h>
#include <Codecs/FastEncodedHeaderAnalyzer.h>
#include <Codecs/TemplateRegistry.h>
#include <Codecs/DataSource.h>

#include <Communication/MulticastReceiver.h>
#include <Communication/TCPReceiver.h>
#include <Communication/BufferedRawFileReceiver.h>
#include <Communication/PCapFileReceiver.h>
#include <Communication/AsynchFileReceiver.h>
#include <Communication/BufferReceiver.h>
#include <Communication/AsioService.h>

using namespace QuickFAST;
using namespace Application;

namespace
{
#ifdef _WIN32
  const std::ios::openmode binaryMode = std::ios::binary;
#else
  const std::ios::openmode binaryMode = static_cast<std::ios::openmode>(0);
#endif
}


CustomizeConnection::CustomizeConnection()
: fastFile_(0)
, echoFile_(0)
, verboseFile_(0)
, ownEchoFile_(false)
, ownVerboseFile_(false)
{
}

CustomizeConnection::~CustomizeConnection()
{
  delete fastFile_;
  if(ownEchoFile_)
  {
    delete echoFile_;
  }
  if(ownVerboseFile_)
  {
    delete verboseFile_;
  }
}

void
CustomizeConnection::setTemplateRegistry(Codecs::TemplateRegistryPtr registry)
{
  registry_ = registry;
}

void
CustomizeConnection::configure(
  Messages::ValueMessageBuilder & builder,
  Application::DecoderConfiguration &configuration)
{
  if(!registry_)
  {
    std::ifstream templates(configuration.templateFileName().c_str(),
      std::ios::in | binaryMode
      );
    if(!templates.good())
    {
        std::stringstream msg;
        msg << "Can't open template file: "
          << configuration.templateFileName();
        throw std::invalid_argument(msg.str());
    }
    Codecs::XMLTemplateParser parser;
    parser.setVerboseOutput(*verboseFile_);
    parser.setNonstandard(configuration.nonstandard());
    registry_ = parser.parse(templates);
  }
  if((configuration.nonstandard() & 4) != 0)
  {
    //registry_->display(std::cout, 0);
  }

  Codecs::NoHeaderAnalyzer * analyzer = new Codecs::NoHeaderAnalyzer;
  analyzer->setTestSkip(configuration.testSkip());
  messageHeaderAnalyzer_.reset(analyzer);

  Codecs::StreamingAssembler * pAssembler = new Codecs::StreamingAssembler(
                                                      registry_,
                                                      *messageHeaderAnalyzer_,
                                                      builder,
                                                      configuration.waitForCompleteMessage());
  assembler_.reset(pAssembler);
  pAssembler->setEcho(
                  *echoFile_,
                  static_cast<Codecs::DataSource::EchoType>(configuration.echoType()),
                  configuration.echoMessage(),
                  configuration.echoField());
  pAssembler->setMessageLimit(configuration.head());

  assembler_->setReset(configuration.reset());
  assembler_->setStrict(configuration.strict());

  receiver_.reset(new Communication::CustomizeRawReceiver());
//  receiver_->startRecv(*assembler_, buf, bufsize);
}

Codecs::Decoder &
CustomizeConnection::decoder() const
{
  if(!assembler_)
  {
    throw UsageError("Coding Error","Using CustomizeConnection decoder before it is configured.");
  }
  return assembler_->decoder();
}

void CustomizeConnection::addReceiveBuf(const unsigned char *buf, size_t bufferSize) {
  receiver_->startRecv(*assembler_, buf, bufferSize);
}

