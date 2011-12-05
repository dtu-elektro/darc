#ifndef __DARC_PUBLISHER_IMPL_H_INCLUDED__
#define __DARC_PUBLISHER_IMPL_H_INCLUDED__

#include <boost/shared_ptr.hpp>
#include <darc/local_dispatcher.h>

namespace darc
{

template<typename T>
class PublisherImpl
{
protected:
  boost::weak_ptr<LocalDispatcher<T> > dispatcher_;

public:
  PublisherImpl()
  {
  }

  void registerDispatcher( boost::weak_ptr<LocalDispatcher<T> > dispatcher )
  {
    dispatcher_ = dispatcher;
  }

  void publish(boost::shared_ptr<T> msg)
  {
    if(boost::shared_ptr<LocalDispatcher<T> > dispatcher_sp = dispatcher_.lock())
    {
      dispatcher_sp->dispatchMessage(msg);
    }
  }

};

}

#endif
