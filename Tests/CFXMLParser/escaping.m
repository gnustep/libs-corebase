#include "CoreFoundation/CFXMLParser.h"
#include "CoreFoundation/CFString.h"

#include "../CFTesting.h"

int
main (void)
{
  CFStringRef esc;
  CFStringRef un;

  /* Each of the five predefined entities is escaped, other text is left
   * alone.
   */
  esc = CFXMLCreateStringByEscapingEntities (NULL, CFSTR ("a<b>c&d\"e'f"),
                                             NULL);
  PASS_CFEQ(esc, CFSTR ("a&lt;b&gt;c&amp;d&quot;e&apos;f"),
            "The five predefined entities are escaped.");

  /* Unescaping is the inverse. */
  un = CFXMLCreateStringByUnescapingEntities (NULL, esc, NULL);
  PASS_CFEQ(un, CFSTR ("a<b>c&d\"e'f"),
            "Unescaping the escaped string round-trips.");
  CFRelease (un);
  CFRelease (esc);

  /* A string with no markup characters is unchanged. */
  esc = CFXMLCreateStringByEscapingEntities (NULL, CFSTR ("plain text 123"),
                                             NULL);
  PASS_CFEQ(esc, CFSTR ("plain text 123"),
            "Text without markup characters is copied verbatim.");
  CFRelease (esc);

  /* The empty string escapes to the empty string. */
  esc = CFXMLCreateStringByEscapingEntities (NULL, CFSTR (""), NULL);
  PASS_CFEQ(esc, CFSTR (""), "The empty string escapes to itself.");
  CFRelease (esc);

  /* Unescaping resolves named entities and numeric character references. */
  un = CFXMLCreateStringByUnescapingEntities (NULL,
    CFSTR ("x &amp; y &lt;z&gt; &#65; &quot;q&quot; &apos;"), NULL);
  PASS_CFEQ(un, CFSTR ("x & y <z> A \"q\" '"),
            "Named entities and a decimal character reference are resolved.");
  CFRelease (un);

  /* A hexadecimal character reference resolves too. */
  un = CFXMLCreateStringByUnescapingEntities (NULL, CFSTR ("&#x41;&#x42;"),
                                              NULL);
  PASS_CFEQ(un, CFSTR ("AB"),
            "A hexadecimal character reference is resolved.");
  CFRelease (un);

  return 0;
}
