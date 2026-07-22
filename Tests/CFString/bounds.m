#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

static CFStringRef
make (UniChar mid)
{
  UniChar buf[3];
  buf[0] = 'A';
  buf[1] = mid;
  buf[2] = 'B';
  return CFStringCreateWithCharacters (NULL, buf, 3);
}

int main (void)
{
  CFStringRef s;
  CFIndex b, e, c;

  /* A newline-terminated line: begin, contents-end (before \n), end (after). */
  s = CFSTR("Hello\nWorld");
  CFStringGetLineBounds (s, CFRangeMake (0, 1), &b, &e, &c);
  PASS_CF(b == 0 && c == 5 && e == 6,
    "CFStringGetLineBounds bounds a newline-terminated line.");
  CFStringGetLineBounds (s, CFRangeMake (6, 1), &b, &e, &c);
  PASS_CF(b == 6 && c == 11 && e == 11,
    "CFStringGetLineBounds bounds the last, unterminated line.");

  /* CRLF counts as a single line terminator. */
  s = CFSTR("A\r\nB");
  CFStringGetLineBounds (s, CFRangeMake (0, 1), &b, &e, &c);
  PASS_CF(b == 0 && c == 1 && e == 3,
    "CFStringGetLineBounds treats CRLF as one terminator.");

  /* U+2028 line separator: breaks a line, not a paragraph. */
  s = make (0x2028);
  CFStringGetLineBounds (s, CFRangeMake (0, 1), &b, &e, &c);
  PASS_CF(b == 0 && c == 1 && e == 2,
    "CFStringGetLineBounds breaks on a line separator.");
  CFStringGetParagraphBounds (s, CFRangeMake (0, 1), &b, &e, &c);
  PASS_CF(b == 0 && c == 3 && e == 3,
    "CFStringGetParagraphBounds does not break on a line separator.");
  CFRelease (s);

  /* U+0085 NEL: breaks a line but not a paragraph. */
  s = make (0x0085);
  CFStringGetLineBounds (s, CFRangeMake (0, 1), &b, &e, &c);
  PASS_CF(b == 0 && c == 1 && e == 2,
    "CFStringGetLineBounds breaks on NEL.");
  CFStringGetParagraphBounds (s, CFRangeMake (0, 1), &b, &e, &c);
  PASS_CF(b == 0 && c == 3 && e == 3,
    "CFStringGetParagraphBounds does not break on NEL.");
  CFRelease (s);

  /* U+2029 paragraph separator: breaks both a line and a paragraph. */
  s = make (0x2029);
  CFStringGetLineBounds (s, CFRangeMake (0, 1), &b, &e, &c);
  PASS_CF(b == 0 && c == 1 && e == 2,
    "CFStringGetLineBounds breaks on a paragraph separator.");
  CFStringGetParagraphBounds (s, CFRangeMake (0, 1), &b, &e, &c);
  PASS_CF(b == 0 && c == 1 && e == 2,
    "CFStringGetParagraphBounds breaks on a paragraph separator.");
  CFRelease (s);

  /* A newline is a paragraph separator. */
  s = CFSTR("P1\nP2");
  CFStringGetParagraphBounds (s, CFRangeMake (0, 1), &b, &e, &c);
  PASS_CF(b == 0 && c == 2 && e == 3,
    "CFStringGetParagraphBounds bounds a newline-terminated paragraph.");

  return 0;
}
