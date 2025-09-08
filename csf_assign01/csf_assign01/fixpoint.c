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

int compareAbsoluteVal(const fixpoint_t *num1, const fixpoint_t *num2) {

  fixpoint_t val1 = *num1;
  fixpoint_t val2 = *num2;
  val1.negative = false;
  val2.negative = false;

  if (val1.whole == 0 && val1.frac == 0){
    val1.negative = false;
  } 

  if (val2.whole == 0 && val2.frac == 0){
    val2.negative = false;
  } 

  return fixpoint_compare(&val1, &val2);

}

result_t
addSameSign (fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right) {
  uint32_t fracSum = left->frac;
  uint32_t wholeSum = left->whole;
  bool carry1;
  //check for overflow potential
  if (fracSum > UINT32_MAX - right->frac) {
    carry1 = 1;
  } else {
    carry1 = 0;
  }

  fracSum += right->frac;
  uint64_t wholeSide = (uint64_t)left->whole + (uint64_t)right->whole;
  if (carry1 == 1) {
    wholeSide += 1;
  }
  result->frac = fracSum;
  result->whole = (uint32_t)wholeSide;

  if (wholeSide >> 32 != 0) {
    return RESULT_OVERFLOW;
  } else {
    return RESULT_OK;
  }

}

result_t
addDiffSign (fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right) {
  uint32_t wholeSum = left->whole;
  uint32_t fracSum  = left->frac;

  //carry the 1
  uint64_t carry1 = (uint64_t)1 << 32;

  if (fracSum < right->frac) {
    fracSum  = (uint32_t)(((uint64_t)fracSum + carry1) - (uint64_t)right->frac);
    wholeSum -= - 1; //account for subtracting from whole
    wholeSum -= right->whole;
  } else {
    fracSum -= right->frac;
    wholeSum -= right->whole;
  }

  if (fracSum == 0 && wholeSum == 0) {
    result->negative = 0;
  }

  result->whole = wholeSum;
  result->frac  = fracSum;
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

    if (addSameSign(result, left, right) == RESULT_OK && result->whole == 0 && result->frac == 0) {
      result->negative = false;
    }
    return addSameSign(result, left, right);

  } else {

    int m = compareAbsoluteVal(left, right); 
    if (m == 0) {
      result->frac  = 0;
      result->whole = 0;
      result->negative = false;
      return RESULT_OK;
    }

    const fixpoint_t *bigger;
    const fixpoint_t *smaller;
    if (m > 0) {
      bigger = left;
      smaller = right;
    } else {
      bigger = right;
      smaller = left;
    }
    //take the sign of the larger
    result->negative = bigger->negative;           
    result_t status = addDiffSign(result, bigger, smaller);

    if (result->whole == 0 && result->frac == 0) {
      result->negative = false;
    }
    return status;
  }
}



result_t
fixpoint_sub( fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right ) {
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
