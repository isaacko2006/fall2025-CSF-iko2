#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "fixpoint.h"

////////////////////////////////////////////////////////////////////////
// Helper functions
// Note that you can make these "visible" (not static)
// if you want to be able to write unit tests for them
////////////////////////////////////////////////////////////////////////

//TODO LIST
//Check addSameSign and addDiffSign (especially for overflow/underflow)
//Complete add 
//Complete parse hex
//Write more Unit Tests

int compareAbsoluteVal(const fixpoint_t *a, const fixpoint_t *b) {

  //write this shit

}

result_t
addSameSign (fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right) {
  /*
  uint64_t fracSum = (uint64_t)left->frac + (uint64_t)right->frac;
  uint32_t fracSumSmaller = (uint32_t)fracSum;
  bool extraOne = 0;

  //check if overflow occured
  if (fracSum < right->frac || fracSum < left->frac) { 
    extraOne = 1;
  }
  //to prevent premature overflow, use a larger bit integer (64 bit)
  int64_t wholeSum = (int64_t) left->whole + (int64_t) right->whole;
  if (extraOne == 1) {
    wholeSum += 1;
  }

  if (wholeSum > INT32_MAX || wholeSum < INT32_MIN) {
    return RESULT_OVERFLOW;
  }

  result->whole = (int32_t) wholeSum;
  result->frac = fracSumSmaller;
  return RESULT_OK;
  */
 uint64_t frac64 = (uint64_t)left->frac + (uint64_t)right->frac;
  uint32_t frac   = (uint32_t)frac64;
  uint64_t carry  = frac64 >> 32;

  // add whole parts (+ carry) in 64 bits to detect overflow
  uint64_t whole64 = (uint64_t)left->whole + (uint64_t)right->whole + carry;
  uint32_t whole   = (uint32_t)whole64;

  result->whole = whole;
  result->frac  = frac;

  result_t st = RESULT_OK;
  if (whole64 >> 32) st |= RESULT_OVERFLOW;   // overflow if high bits set

  return st;
}

result_t
addDiffSign (fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right) {
  /*
  fixpoint_t holder;
  //use larger bits to prevent premature errors
  int64_t fracSum = left->frac;
  int64_t wholeSum = (int64_t)left->whole;

  if (left->frac < right->frac) {
    fracSum += ((uint64_t)1 << 32);
    wholeSum -= 1;
  }
  //Cast down from the larger bit
  holder.frac = (uint32_t)(fracSum - right->frac);
  holder.whole = (uint32_t)(wholeSum - (int64_t)right->whole);

  //check for special case of zero being negative
  if (holder.whole == 0 && holder.frac == 0) {
    holder.negative = 0;
  }

  *result = holder;
  return RESULT_OK;
  */
 uint32_t whole = left->whole;
  uint32_t frac  = left->frac;

  if (frac < right->frac) {
    // borrow 1 from whole into frac
    frac  = (uint32_t)( ((uint64_t)frac + (1ULL << 32)) - (uint64_t)right->frac );
    whole = whole - 1 - right->whole;
  } else {
    frac  = frac  - right->frac;
    whole = whole - right->whole;
  }

  result->whole = whole;
  result->frac  = frac;
  return RESULT_OK;   // no overflow possible in different-sign add
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
  /*
  if (left->negative == right->negative) {
    result->negative = left->negative;
   return addSameSign(result, left, right);
  } else {
    int magnitude = fixpoint_compare(left, right);
    if (magnitude >= 0) {
      result->negative = left->negative;
      return addDiffSign(result, left, right);
    } else {
      result->negative = right->negative;
      return addDiffSign(result, right, left);
    }
  }
  return RESULT_OK;
  */
 if (left->negative == right->negative) {
    // same sign -> add magnitudes; sign is that common sign
    result->negative = left->negative;
    result_t st = addSameSign(result, left, right);

    // if exact and numerically zero, clear sign
    if (st == RESULT_OK && result->whole == 0 && result->frac == 0)
      result->negative = false;

    // note: negative-overflow zero keeps negative=true per spec
    return st;
  } else {
    // different signs -> subtract smaller magnitude from larger
    int m = cmp_mag(left, right);  // compare magnitudes, ignore sign
    if (m == 0) {
      // exact zero
      result->whole = 0;
      result->frac  = 0;
      result->negative = false;
      return RESULT_OK;
    }

    const fixpoint_t *big   = (m > 0) ? left  : right;
    const fixpoint_t *small = (m > 0) ? right : left;

    result->negative = big->negative;           // sign of larger magnitude
    result_t st = addDiffSign(result, big, small);

    if (result->whole == 0 && result->frac == 0)
      result->negative = false;                 // zero is never negative

    return st;  // no overflow in different-sign case
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
  /*
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
    */
}

int
fixpoint_compare( const fixpoint_t *left, const fixpoint_t *right ) {

  if (left->whole == 0 && right->whole == 0 && left->frac == 0 && right->frac == 0) {
    return 0;
  }

  if (left->negative && !right->negative) {
    return -1;
  }
  if (right->negative && !left->negative) {
    return 1;
  }

  int sign;
  if (left->negative == 1) {
    sign = -1;
  } else {
    sign = 1;
  }

  if (left->whole > right->whole) {
    return 1 * sign;
  }

  if (left->whole < right->whole) {
    return -1 * sign;
  }

  if (left->frac > right->frac) {
    return 1 * sign;
  }

  if (left->frac < right->frac) {
    return -1 * sign;
  }

  return 0;
}

void
fixpoint_format_hex( fixpoint_str_t *s, const fixpoint_t *val ) {
  // TODO: implement
  /*
  if (val->negative && (val->whole != 0 || val->frac != 0)) {
    snprintf(s, FIXPOINT_STR_MAX_SIZE, "-%08X.%08X", val->whole, val->frac);
  }
  else {
    snprintf(s, FIXPOINT_STR_MAX_SIZE, "%08X.%08X", val->whole, val->frac);
  }

  char *decimal = strchr(s, '.');
  if (decimal) {
    char *endpoint = s + strlen(s) - 1;
    while (endpoint > decimal && *endpoint == '0') {
      *endpoint = '\0';
      endpoint--;
    }
    if (*endpoint == '.') {
      *endpoint = '\0';
    }
  }
    */
}

bool
fixpoint_parse_hex( fixpoint_t *val, const fixpoint_str_t *s ) {
  // TODO: implement
  /*
  val->negative = false;
  val->whole = 0;
  val->frac = 0;
  const char *str = s;

   if (*str == '-') {
    val->negative = true;
    str++;
   }
   int readChars = 0;
   unsigned int wholePortion = 0;

   if (sscanf(str, "%*X%n", &wholePortion, &readChars) != 1) {
    return false;
   }
   val->whole = wholePortion;
   str += readChars;

   //TODO - Continue implementation of parse
   */
}
