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
  CFSocketContext ctx;
  CFSocketContext got;
  int marker = 42;
  struct sockaddr_in sin;
  CFDataRef addrData;
  CFDataRef bound;

  PASS_CF(CFSocketGetTypeID () != 0, "CFSocketGetTypeID is non-zero.");

  memset (&ctx, 0, sizeof ctx);
  ctx.info = &marker;
  s = CFSocketCreate (NULL, PF_INET, SOCK_STREAM, IPPROTO_TCP, 0, NULL, &ctx);
  PASS_CF(s != NULL, "A TCP socket is created.");

  PASS_CF(CFGetTypeID (s) == CFSocketGetTypeID (),
          "The socket carries the socket type id.");
  PASS_CF(CFSocketGetNative (s) >= 0, "The socket has a native handle.");
  PASS_CF(CFSocketIsValid (s), "A fresh socket is valid.");

  CFSocketSetSocketFlags (s, kCFSocketCloseOnInvalidate
                             | kCFSocketAutomaticallyReenableReadCallBack);
  PASS_CF(CFSocketGetSocketFlags (s) == (CFOptionFlags)
            (kCFSocketCloseOnInvalidate
             | kCFSocketAutomaticallyReenableReadCallBack),
          "Socket flags round-trip through the getter.");

  memset (&got, 0, sizeof got);
  CFSocketGetContext (s, &got);
  PASS_CF(got.info == &marker, "The context info is returned.");

  /* Bind to an ephemeral loopback port and read the address back. */
  memset (&sin, 0, sizeof sin);
  sin.sin_family = AF_INET;
  sin.sin_port = 0;
  sin.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  addrData = CFDataCreate (NULL, (const UInt8 *)&sin, sizeof sin);
  PASS_CF(CFSocketSetAddress (s, addrData) == kCFSocketSuccess,
          "Binding to a loopback address succeeds.");
  CFRelease (addrData);

  bound = CFSocketCopyAddress (s);
  PASS_CF(bound != NULL
          && CFDataGetLength (bound) == (CFIndex)sizeof (struct sockaddr_in),
          "The bound address has the length of a sockaddr_in.");
  if (bound != NULL
      && CFDataGetLength (bound) == (CFIndex)sizeof (struct sockaddr_in))
    {
      const struct sockaddr_in *b =
        (const struct sockaddr_in *)CFDataGetBytePtr (bound);
      PASS_CF(b->sin_family == AF_INET
              && b->sin_addr.s_addr == htonl (INADDR_LOOPBACK),
              "The bound address is the loopback address.");
    }
  if (bound != NULL)
    CFRelease (bound);

  CFSocketInvalidate (s);
  PASS_CF(CFSocketIsValid (s) == false,
          "An invalidated socket is no longer valid.");

  CFRelease (s);

  return 0;
}
