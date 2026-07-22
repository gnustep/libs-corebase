#include "CoreFoundation/CFTimeZone.h"
#include "../CFTesting.h"

/* The name of a GMT-offset zone is GMT for a zero offset and GMT followed by
   the signed four-digit offset otherwise; the abbreviation is compact. */

static CFStringRef
tzname_of (CFTimeInterval ti)
{
  return CFTimeZoneGetName (
    CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, ti));
}

static CFStringRef
tzabbr_of (CFTimeInterval ti)
{
  return CFTimeZoneCopyAbbreviation (
    CFTimeZoneCreateWithTimeIntervalFromGMT (NULL, ti), 0.0);
}

int main (void)
{
  PASS_CFEQ (tzname_of (0.0), CFSTR ("GMT"), "A zero offset is named GMT.");
  PASS_CFEQ (tzname_of (5 * 3600), CFSTR ("GMT+0500"),
    "A five-hour offset is named GMT+0500.");
  PASS_CFEQ (tzname_of (-5 * 3600), CFSTR ("GMT-0500"),
    "A negative five-hour offset is named GMT-0500.");
  PASS_CFEQ (tzname_of (5 * 3600 + 30 * 60), CFSTR ("GMT+0530"),
    "A five-and-a-half-hour offset is named GMT+0530.");
  PASS_CFEQ (tzname_of (30 * 60), CFSTR ("GMT+0030"),
    "A thirty-minute offset is named GMT+0030.");

  PASS_CFEQ (tzabbr_of (0.0), CFSTR ("GMT"),
    "A zero offset abbreviates to GMT.");
  PASS_CFEQ (tzabbr_of (5 * 3600), CFSTR ("GMT+5"),
    "A five-hour offset abbreviates to GMT+5.");
  PASS_CFEQ (tzabbr_of (-5 * 3600), CFSTR ("GMT-5"),
    "A negative five-hour offset abbreviates to GMT-5.");
  PASS_CFEQ (tzabbr_of (5 * 3600 + 30 * 60), CFSTR ("GMT+5:30"),
    "A five-and-a-half-hour offset abbreviates to GMT+5:30.");
  PASS_CFEQ (tzabbr_of (30 * 60), CFSTR ("GMT+0:30"),
    "A thirty-minute offset abbreviates to GMT+0:30.");
  PASS_CFEQ (tzabbr_of (10 * 3600), CFSTR ("GMT+10"),
    "A ten-hour offset abbreviates to GMT+10.");

  return 0;
}
