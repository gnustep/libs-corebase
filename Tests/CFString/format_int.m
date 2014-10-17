#include "CoreFoundation/CFString.h"
#include "../CFTesting.h"

int main (void)
{
  CFStringRef str1;
  
  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%x"), 0x321);
  PASS_CFEQ (str1, CFSTR ("321"), "%%x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%.5x"), 0x321);
  PASS_CFEQ (str1, CFSTR ("00321"), "%%.5x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%05x"), 0x321);
  PASS_CFEQ (str1, CFSTR ("00321"), "%%05x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%5.4x"), 0x321);
  PASS_CFEQ (str1, CFSTR (" 0321"), "%%5.4x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%0#5.4x"), 0x321);
  PASS_CFEQ (str1, CFSTR ("0x0321"), "%%0#5.4x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%.*X"), 6, 0xabc);
  PASS_CFEQ (str1, CFSTR ("000ABC"), "%%.*x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%0*X"), 4, 0xabc);
  PASS_CFEQ (str1, CFSTR ("0ABC"), "%%0*x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%*.*X"), 6, 5, 0xabc);
  PASS_CFEQ (str1, CFSTR (" 00ABC"), "%%*.*x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%0*.*X"), 6, 5, 0xabc);
  PASS_CFEQ (str1, CFSTR (" 00ABC"), "%%0*.*x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+#08o"), 0321);
  PASS_CFEQ (str1, CFSTR ("0321    "), "%%-+#08o formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+#08d"), -321);
  PASS_CFEQ (str1, CFSTR ("-321    "), "%%-+#08d formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+#08x"), 0x321);
  PASS_CFEQ (str1, CFSTR ("0x321   "), "%%-+#08x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+#08X"), 0x321);
  PASS_CFEQ (str1, CFSTR ("0X321   "), "%%-+#08X formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+#08u"), 321);
  PASS_CFEQ (str1, CFSTR ("321     "), "%%-+#08u formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+#8o"), 0321);
  PASS_CFEQ (str1, CFSTR ("0321    "), "%%-+#8o formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+#8d"), 321);
  PASS_CFEQ (str1, CFSTR ("+321    "), "%%-+#8d formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+#8x"), 0x321);
  PASS_CFEQ (str1, CFSTR ("0x321   "), "%%-+#8x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+#8X"), 0x321);
  PASS_CFEQ (str1, CFSTR ("0X321   "), "%%-+#8x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+#8u"), 321);
  PASS_CFEQ (str1, CFSTR ("321     "), "%%-+#8u formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+#8o"), 0321);
  PASS_CFEQ (str1, CFSTR ("0321    "), "%%-+#8o formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+#8d"), -321);
  PASS_CFEQ (str1, CFSTR ("-321    "), "%%-+#8d formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+#8x"), 0x321);
  PASS_CFEQ (str1, CFSTR ("0x321   "), "%%-+#8x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+#8X"), 0x321);
  PASS_CFEQ (str1, CFSTR ("0X321   "), "%%-+#8x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+#8u"), 321);
  PASS_CFEQ (str1, CFSTR ("321     "), "%%-+#8u formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+8o"), 0321);
  PASS_CFEQ (str1, CFSTR ("321     "), "%%-+8o formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+8d"), 321);
  PASS_CFEQ (str1, CFSTR ("+321    "), "%%-+8d formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+8x"), 0x321);
  PASS_CFEQ (str1, CFSTR ("321     "), "%%-+8x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+8X"), 0x321);
  PASS_CFEQ (str1, CFSTR ("321     "), "%%-+8x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-+8u"), 321);
  PASS_CFEQ (str1, CFSTR ("321     "), "%%-+8u formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-8o"), 0321);
  PASS_CFEQ (str1, CFSTR ("321     "), "%%-8o formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-8d"), 321);
  PASS_CFEQ (str1, CFSTR ("321     "), "%%-8d formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-8x"), 0x321);
  PASS_CFEQ (str1, CFSTR ("321     "), "%%-8x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-8X"), 0x321);
  PASS_CFEQ (str1, CFSTR ("321     "), "%%-8x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%-8u"), 321);
  PASS_CFEQ (str1, CFSTR ("321     "), "%%-8u formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%+#08o"), 0321);
  PASS_CFEQ (str1, CFSTR ("00000321"), "%%+#08o formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%+#08d"), 321);
  PASS_CFEQ (str1, CFSTR ("+0000321"), "%%+#08d formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%+#08x"), 0x321);
  PASS_CFEQ (str1, CFSTR ("0x000321"), "%%+#08x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%+#08X"), 0x321);
  PASS_CFEQ (str1, CFSTR ("0X000321"), "%%+#08X formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%+#08u"), 321);
  PASS_CFEQ (str1, CFSTR ("00000321"), "%%+#08u formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%+#8o"), 0321);
  PASS_CFEQ (str1, CFSTR ("    0321"), "%%+#8o formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%+#8d"), -321);
  PASS_CFEQ (str1, CFSTR ("    -321"), "%%+#8d formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%+#8x"), 0x321);
  PASS_CFEQ (str1, CFSTR ("   0x321"), "%%+#8x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%+#8X"), 0x321);
  PASS_CFEQ (str1, CFSTR ("   0X321"), "%%+#8X formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%+#8u"), 321);
  PASS_CFEQ (str1, CFSTR ("     321"), "%%+#8u formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%+8o"), 0321);
  PASS_CFEQ (str1, CFSTR ("     321"), "%%+8o formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%+8d"), 321);
  PASS_CFEQ (str1, CFSTR ("    +321"), "%%+8d formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%+8x"), 0x321);
  PASS_CFEQ (str1, CFSTR ("     321"), "%%+8x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%+8X"), 0x321);
  PASS_CFEQ (str1, CFSTR ("     321"), "%%+8X formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%+8u"), 321);
  PASS_CFEQ (str1, CFSTR ("     321"), "%%+8u formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%#08o"), 0321);
  PASS_CFEQ (str1, CFSTR ("00000321"), "%%#08o formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%#08d"), 321);
  PASS_CFEQ (str1, CFSTR ("00000321"), "%%#08d formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%#08x"), 0x321);
  PASS_CFEQ (str1, CFSTR ("0x000321"), "%%#08x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%#08X"), 0x321);
  PASS_CFEQ (str1, CFSTR ("0X000321"), "%%#08X formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%#08u"), 321);
  PASS_CFEQ (str1, CFSTR ("00000321"), "%%#08u formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%#8o"), 0321);
  PASS_CFEQ (str1, CFSTR ("    0321"), "%%#8o formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%#8d"), 321);
  PASS_CFEQ (str1, CFSTR ("     321"), "%%#8d formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%#8x"), 0x321);
  PASS_CFEQ (str1, CFSTR ("   0x321"), "%%#8x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%#8X"), 0x321);
  PASS_CFEQ (str1, CFSTR ("   0X321"), "%%#8X formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%#8u"), 321);
  PASS_CFEQ (str1, CFSTR ("     321"), "%%#8u formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%08o"), 0321);
  PASS_CFEQ (str1, CFSTR ("00000321"), "%%08o formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%08d"), -321);
  PASS_CFEQ (str1, CFSTR ("-0000321"), "%%08d formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%08x"), 0x321);
  PASS_CFEQ (str1, CFSTR ("00000321"), "%%08x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%08X"), 0x321);
  PASS_CFEQ (str1, CFSTR ("00000321"), "%%08X formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%08u"), 321);
  PASS_CFEQ (str1, CFSTR ("00000321"), "%%08u formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%8o"), 0321);
  PASS_CFEQ (str1, CFSTR ("     321"), "%%8o formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%8d"), 321);
  PASS_CFEQ (str1, CFSTR ("     321"), "%%8d formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%8x"), 0x321);
  PASS_CFEQ (str1, CFSTR ("     321"), "%%8x formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%8X"), 0x321);
  PASS_CFEQ (str1, CFSTR ("     321"), "%%8X formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%8u"), 321);
  PASS_CFEQ (str1, CFSTR ("     321"), "%%8u formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%4.2i"), 321);
  PASS_CFEQ (str1, CFSTR (" 321"), "%%4.2i formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%5.1i"), -321);
  PASS_CFEQ (str1, CFSTR (" -321"), "%%5.1i formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%i"), -123);
  PASS_CFEQ (str1, CFSTR ("-123"), "Decimal formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%o"), -123);
  PASS_CFEQ (str1, CFSTR ("37777777605"), "Negative octal formatted correctly");
  CFRelease (str1);

  str1 = CFStringCreateWithFormat (NULL, NULL, CFSTR("%x"), -123);
  PASS_CFEQ (str1, CFSTR ("ffffff85"), "Negative hex formatted correctly");
  CFRelease (str1);

  return 0;
}

