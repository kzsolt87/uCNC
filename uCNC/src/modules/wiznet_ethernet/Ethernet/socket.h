
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define u8_t uint8_t
#define u32_t uint32_t
#define	AF_INET		2		/* internetwork: UDP, TCP, etc. */
#define BACKLOG		4
#define IPPROTO_UDP             17              /* user datagram protocol */
#define IPPROTO_TCP             6               /* tcp */

struct in_addr {
	unsigned long s_addr;          // load with inet_pton()
};

// IPv4 AF_INET sockets:
struct sockaddr_in {
    short            sin_len;
    short            sin_family;   // e.g. AF_INET, AF_INET6
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};

struct sockaddr {
  u8_t sa_len;
  u8_t sa_family;
  char sa_data[14];
};

typedef u32_t socklen_t;


int socket(int domain, int type, int protocol);

int accept(int s, struct sockaddr *addr, socklen_t *addrlen);

int bind(int s, const struct sockaddr *name, socklen_t namelen);

int close(int s);

int connect(int s, const struct sockaddr *name, socklen_t namelen);

int listen(int s, int backlog);

int recv(int s, void *mem, size_t len, int flags);

int recvfrom(int s, void *mem, size_t len, int flags,
      struct sockaddr *from, socklen_t *fromlen);

int send(int s, const void *dataptr, size_t size, int flags);

int sendto(int s, const void *dataptr, size_t size, int flags,
    const struct sockaddr *to, socklen_t tolen);

int getsockname (int s, struct sockaddr *name, socklen_t *namelen);

int getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen);

int setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen);

