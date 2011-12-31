/*
 * Copyright (c) 2011, Prevas A/S
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Prevas A/S nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * DARC LocalDispatcherManager class
 *
 * \author Morten Kjaergaard
 */

#ifndef __DARC_PUBLISH_LOCAL_DISPATCHER_MANAGER_H_INCLUDED__
#define __DARC_PUBLISH_LOCAL_DISPATCHER_MANAGER_H_INCLUDED__

#include <vector>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <darc/publish/local_dispatcher.h>
#include <darc/publish/subscriber_impl.h>
#include <darc/publish/publisher_impl.h>
#include <darc/publish/remote_dispatcher_manager.h>

namespace darc
{
namespace publish
{

class LocalDispatcherManager
{
  typedef std::map<const std::string, boost::shared_ptr<LocalDispatcherAbstract> > LocalDispatcherListType;
  LocalDispatcherListType local_dispatcher_list_;
  
public:
  // link stuff
  boost::asio::io_service * io_service_;
  RemoteDispatcherManager * remote_dispatch_handler_;  // todo: use bind instead

public:
  LocalDispatcherManager( boost::asio::io_service * io_service, RemoteDispatcherManager * remote_dispatch_handler ) :
    io_service_(io_service),
    remote_dispatch_handler_(remote_dispatch_handler)
  {
  }

  // called by the Subscriber
  // todo: not thread safe
  template<typename T>
  void registerSubscriber( const std::string& topic, boost::shared_ptr<SubscriberImpl<T> > sub )
  {
    boost::shared_ptr<LocalDispatcher<T> > disp = getLocalDispatcher<T>(topic);
    disp->registerSubscriber( sub );
  }

  // Called by Publisher
  // todo: not thread safe
  template<typename T>
  void registerPublisher( const std::string& topic, boost::shared_ptr<PublisherImpl<T> > pub )
  {
    boost::shared_ptr<LocalDispatcher<T> > disp = getLocalDispatcher<T>(topic);
    pub->registerDispatcher(disp);
  }

  void receiveFromRemoteNode( const std::string& topic, SerializedMessage::ConstPtr msg_s )
  {
    LocalDispatcherListType::iterator elem = local_dispatcher_list_.find(topic);
    if( elem != local_dispatcher_list_.end() )
    {
      elem->second->dispatchMessageLocally( msg_s );
    }
  }

private:
  template<typename T>
  boost::shared_ptr<LocalDispatcher<T> > getLocalDispatcher( const std::string& topic )
  {
    // do single lookup with
    LocalDispatcherListType::iterator elem = local_dispatcher_list_.find(topic);
    if( elem == local_dispatcher_list_.end() )
    {
      boost::shared_ptr<LocalDispatcher<T> > disp( new LocalDispatcher<T>( topic, remote_dispatch_handler_ ) );
      local_dispatcher_list_[ topic ] = disp;
      return disp;
    }
    else
    {
      boost::shared_ptr<LocalDispatcherAbstract> &disp_a = elem->second;
      // todo, try
      boost::shared_ptr<LocalDispatcher<T> > disp = boost::dynamic_pointer_cast<LocalDispatcher<T> >(disp_a);
      return disp;
    }
  }

};

}
}

#endif
