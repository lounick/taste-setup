#include "linuxSocket.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cstdio>

#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include <errno.h>
#include <sys/types.h>


#define DEFAULT_PORT "11411"

using std::string;
using std::cerr;
using std::endl;

class LinuxSocketImpl
{
  public:
    LinuxSocketImpl(): sockfd(0) {}
    void init (const char *server_hostname) {
      sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
      serv_addr.sin_family = AF_INET;
      if (server_hostname == nullptr) {
        // Do a default initialisation
        serv_addr.sin_port = htons(atoi(DEFAULT_PORT));
        serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) == -1) {
          perror("connect");
          exit(1);
        }
      } else {
        char *result = std::strrchr(const_cast<char *>(server_hostname), ':');
        char port[] = DEFAULT_PORT;
        char *addr = (char*)malloc(sizeof(server_hostname));
        std::strncpy(addr, server_hostname, strlen(server_hostname));
        if (result) {
          // serv_addr.sin_port = htons(atoi(result+1));
          std::strncpy(port, result+1, strlen(port));
          std::strncpy(addr, server_hostname, result-server_hostname);
          // inet_pton(AF_INET, addr, &serv_addr.sin_addr.s_addr);
        }
        struct addrinfo hints, *servinfo, *p;
        int rv;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_CANONNAME;

        if ((rv = getaddrinfo(addr, port, &hints, &servinfo)) != 0) {
          fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
          free(addr);
          exit(1);
        }
        // loop through all the results and connect to the first we can
        for(p = servinfo; p != NULL; p = p->ai_next) {
          printf("%s\n", p->ai_canonname);
          if ((sockfd = socket(p->ai_family, p->ai_socktype,
               p->ai_protocol)) == -1) {
            perror("socket");
            continue;
          }

          if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("connect");
            close(sockfd);
            continue;
          }

          break; // if we get here, we must have connected successfully
        }

        if (p == NULL) {
          // looped off the end of the list with no connection
          fprintf(stderr, "failed to connect\n");
          freeaddrinfo(servinfo);
          free(addr);
          exit(2);
        }
        freeaddrinfo(servinfo);
        free(addr);
      }
    }

    int read() {
      char data;
      int result = recv(sockfd, &data, 1, MSG_DONTWAIT);
      if (result < 0) {
        if (!(errno == EWOULDBLOCK || errno == EAGAIN)) {
          perror("Error receiving from socket");
        }
        return -1;
      } else if (result == 0) {
        perror("Connection to server closed");
        return -1;
      }
      return (unsigned char) data;
    }

    void write(const uint8_t* data, int length) {
      int result = send (sockfd, reinterpret_cast<const char *>(data), length, 0);
      if (result < 0) {
        perror("Error sending data");
        close(sockfd);
        exit(1);
      }
    }

    unsigned long time() {
      struct timespec spec;
      // NON MONOTONIC!
      clock_gettime(CLOCK_REALTIME, &spec);
      return spec.tv_sec * 1000 + spec.tv_nsec / 1000000;
    }
  private:
    int sockfd;
    struct sockaddr_in serv_addr;
};

LinuxSocket::LinuxSocket() {
  impl.reset(new LinuxSocketImpl());
}

LinuxSocket::~LinuxSocket(){}

void LinuxSocket::init (const char *server_hostname) {
  impl->init(server_hostname);
}

int LinuxSocket::read () {
  return impl->read ();
}

void LinuxSocket::write (const uint8_t *data, int length) {
  impl->write (data, length);
}

unsigned long LinuxSocket::time () {
  return impl->time ();
}
