#include "CoreFoundation/CFTimeZone.h"
#include "../CFTesting.h"

/* The name of a GMT-offset zone is GMT for a zero offset and GMT followed by
   the signed four-digit offset otherwise; the abbreviation is compact. */

static CFStringRef
tzname_of (CFTimeInterval ti)
{
  CFTimeZoneRef tz = CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, ti);
  CFStringRef name = (CFStringRef) CFRetain (CFTimeZoneGetName (tz));
  CFRelease (tz);
  return name;
}

static CFStringRef
tzabbr_of (CFTimeInterval ti)
{
  CFTimeZoneRef tz = CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, ti);
  CFStringRef abbr = CFTimeZoneCopyAbbreviation (tz, 0.0);
  CFRelease (tz);
  return abbr;
}

int main (void)
{
  CFStringRef s;

  s = tzname_of (0.0);
  PASS_CFEQ (s, CFSTR ("GMT"), "A zero offset is named GMT.");
  CFRelease (s);
  s = tzname_of (5 * 3600);
  PASS_CFEQ (s, CFSTR ("GMT+0500"), "A five-hour offset is named GMT+0500.");
  CFRelease (s);
  s = tzname_of (-5 * 3600);
  PASS_CFEQ (s, CFSTR ("GMT-0500"),
    "A negative five-hour offset is named GMT-0500.");
  CFRelease (s);
  s = tzname_of (5 * 3600 + 30 * 60);
  PASS_CFEQ (s, CFSTR ("GMT+0530"),
    "A five-and-a-half-hour offset is named GMT+0530.");
  CFRelease (s);
  s = tzname_of (30 * 60);
  PASS_CFEQ (s, CFSTR ("GMT+0030"), "A thirty-minute offset is named GMT+0030.");
  CFRelease (s);

  s = tzabbr_of (0.0);
  PASS_CFEQ (s, CFSTR ("GMT"), "A zero offset abbreviates to GMT.");
  CFRelease (s);
  s = tzabbr_of (5 * 3600);
  PASS_CFEQ (s, CFSTR ("GMT+5"), "A five-hour offset abbreviates to GMT+5.");
  CFRelease (s);
  s = tzabbr_of (-5 * 3600);
  PASS_CFEQ (s, CFSTR ("GMT-5"),
    "A negative five-hour offset abbreviates to GMT-5.");
  CFRelease (s);
  s = tzabbr_of (5 * 3600 + 30 * 60);
  PASS_CFEQ (s, CFSTR ("GMT+5:30"),
    "A five-and-a-half-hour offset abbreviates to GMT+5:30.");
  CFRelease (s);
  s = tzabbr_of (30 * 60);
  PASS_CFEQ (s, CFSTR ("GMT+0:30"),
    "A thirty-minute offset abbreviates to GMT+0:30.");
  CFRelease (s);
  s = tzabbr_of (10 * 3600);
  PASS_CFEQ (s, CFSTR ("GMT+10"), "A ten-hour offset abbreviates to GMT+10.");
  CFRelease (s);

  return 0;
}
