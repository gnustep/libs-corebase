#include "CoreFoundation/CFTimeZone.h"
#include "../CFTesting.h"

/* A time zone built from a GMT offset is named GMT for a zero offset and
   GMT followed by the signed four-digit offset otherwise. */

int main (void)
{
  PASS_CFEQ (CFTimeZoneGetName (
    CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, 0.0)),
    CFSTR ("GMT"), "A zero offset is named GMT.");
  PASS_CFEQ (CFTimeZoneGetName (
    CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, 5 * 3600)),
    CFSTR ("GMT+0500"), "A five-hour offset is named GMT+0500.");
  PASS_CFEQ (CFTimeZoneGetName (
    CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, -5 * 3600)),
    CFSTR ("GMT-0500"), "A negative five-hour offset is named GMT-0500.");
  PASS_CFEQ (CFTimeZoneGetName (
    CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, 5 * 3600 + 30 * 60)),
    CFSTR ("GMT+0530"), "A five-and-a-half-hour offset is named GMT+0530.");
  PASS_CFEQ (CFTimeZoneGetName (
    CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, 30 * 60)),
    CFSTR ("GMT+0030"), "A thirty-minute offset is named GMT+0030.");

  return 0;
}
