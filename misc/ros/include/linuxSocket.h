#ifndef ROS_LINUX_SOCKET_H_
#define ROS_LINUX_SOCKET_H_

#include <memory>

class LinuxSocketImpl;

class LinuxSocket
{
public:
  LinuxSocket ();

  ~LinuxSocket();

  void init (const char *server_hostname);

  int read ();

  void write (const unsigned char *data, int length);

  unsigned long time ();

private:
    std::unique_ptr<LinuxSocketImpl> impl;
};

#endif
