/*#include <boost/serialization/singleton.hpp> */
#include <boost/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/container/detail/singleton.hpp>
using  namespace boost::container::container_detail;
typedef boost::mutex                   Uni_Mutex;
typedef boost::shared_mutex            Thread_WR_Mutex;
typedef boost::unique_lock<Thread_WR_Mutex>   Thread_Write_Lock;
typedef boost::shared_lock<Thread_WR_Mutex>   Thread_Read_Lock;

typedef boost::recursive_mutex         Thread_Recursive_Mutex;
typedef boost::unique_lock<Thread_Recursive_Mutex>  Thread_Recursive_Lock;