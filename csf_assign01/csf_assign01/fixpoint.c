#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "fixpoint.h"

////////////////////////////////////////////////////////////////////////
// Helper functions
// Note that you can make these "visible" (not static)
// if you want to be able to write unit tests for them
////////////////////////////////////////////////////////////////////////

result_t
addSameSign (fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right) {
  uint32_t fracSum = left->frac + right->frac;
  bool extraOne = 0;

  //check if overflow occured
  if (fracSum < right->frac || fracSum < left->frac) { 
    extraOne = 1;
  }
  int32_t wholeSum = left->whole + right->whole;
  if (extraOne == 1) {
    wholeSum += 1;
  }

  result->whole = wholeSum;
  result->frac = fracSum;

  if (wholeSum > UINT32_MAX) {
    return RESULT_OVERFLOW;
  }
  return RESULT_OK;
}

result_t
addDiffSign (fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right) {
  uint32_t fracSum = left->frac;
  uint32_t wholeSum = left->whole;

  if (fracSum < right->frac) {
    fracSum += 0x100000000;
    wholeSum -= 1;
  }

  result->frac = fracSum - right->frac;
  result->whole = wholeSum - left->frac;
  
  //check for special case of zero being negative
  if (wholeSum == 0 && fracSum == 0) {
    result->negative = 0;
  }

  return RESULT_OK;
}

////////////////////////////////////////////////////////////////////////
// Public API functions
////////////////////////////////////////////////////////////////////////

void
fixpoint_init( fixpoint_t *val, uint32_t whole, uint32_t frac, bool negative ) {
  val->whole = whole;
  val->frac = frac;
  val->negative = negative;

  if (whole == 0 && frac == 0) {
    val->negative = false;
  }
}

uint32_t
fixpoint_get_whole( const fixpoint_t *val ) {
  return val->whole;
}

uint32_t
fixpoint_get_frac( const fixpoint_t *val ) {
  return val->frac;
}

bool
fixpoint_is_negative( const fixpoint_t *val ) {
  if (val->negative == true) {
    return true;
  } else {
    return false;
  }
}

void
fixpoint_negate( fixpoint_t *val ) {
  if (val->whole != 0 || val->frac != 0) {
    if (val->negative == true) {
      val->negative = false;
    } else {
      val->negative = true;
    }
  }
}

result_t
fixpoint_add( fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right ) {
  if (left->negative == right->negative) {
    result->negative = left->negative;
   return addSameSign(result, left, right);
  } else {
    if (fixpoint_compare(left, right) == 1 || fixpoint_compare(left, right) == 0) {
      return addDiffSign(result, left, right);
    } else if (fixpoint_compare(left, right) == -1) {
      return addDiffSign(result, right, left);
    } 
  }
}



result_t
fixpoint_sub( fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right ) {
  // TODO: implement
  fixpoint_t flippedRight = *right;
  fixpoint_negate(&flippedRight);

  return fixpoint_add(result, left, &flippedRight);
}

result_t
fixpoint_mul( fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right ) {
  // TODO: implement
  uint64_t leftMag = (((uint64_t)left->whole) << 32) | left->frac;
  uint64_t rightMag = (((uint64_t)right->whole) << 32) | right->frac;

  uint64_t Y = rightMag >> 32;
  uint64_t Z = (uint32_t) rightMag;

  uint64_t PRS = leftMag * Z;
  uint64_t TUV = leftMag * Y;

  uint64_t low32 = (uint32_t) PRS;
  uint64_t mid64 = (PRS >> 32) + (TUV << 32);
  uint64_t high32 = (TUV >> 32);

  result->whole = (uint32_t)(mid64 >> 32);
  result->frac = (uint32_t) mid64;

  result->negative = (left->negative != right->negative);

  if (result->whole == 0 && result->frac == 0 ) {
    result->negative = false;
  }

  if (high32 != 0 && low32 != 0) {
    return RESULT_OVERFLOW | RESULT_UNDERFLOW;
  }
  else if (high32 != 0) {
    return RESULT_OVERFLOW;
  }
  else if (low32 != 0) {
    return RESULT_UNDERFLOW;
  }
  else {
    return RESULT_OK;
  }
}

int
fixpoint_compare( const fixpoint_t *left, const fixpoint_t *right ) {

  if (left->negative && !right->negative) {
    return -1;
  }
  if (right->negative && !left->negative) {
    return 1;
  }

  bool lNeg = left->negative;
  bool rNeg = right->negative;

  if (left->whole > right->whole) {
    if (lNeg && rNeg) {
      return -1;
    } else {
      return 1;
    }
  }

  if (left->whole < right->whole) {
    if (lNeg && rNeg) {
      return 1;
    } else {
      return -1;
    }
  }

  if (left->frac > right->frac) {
    if (lNeg && rNeg) {
      return -1;
    } else {
      return 1;
    }
  }

  if (left->frac < right->frac) {
    if (lNeg && rNeg) {
      return 1;
    } else {
      return -1;
    }
  }

  return 0;
}

void
fixpoint_format_hex( fixpoint_str_t *s, const fixpoint_t *val ) {
  // TODO: implement
  char arr[32];
  char *arrPos = arr;

  if (val->negative && (val->whole != 0 || val->frac != 0)) {
    *arrPos++ = '-';
  }
  
}

bool
fixpoint_parse_hex( fixpoint_t *val, const fixpoint_str_t *s ) {
  // TODO: implement
}
