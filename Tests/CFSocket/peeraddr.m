#include "CoreFoundation/CFRunLoop.h"
#include "CoreFoundation/CFSocket.h"
#include "CoreFoundation/CFData.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "../CFTesting.h"

int
main (void)
{
  CFSocketRef s;
  struct sockaddr_in sin;
  CFDataRef addrData;
  CFDataRef local;
  CFDataRef peer;

  s = CFSocketCreate (NULL, PF_INET, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL);
  PASS_CF(s != NULL, "A TCP socket is created.");

  memset (&sin, 0, sizeof sin);
  sin.sin_family = AF_INET;
  sin.sin_port = 0;
  sin.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  addrData = CFDataCreate (NULL, (const UInt8 *)&sin, sizeof sin);
  CFSocketSetAddress (s, addrData);
  CFRelease (addrData);

  /* The local address is available after binding. */
  local = CFSocketCopyAddress (s);
  PASS_CF(local != NULL, "A bound socket has a local address.");

  /* The socket is never connected, so it has no peer address, even after
   * the local address has been read.
   */
  peer = CFSocketCopyPeerAddress (s);
  PASS_CF(peer == NULL,
          "An unconnected socket has no peer address.");

  if (local != NULL) CFRelease (local);
  if (peer != NULL) CFRelease (peer);
  CFSocketInvalidate (s);
  CFRelease (s);

  return 0;
}
