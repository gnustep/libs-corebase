#include "CoreFoundation/CFBitVector.h"
#include "../CFTesting.h"

int main (void)
{
  CFMutableBitVectorRef bv;
  UInt8 out[4];

  bv = CFBitVectorCreateMutable (NULL, 0);
  CFBitVectorSetCount (bv, 16);
  PASS_CF(CFBitVectorGetCount(bv) == 16, "Count set to 16.");

  /* Newly grown bits start cleared. */
  PASS_CF(CFBitVectorGetBitAtIndex(bv, 0) == 0
       && CFBitVectorGetBitAtIndex(bv, 15) == 0, "New bits are 0.");

  /* SetAllBits must set every bit. */
  CFBitVectorSetAllBits (bv, 1);
  PASS_CF(CFBitVectorGetBitAtIndex(bv, 0) == 1
       && CFBitVectorGetBitAtIndex(bv, 15) == 1, "SetAllBits sets all bits.");
  CFBitVectorSetAllBits (bv, 0);

  /* Set a couple of bits and search within sub-ranges (location > 0). */
  CFBitVectorSetBitAtIndex (bv, 5, 1);
  CFBitVectorSetBitAtIndex (bv, 11, 1);
  PASS_CF(CFBitVectorGetFirstIndexOfBit(bv, CFRangeMake(3, 10), 1) == 5,
    "GetFirstIndexOfBit honours the range location.");
  PASS_CF(CFBitVectorGetLastIndexOfBit(bv, CFRangeMake(0, 16), 1) == 11,
    "GetLastIndexOfBit returns the last set bit.");
  PASS_CF(CFBitVectorGetFirstIndexOfBit(bv, CFRangeMake(0, 5), 1) == kCFNotFound,
    "GetFirstIndexOfBit reports no match when the range has none.");

  /* GetBits packs the requested range, most-significant bit first.
     Range (4,8) covers bits 4..11; the set bits 5 and 11 map to range
     offsets 1 and 7 -> 0b01000001 = 0x41. */
  CFBitVectorGetBits (bv, CFRangeMake(4, 8), out);
  PASS_CF(out[0] == 0x41, "GetBits packs the requested range.");

  CFRelease (bv);
  return 0;
}
