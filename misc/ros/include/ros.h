#ifndef _ROS_H_
#define _ROS_H_

#include "linuxSocket.h"
#include "ros/node_handle.h"

namespace ros
{
  typedef NodeHandle_<LinuxSocket> NodeHandle;
}

#endif
