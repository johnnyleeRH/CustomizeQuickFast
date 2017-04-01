// Copyright (c) 2009, Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
//
#ifndef QUICKFASTDECODE_H
#define QUICKFASTDECODE_H

#include <Codecs/DataSource.h>
#include <Communication/Receiver_fwd.h>
#include <Application/CommandArgParser.h>
#include <Application/CustomizeConnection.h>

namespace QuickFAST{
  namespace Examples{
    /// @brief lrh need modify Read FAST encoded messages from a file; decode them; and interpret the resulting messages.
    class QuickDecode
    {
    public:
      explicit QuickDecode(std::string templatefile, int bufcount = 1);
      ~QuickDecode();
      /// @returns true if everything is ok.
      bool init(std::ostream &out_ = std::cout);
      /// @brief run the program
      /// @returns a value to be used as an exit code of the program (0 means all is well)
      int run();
      /// @brief do final cleanup after a run.
      void fini();

      void addSource(const unsigned char* buf, size_t bufSize);

      bool decodefinished();

    private:
      typedef boost::shared_ptr<Application::DecoderConfiguration> ConfigurationPtr;
      typedef boost::shared_ptr<Application::CustomizeConnection> ConnectionPtr;
      typedef std::vector<ConnectionPtr> Connections;
      Messages::ValueMessageBuilderPtr builder_;
      ConnectionPtr pConnection_;
      // the currently active configuration
      ConfigurationPtr configuration_;
      bool fixOutput_;
    };
  }
}
#endif // QUICKFASTDECODE_H
