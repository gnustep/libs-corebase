#include "CoreFoundation/CFBitVector.h"
#include "../CFTesting.h"

int main (void)
{
  CFMutableBitVectorRef bv;
  CFMutableBitVectorRef bv13;
  CFMutableBitVectorRef bv1;
  CFMutableBitVectorRef bv0;
  CFMutableBitVectorRef bvf;
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

  /* A range wider than one byte packs into consecutive output bytes.
     With bits 5, 9 and 11 set, range (3,11) covers bits 3..13; the set
     bits map to range offsets 2, 6 and 8, i.e. out[0] = 0x22 (offsets 2
     and 6) and out[1] = 0x80 (offset 8, the first bit of the next byte). */
  CFBitVectorSetBitAtIndex (bv, 9, 1);
  CFBitVectorGetBits (bv, CFRangeMake(3, 11), out);
  PASS_CF(out[0] == 0x22 && out[1] == 0x80,
    "GetBits packs a range spanning two bytes.");

  CFRelease (bv);

  /* Flipping a range spanning more than one byte must flip each bit exactly
     once (in particular the first byte must not be processed twice). */
  bvf = CFBitVectorCreateMutable (NULL, 0);
  CFBitVectorSetCount (bvf, 16);
  CFBitVectorFlipBits (bvf, CFRangeMake(0, 16));
  PASS_CF(CFBitVectorGetCountOfBit(bvf, CFRangeMake(0, 16), 1) == 16,
    "FlipBits flips every bit of a multi-byte range.");
  CFBitVectorFlipBits (bvf, CFRangeMake(4, 8));
  PASS_CF(CFBitVectorGetCountOfBit(bvf, CFRangeMake(0, 16), 1) == 8,
    "Flipping a sub-range clears exactly that range.");
  CFRelease (bvf);

  /* A count that is not a multiple of 8 exercises the partial final byte. */
  bv13 = CFBitVectorCreateMutable (NULL, 0);
  CFBitVectorSetCount (bv13, 13);
  PASS_CF(CFBitVectorGetCount(bv13) == 13, "Count set to 13.");
  CFBitVectorSetAllBits (bv13, 1);
  PASS_CF(CFBitVectorGetBitAtIndex(bv13, 0) == 1
       && CFBitVectorGetBitAtIndex(bv13, 12) == 1,
    "SetAllBits sets every bit of a 13-bit vector.");
  PASS_CF(CFBitVectorGetCountOfBit(bv13, CFRangeMake(0, 13), 1) == 13,
    "All 13 bits are counted, ignoring the unused tail of the last byte.");
  CFBitVectorSetBitAtIndex (bv13, 8, 0);
  PASS_CF(CFBitVectorGetFirstIndexOfBit(bv13, CFRangeMake(0, 13), 0) == 8,
    "The single clear bit is found in the partial final byte.");
  CFRelease (bv13);

  /* A single-bit vector. */
  bv1 = CFBitVectorCreateMutable (NULL, 0);
  CFBitVectorSetCount (bv1, 1);
  PASS_CF(CFBitVectorGetBitAtIndex(bv1, 0) == 0, "Single new bit is 0.");
  CFBitVectorSetBitAtIndex (bv1, 0, 1);
  PASS_CF(CFBitVectorGetBitAtIndex(bv1, 0) == 1, "Single bit is set.");
  PASS_CF(CFBitVectorGetFirstIndexOfBit(bv1, CFRangeMake(0, 1), 1) == 0
       && CFBitVectorGetLastIndexOfBit(bv1, CFRangeMake(0, 1), 1) == 0,
    "The single set bit is found by both first and last searches.");
  CFRelease (bv1);

  /* An empty vector: queries over the empty range are well-defined. */
  bv0 = CFBitVectorCreateMutable (NULL, 0);
  PASS_CF(CFBitVectorGetCount(bv0) == 0, "Empty vector has count 0.");
  PASS_CF(CFBitVectorGetCountOfBit(bv0, CFRangeMake(0, 0), 1) == 0,
    "The empty range counts no bits.");
  PASS_CF(CFBitVectorGetFirstIndexOfBit(bv0, CFRangeMake(0, 0), 1) == kCFNotFound,
    "The empty range finds no set bit.");
  CFRelease (bv0);

  return 0;
}
