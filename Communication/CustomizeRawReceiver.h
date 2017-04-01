// Copyright (c) 2009, 2010, 2011 Object Computing, Inc.
// All rights reserved.
// See the file license.txt for licensing information.
//
#ifdef _MSC_VER
# pragma once
#endif
#ifndef CUSTOMIZERAWRECEIVER_H
#define CUSTOMIZERAWRECEIVER_H
// All inline, do not export.
//#include <Common/QuickFAST_Export.h>
#include "RawFileReceiver_fwd.h"
#include <Communication/SynchReceiver.h>

namespace QuickFAST
{
  namespace Communication
  {
    /// A Receiver that reads input from an istream.
    class CustomizeRawReceiver
      : public SynchReceiver
    {
    public:
      /// @brief Wrap a standard istream into a Receiver
      ///
      /// The input stream should be opened in binary mode
      /// if that option is available/needed on your operating
      /// system and stream type. (i.e. specify std::ios::binary
      /// when you open a ofstream on Windows.)
      /// @param stream supplies the data
      CustomizeRawReceiver(
        ) : decodefinish(false)
      {
      }

      ~CustomizeRawReceiver()
      {
      }

      bool startRecv(
              Assembler & assembler,
              const unsigned char *buf,
              size_t bufferSize)
      {
        bool result = false;
        assembler_ = & assembler;
        bufferSize_ = bufferSize;
        decodefinish = false;
        if(initializeReceiver())
        {
          assembler_->receiverStarted(*this);

          // Allocate initial set of buffers
          boost::mutex::scoped_lock lock(bufferMutex_);
          BufferLifetime buffer(new LinkedBuffer(buf, bufferSize));
          bufferLifetimes_.push_back(buffer);
          idleBufferPool_.push(buffer.get());
          startReceive(lock);
          result = true;
        }
        return result;
      }

      // Implement Receiver method
      virtual void resetService()
      {
        return;
      }

      //lrh if no buffer for use, return false, else return true
      virtual bool acceptFullBuffer(
              LinkedBuffer * buffer,
              size_t bytesReceived,
              boost::mutex::scoped_lock & lock
      )
      {
        bool needService = false;
        --readsInProgress_;
        ++packetsReceived_;
        if(bytesReceived > 0)
        {
          ++packetsQueued_;
          largestPacket_ = std::max(largestPacket_, bytesReceived);
          buffer->setUsed(bytesReceived);
          //lrh this add the input for service
          needService = queue_.push(buffer, lock);
        }
        else
        {
          // empty buffer? just use it again
          ++emptyPackets_;
          idleBufferPool_.push(buffer);
        }
        return needService;
      }

      virtual LinkedBuffer * getBuffer(bool wait)
      {
//        LinkedBuffer *next = queue_.serviceNext();
//        bool more = true;
//        while(more && next == 0  && !stopping_)
//        {
//          more = wait;
//          {
//            boost::mutex::scoped_lock lock(bufferMutex_);
//            // add any idle buffers to pool
//            idleBufferPool_.push(idleBuffers_);
//            std::cout << "lrh getBuffer call startReceive." << std::endl;
//            startReceive(lock);
//            queue_.refresh(lock, wait && !stopping_);
//          }
//          next = queue_.serviceNext();
//        }
//        if(next != 0)
//        {
//          ++packetsProcessed_;
//          bytesProcessed_ += next->used();
//        }
        return queue_.serviceNext();
      }

      bool decodefinished()
      {
        return decodefinish;
      }

    protected:
      virtual void startReceive(boost::mutex::scoped_lock& lock)
      {
        bool more = canStartRead();
        while( more && !stopping_)
        {
          more = false;
          LinkedBuffer *buffer = idleBufferPool_.pop();
          if(buffer != 0)
          {
            ++readsInProgress_;
            if(fillBuffer(buffer, lock))
            {
              more = canStartRead();
            }
            else
            {
              idleBufferPool_.push(buffer);
              --readsInProgress_;
              stop();
            }
          }
          else
          {
            //std::ostringstream msg;
            //msg << "{" << (void *)this << "} Trying to read. No buffer available\n";
            //std::cout << msg.str();
            ++noBufferAvailable_;
          }
        }
      }

      virtual  bool serviceQueue()
      {
        ++batchesProcessed_;
        if(!assembler_->serviceQueue(*this))
        {
          stop();
        }
        queue_.stopService();
        decodefinish = true;
        return false;
      }

    private:

      // Implement Receiver method
      virtual bool initializeReceiver()
      {
        return true;
      }

      // Implement Receiver method
      bool fillBuffer(LinkedBuffer * buffer, boost::mutex::scoped_lock& lock)
      {
        bool filling = false;
        if(!stopping_)
        {
//          if(acceptFullBuffer(buffer, buffer->used(), lock))
//          {
//            needService_ = true;
//          }
          acceptFullBuffer(buffer, buffer->used(), lock);
          filling = true;
        }
        return filling;
      }

    private:
      bool decodefinish;
    };
  }
}
#endif // CUSTOMIZERAWRECEIVER_H
