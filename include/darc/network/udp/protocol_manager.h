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
 * DARC LinkManager class
 *
 * \author Morten Kjaergaard
 */

#ifndef __DARC_UDP_PROTOCOL_MANAGER_H_INCLUDED__
#define __DARC_UDP_PROTOCOL_MANAGER_H_INCLUDED__

#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <darc/network/protocol_manager_base.h>
#include <darc/network/udp/link.h>
#include <darc/log.h>

namespace darc
{
namespace network
{
namespace udp
{

class ProtocolManager : public darc::network::ProtocolManagerBase
{
private:
  const static int DEFAULT_LISTEN_PORT = 58500;

  boost::asio::io_service * io_service_;
  boost::asio::ip::udp::resolver resolver_;

  typedef std::map<const ID, udp::LinkPtr> OutboundConnectionListType;
  typedef std::map<const ID, udp::LinkPtr> InboundConnectionListType;

  OutboundConnectionListType outbound_connection_list_;
  InboundConnectionListType inbound_connection_list_;
  udp::LinkPtr last_inbound_;

public:
  ProtocolManager(boost::asio::io_service * io_service, network::LinkManagerCallbackIF * callback) :
    network::ProtocolManagerBase(callback),
    io_service_(io_service),
    resolver_(*io_service)
  {
  }

  boost::asio::ip::udp::endpoint resolve(const std::string& host, const std::string& port)
  {
    // todo: do it async and handle errors and so on....
    boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), host, port);
    return *resolver_.resolve(query);
  }

  void createDefaultAcceptor()
  {
    DARC_INFO("Accepting UDP on (ALL:%u) ", DEFAULT_LISTEN_PORT);
    LinkPtr connection(new udp::Link(callback_, io_service_,
				     boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), DEFAULT_LISTEN_PORT)) );
    inbound_connection_list_.insert( InboundConnectionListType::value_type(connection->getInboundID(), connection) );
    last_inbound_ = connection;
  }

  void sendPacket(const ID& outbound_id,
		  packet::Header::PayloadType type, const ID& recv_node_id,
		  SharedBuffer buffer, std::size_t data_len )
  {
    outbound_connection_list_[outbound_id]->sendPacket(outbound_id, type, recv_node_id, buffer, data_len);
  }

  const ID& accept( const std::string& url )
  {
    boost::smatch what;
    if( boost::regex_match( url, what, boost::regex("^(.+):(\\d+)$") ) )
    {
      DARC_INFO("Accepting UDP on (%s:%s) ", std::string(what[1]).c_str(), std::string(what[2]).c_str());
      LinkPtr connection(new udp::Link(callback_, io_service_,
					 resolve(what[1], what[2])) );
      inbound_connection_list_.insert( InboundConnectionListType::value_type(connection->getInboundID(), connection) );
      last_inbound_ = connection;
      return connection->getInboundID();
    }
    else
    {
      std::cout << "Invalid URL: " << url << std::endl;
    }
    return ID::null();
  }

  const ID& connect( const std::string& url )
  {
    boost::smatch what;
    if( boost::regex_match( url, what, boost::regex("^(.+):(|\\d+)$") ) )
    {
      if( last_inbound_.get() == 0 )
      {
	createDefaultAcceptor();
      }
      // Allocate a connection ID
      const ID& outbound_id = last_inbound_->addOutboundConnection(resolve(what[1], what[2]) );
      outbound_connection_list_.insert( OutboundConnectionListType::value_type(outbound_id, last_inbound_) );
      DARC_INFO("Connecting to UDP (%s:%s) (%s) ", std::string(what[1]).c_str(), std::string(what[2]).c_str(), outbound_id.short_string().c_str());
      last_inbound_->sendDiscover(outbound_id);
      return outbound_id;
    }
    else
    {
      DARC_ERROR("Invalid UDP URL: %s", url.c_str());
      return ID::null();
    }
  }

};

} // namespace udp
} // namespace network
} // namespace darc

#endif
