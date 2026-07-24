#include "CoreFoundation/CFRunLoop.h"
#include "CoreFoundation/CFSocket.h"
#include "CoreFoundation/CFData.h"
#include "../CFTesting.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

/* CFSocket datagram send and address queries over loopback UDP. */

int main (void)
{
  int rfd, sfd;
  struct sockaddr_in raddr;
  socklen_t rlen;
  struct timeval tv;
  CFSocketRef cs;
  CFDataRef addrData, payload, local, peer;
  CFSocketError serr;
  char buf[16];
  ssize_t n;

  /* Receiver bound to a loopback ephemeral port. */
  rfd = socket (AF_INET, SOCK_DGRAM, 0);
  memset (&raddr, 0, sizeof (raddr));
  raddr.sin_family = AF_INET;
  raddr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  bind (rfd, (struct sockaddr *) &raddr, sizeof (raddr));
  rlen = sizeof (raddr);
  getsockname (rfd, (struct sockaddr *) &raddr, &rlen);
  tv.tv_sec = 2;        /* do not hang if no datagram arrives */
  tv.tv_usec = 0;
  setsockopt (rfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof (tv));

  sfd = socket (AF_INET, SOCK_DGRAM, 0);
  cs = CFSocketCreateWithNative (NULL, sfd, 0, NULL, NULL);
  PASS_CF(cs != NULL, "CFSocket created from a native UDP socket.");

  /* Send to an explicit address (the sendto() path). */
  addrData = CFDataCreate (NULL, (const UInt8 *) &raddr, rlen);
  payload = CFDataCreate (NULL, (const UInt8 *) "hello", 5);
  serr = CFSocketSendData (cs, addrData, payload, 1.0);
  PASS_CF(serr == kCFSocketSuccess, "CFSocketSendData reports success.");
  memset (buf, 0, sizeof (buf));
  n = recv (rfd, buf, sizeof (buf), 0);
  PASS_CF(n == 5 && memcmp (buf, "hello", 5) == 0,
    "The 5-byte datagram was actually transmitted and received.");

  /* Connect so the local and peer addresses are well-defined. */
  connect (sfd, (struct sockaddr *) &raddr, rlen);

  local = CFSocketCopyAddress (cs);
  PASS_CF(local != NULL
    && CFDataGetLength (local) == (CFIndex) sizeof (struct sockaddr_in),
    "CFSocketCopyAddress returns a correctly-sized address.");

  peer = CFSocketCopyPeerAddress (cs);
  PASS_CF(peer != NULL
    && CFDataGetLength (peer) == (CFIndex) sizeof (struct sockaddr_in)
    && ((const struct sockaddr_in *) CFDataGetBytePtr (peer))->sin_port
       == raddr.sin_port,
    "CFSocketCopyPeerAddress returns the peer address.");

  return 0;
}
