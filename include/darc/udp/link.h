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
 * DARC Link class
 *
 * \author Morten Kjaergaard
 */

#ifndef __DARC_UDP_LINK_H_INCLUDED__
#define __DARC_UDP_LINK_H_INCLUDED__

#include <boost/asio.hpp>
#include <darc/link_manager_abstract.h>
#include <darc/serialized_message.h>
#include <darc/node_link.h>
#include <darc/shared_buffer.h>
#include <darc/packet/header.h>
#include <darc/packet/message.h>

namespace darc
{
namespace udp
{

class Link : public darc::NodeLink
{
public:
  typedef boost::shared_ptr<udp::Link> Ptr;

private:
  boost::asio::io_service * io_service_;
  
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint remote_endpoint_;
  
  unsigned int local_port_;

  typedef std::map<uint32_t, boost::asio::ip::udp::endpoint> EndpointsType;
  EndpointsType endpoints_;

public:
  Link(boost::asio::io_service * io_service, unsigned int local_port):
    io_service_(io_service),
    socket_(*io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), local_port)),
    local_port_(local_port)
  {
    startReceive();
  }
  
  void startReceive()
  {
    SharedBuffer recv_buffer = SharedBuffer::create(4098);
    socket_.async_receive_from( boost::asio::buffer(recv_buffer.data(), recv_buffer.size()), remote_endpoint_,
                                boost::bind(&Link::handleReceive, this,
				recv_buffer,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
  }

  void addRemoteNode( uint32_t remote_node_id, const std::string& host, const std::string& port)
  {
    // todo: do it async
    boost::asio::ip::udp::resolver resolver(*io_service_);
    boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), host, port);
    endpoints_[remote_node_id] = *resolver.resolve(query);
  }

  void sendPacket( uint32_t remote_node_id, packet::Header::PayloadType type, SharedBuffer buffer, std::size_t data_len )
  {
    // Create Header
    packet::Header header(node_id_, type);

    boost::array<uint8_t, 512> header_buffer;

    std::size_t header_length = header.write( header_buffer.data(), header_buffer.size() );

    boost::array<boost::asio::const_buffer, 2> combined_buffers = {{
	boost::asio::buffer(header_buffer.data(), header_length),
	boost::asio::buffer(buffer.data(), data_len)
      }};
    
    // todo: to do an async send_to, msg must be kept alive until the send is finished. How to do this?
    //       Impl a object fulfilling the boost buffer interface which holds the smart pointer internally....
    socket_.send_to(combined_buffers, endpoints_[remote_node_id]);
  }
  /*
  // impl of virtual
  void dispatchToRemoteNode( uint32_t remote_node_id, const std::string& topic, SerializedMessage::ConstPtr msg)
  {


    // Send Header
    packet::Header header(node_id_, packet::Header::MSG_PACKET);


    boost::array<uint8_t, 512> tmp_buf;

    size_t pos = header.write( tmp_buf.data(), tmp_buf.size() );


    boost::array<boost::asio::const_buffer, 2> tmp_bufs = {{
	boost::asio::buffer(tmp_buf.data(), pos),
	boost::asio::buffer(msg->getBuffer().data(), msg->getBuffer().size())
      }};
    
    // Send Msg packet

    socket_.send_to(tmp_bufs, endpoints_[remote_node_id]);
  }
  */
 public:
  void handleReceive(SharedBuffer recv_buffer, const boost::system::error_code& error, std::size_t size)
  {
    if ( error )
    {
      std::cerr << "read error: " << boost::system::system_error(error).what() << std::endl;
    }
    else
    {
      //      std::cout << "Received: " << size << " on port " << local_port_ << std::endl;
      receive_callback_( recv_buffer, size );
    }
    startReceive();
  }

};

} // namespace udp
} // namespace darc

#endif
