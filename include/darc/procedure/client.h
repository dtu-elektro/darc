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
 * DARC Procedure class
 *
 * \author Morten Kjaergaard
 */

#ifndef __DARC_PROCEDURE_CLIENT_H_INCLUDED__
#define __DARC_PROCEDURE_CLIENT_H_INCLUDED__

#include <boost/shared_ptr.hpp>
#include <darc/procedure/client_impl.h>
#include <darc/owner.h>

namespace darc
{
namespace procedure
{

template<typename T_Arg, typename T_Ret, typename T_Sta>
class Client
{
private:
  typename ClientImpl<T_Arg, T_Ret, T_Sta>::Ptr impl_;

public:
  Client(darc::Owner * owner,
	 const std::string& name,
	 typename ClientImpl<T_Arg, T_Ret, T_Sta>::ReturnHandlerType return_handler,
	 typename ClientImpl<T_Arg, T_Ret, T_Sta>::StatusHandlerType status_handler) :
    impl_( new ClientImpl<T_Arg, T_Ret, T_Sta>(owner->getIOService(), name, return_handler, status_handler) )
  {
    owner->getNode()->getProcedureManager().registerClient<T_Arg, T_Ret, T_Sta>(name, impl_);
  }

  ~Client()
  {
    //unregister
  }

  void call(boost::shared_ptr<T_Arg> argument)
  {
    impl_->call(argument);
  }

};

}
}

#endif