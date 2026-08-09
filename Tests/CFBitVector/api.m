#include "CoreFoundation/CFBitVector.h"
#include "../CFTesting.h"

int main (void)
{
  UInt8 bytes[1] = { 0xA0 };            /* 1010 0000, bit 0 is the high bit */
  CFBitVectorRef bv, copy;
  CFMutableBitVectorRef mcopy;

  PASS_CF(CFBitVectorGetTypeID () != 0, "CFBitVectorGetTypeID is not zero.");

  bv = CFBitVectorCreate (NULL, bytes, 8);
  PASS_CF(CFGetTypeID (bv) == CFBitVectorGetTypeID (),
    "A bit vector has the bit-vector type ID.");
  PASS_CF(CFBitVectorGetCount (bv) == 8,
    "CFBitVectorCreate sets the bit count.");
  PASS_CF(CFBitVectorGetBitAtIndex (bv, 0) == 1
      && CFBitVectorGetBitAtIndex (bv, 1) == 0
      && CFBitVectorGetBitAtIndex (bv, 2) == 1
      && CFBitVectorGetBitAtIndex (bv, 3) == 0,
    "CFBitVectorCreate reads bits most-significant first.");

  PASS_CF(CFBitVectorContainsBit (bv, CFRangeMake (0, 8), 1),
    "CFBitVectorContainsBit finds a set bit in a range.");
  PASS_CF(!CFBitVectorContainsBit (bv, CFRangeMake (1, 1), 1),
    "CFBitVectorContainsBit reports a clear bit in a range.");

  copy = CFBitVectorCreateCopy (NULL, bv);
  PASS_CF(CFEqual (copy, bv), "CFBitVectorCreateCopy equals the original.");
  CFRelease (copy);

  mcopy = CFBitVectorCreateMutableCopy (NULL, 0, bv);
  PASS_CF(CFEqual (mcopy, bv),
    "CFBitVectorCreateMutableCopy equals the original.");
  CFRelease (bv);

  CFBitVectorFlipBitAtIndex (mcopy, 1);
  PASS_CF(CFBitVectorGetBitAtIndex (mcopy, 1) == 1,
    "CFBitVectorFlipBitAtIndex flips a single bit.");

  CFBitVectorSetBits (mcopy, CFRangeMake (4, 4), 1);
  PASS_CF(CFBitVectorGetBitAtIndex (mcopy, 4) == 1
      && CFBitVectorGetBitAtIndex (mcopy, 7) == 1,
    "CFBitVectorSetBits sets a range of bits.");
  CFBitVectorSetBits (mcopy, CFRangeMake (0, 8), 0);
  PASS_CF(CFBitVectorGetCountOfBit (mcopy, CFRangeMake (0, 8), 1) == 0,
    "CFBitVectorSetBits clears a range of bits.");
  CFRelease (mcopy);

  return 0;
}
