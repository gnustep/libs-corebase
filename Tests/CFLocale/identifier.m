#include "CoreFoundation/CFDictionary.h"
#include "CoreFoundation/CFString.h"
#include "CoreFoundation/CFLocale.h"
#include "../CFTesting.h"

int main (void)
{
  CFStringRef str;
  CFStringRef str2;
  CFDictionaryRef dict;
  CFDictionaryRef dict2;
  
  str = CFSTR("sr_Latn_RS@collation=phonebook;currency=USD;calendar=greogian");
  str = CFLocaleCreateCanonicalLocaleIdentifierFromString (NULL, str);
  dict = CFLocaleCreateComponentsFromLocaleIdentifier (NULL, str);
  str2 = CFLocaleCreateLocaleIdentifierFromComponents (NULL, dict);
  PASS_CFEQ(str, str2, "Locale identifiers are the same");
  CFRelease (str);
  CFRelease (str2);
  
  str2 = CFLocaleCreateLocaleIdentifierFromComponents (NULL, dict);
  dict2 = CFLocaleCreateComponentsFromLocaleIdentifier (NULL, str2);
  PASS_CFEQ(dict, dict2, "Locale components are the same");
  
  CFRelease (str2);
  CFRelease (dict);
  CFRelease (dict2);
  
  return 0;
}

