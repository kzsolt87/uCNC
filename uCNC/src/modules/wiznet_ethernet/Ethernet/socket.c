
#include <string.h>
#include "socket.h"
#include "wizchip_socket.h"
// #include "common.h"
#include "netutil.h"

int socket(int domain, int type, int protocol)
{
   int32_t ret;
   int i;
   int internal_protocol;

   if(protocol == IPPROTO_TCP)
	   internal_protocol = Sn_MR_TCP;
   else if(protocol == IPPROTO_UDP)
	   internal_protocol = Sn_MR_UDP;
   else
	   internal_protocol = Sn_MR_TCP;

   for(i=0; i<(_WIZCHIP_SOCK_NUM_+1); i++)
   {
	   if((getSn_RX_RSR(i) == SOCK_CLOSED) && (i<_WIZCHIP_SOCK_NUM_))
	   {
		   break;
	   }
   }

   if(i >= _WIZCHIP_SOCK_NUM_)
	   return -1;

   ret = wizchip_socket(i, internal_protocol, (BSD_TEST_PORT+i), 0x00);
   printf("\r\nSocket:%d Port:%d\r\n", i, (BSD_TEST_PORT+i));

   return ret;
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
   return 0;
}

int bind(int s, const struct sockaddr *name, socklen_t namelen)
{
   return 0;
}

int close(int s)
{
   int32_t ret;

   ret = wizchip_close(s);

   return ret;
}

int connect(int s, const struct sockaddr *name, socklen_t namelen)
{
   int32_t ret;
   uint8_t destip[4];
   uint16_t destport;
   struct sockaddr_in sa;
   uint32_t s_addr;

   sa.sin_port = ((struct sockaddr_in *)name)->sin_port;
   sa.sin_addr.s_addr = ((struct sockaddr_in *)name)->sin_addr.s_addr;
   destport = ntohs(sa.sin_port);
   s_addr = ntohl(sa.sin_addr.s_addr);
   destip[3] = (uint8_t)(s_addr & 0xFF);
   destip[2] = (uint8_t)((s_addr >> 8) & 0xFF);
   destip[1] = (uint8_t)((s_addr >> 16) & 0xFF);
   destip[0] = (uint8_t)((s_addr >> 24) & 0xFF);

   ret = wizchip_connect(s, destip, destport);

   return ret;
}

int listen(int s, int backlog)
{
   int32_t ret;

   ret = wizchip_listen(s);

   return ret;
}

int recv(int s, void *mem, size_t len, int flags)
{
   int32_t ret;

   ret = wizchip_recv(s, mem, len);

   return ret;
}

int recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
{
   int32_t ret;
   uint8_t destip[4];
   uint16_t destport;
   struct sockaddr_in sa;
   uint32_t s_addr;

   ret = wizchip_recvfrom(s, mem, len, destip, (uint16_t*)&destport);

   sa.sin_len = (uint16_t)sizeof(sa);
   sa.sin_family = AF_INET;
   sa.sin_port = htons(destport);
   s_addr = (((((destip[0] << 8) | destip[1]) << 8) | destip[2]) << 8) | destip[3];
   sa.sin_addr.s_addr = htonl(s_addr);
   fromlen = (socklen_t *)sizeof(sa);
   memcpy(from, &sa, sa.sin_len);

   return ret;
}

int send(int s, const void *dataptr, size_t size, int flags)
{
   int32_t ret;

   ret = wizchip_send(s, (uint8_t *)dataptr, size);

   return ret;
}

int sendto(int s, const void *dataptr, size_t size, int flags, const struct sockaddr *to, socklen_t tolen)
{
   int32_t ret;
   uint8_t destip[4];
   uint16_t destport;
   struct sockaddr_in sa;
   uint32_t s_addr;

   sa.sin_port = ((struct sockaddr_in *)to)->sin_port;
   sa.sin_addr.s_addr = ((struct sockaddr_in *)to)->sin_addr.s_addr;
   destport = ntohs(sa.sin_port);
   s_addr = ntohl(sa.sin_addr.s_addr);
   destip[3] = (uint8_t)(s_addr & 0xFF);
   destip[2] = (uint8_t)((s_addr >> 8) & 0xFF);
   destip[1] = (uint8_t)((s_addr >> 16) & 0xFF);
   destip[0] = (uint8_t)((s_addr >> 24) & 0xFF);

   ret = wizchip_sendto(s, (uint8_t *)dataptr, size, destip, destport);

   return ret;
}

int getsockname (int s, struct sockaddr *name, socklen_t *namelen)
{
   return 0;
}

int getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen)
{
   return 0;
}

int setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen)
{
   return 0;
}

