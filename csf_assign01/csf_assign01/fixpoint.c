#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "fixpoint.h"

////////////////////////////////////////////////////////////////////////
// Helper functions
// Note that you can make these "visible" (not static)
// if you want to be able to write unit tests for them
////////////////////////////////////////////////////////////////////////

// TODO LIST
// Check addSameSign and addDiffSign (especially for overflow/underflow)
// Complete add
// Complete parse hex
// Write more Unit Tests

int compareAbsoluteVal(const fixpoint_t *num1, const fixpoint_t *num2)
{

  fixpoint_t val1 = *num1;
  fixpoint_t val2 = *num2;
  val1.negative = false;
  val2.negative = false;

  if (val1.whole == 0 && val1.frac == 0)
  {
    val1.negative = false;
  }

  if (val2.whole == 0 && val2.frac == 0)
  {
    val2.negative = false;
  }

  return fixpoint_compare(&val1, &val2);
}

result_t
addSameSign(fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right)
{
  uint32_t fracSum = left->frac;
  bool carry1;
  // check for overflow potential
  if (fracSum > UINT32_MAX - right->frac)
  {
    carry1 = 1;
  }
  else
  {
    carry1 = 0;
  }

  fracSum += right->frac;
  uint64_t wholeSide = (uint64_t)left->whole + (uint64_t)right->whole;
  if (carry1 == 1)
  {
    wholeSide += 1;
  }
  result->frac = fracSum;
  result->whole = (uint32_t)wholeSide;

  if (wholeSide >> 32 != 0)
  {
    return RESULT_OVERFLOW;
  }
  else
  {
    return RESULT_OK;
  }
}

result_t
addDiffSign(fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right)
{
  uint32_t wholeSum = left->whole;
  uint32_t fracSum = left->frac;

  // carry the 1
  uint64_t carry1 = (uint64_t)1 << 32;

  if (fracSum < right->frac)
  {
    fracSum = (uint32_t)(((uint64_t)fracSum + carry1) - (uint64_t)right->frac);
    wholeSum -= 1; // account for subtracting from whole
    wholeSum -= right->whole;
  }
  else
  {
    fracSum -= right->frac;
    wholeSum -= right->whole;
  }

  if (fracSum == 0 && wholeSum == 0)
  {
    result->negative = 0;
  }

  /*
  if (fracSum >> 32 != 0 || wholeSum >> 32 != 0)
  {
    return RESULT_OVERFLOW;
  }
  */
 
  result->whole = wholeSum;
  result->frac = fracSum;

  return RESULT_OK;
}

////////////////////////////////////////////////////////////////////////
// Public API functions
////////////////////////////////////////////////////////////////////////

void fixpoint_init(fixpoint_t *val, uint32_t whole, uint32_t frac, bool negative)
{
  val->whole = whole;
  val->frac = frac;
  val->negative = negative;

  if (whole == 0 && frac == 0)
  {
    val->negative = false;
  }
}

uint32_t
fixpoint_get_whole(const fixpoint_t *val)
{
  return val->whole;
}

uint32_t
fixpoint_get_frac(const fixpoint_t *val)
{
  return val->frac;
}

bool fixpoint_is_negative(const fixpoint_t *val)
{
  if (val->negative == true)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void fixpoint_negate(fixpoint_t *val)
{
  if (val->whole != 0 || val->frac != 0)
  {
    if (val->negative == true)
    {
      val->negative = false;
    }
    else
    {
      val->negative = true;
    }
  }
}

result_t
fixpoint_add(fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right)
{
  if (left->negative == right->negative)
  {
    result->negative = left->negative;

    if (addSameSign(result, left, right) == RESULT_OK && result->whole == 0 && result->frac == 0)
    {
      result->negative = false;
    }
    return addSameSign(result, left, right);
  }
  else
  {

    int m = compareAbsoluteVal(left, right);
    if (m == 0)
    {
      result->frac = 0;
      result->whole = 0;
      result->negative = false;
      return RESULT_OK;
    }

    const fixpoint_t *bigger;
    const fixpoint_t *smaller;
    if (m > 0)
    {
      bigger = left;
      smaller = right;
    }
    else
    {
      bigger = right;
      smaller = left;
    }
    // take the sign of the larger
    result->negative = bigger->negative;
    result_t status = addDiffSign(result, bigger, smaller);

    if (result->whole == 0 && result->frac == 0)
    {
      result->negative = false;
    }
    return status;
  }
}

result_t
fixpoint_sub(fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right)
{
  fixpoint_t flippedRight = *right;
  fixpoint_negate(&flippedRight);

  return fixpoint_add(result, left, &flippedRight);
}

result_t
fixpoint_mul(fixpoint_t *result, const fixpoint_t *left, const fixpoint_t *right)
{
  // 64 bit magnitudes
  uint64_t WX = ((uint64_t)left->whole << 32) | left->frac;
  uint64_t YZ = ((uint64_t)right->whole << 32) | right->frac;

  // split magnitudes into low and high portions (32 bit)
  uint32_t leftHigh = (uint32_t)(WX >> 32);
  uint32_t leftLow = (uint32_t)(WX & 0xFFFFFFFF);
  uint32_t rightHigh = (uint32_t)(YZ >> 32);
  uint32_t rightLow = (uint32_t)(YZ & 0xFFFFFFFF);

  // 64 bit products uncombined
  uint64_t lowProduct = (uint64_t)leftLow * rightLow;      // 64-bit
  uint64_t lowHighProduct = (uint64_t)leftLow * rightHigh; // 64-bit
  uint64_t highLowProduct = (uint64_t)leftHigh * rightLow; // 64-bit
  uint64_t highProduct = (uint64_t)leftHigh * rightHigh;   // 64-bit

  // recombine to get middle 64 bits split into low and high, get overflowed bits from middleLow to go into middleHigh
  uint64_t middleLow = (lowProduct >> 32) + (lowHighProduct & 0xFFFFFFFF) + (highLowProduct & 0xFFFFFFFF);

  uint64_t midLowOverflow = middleLow >> 32;
  uint64_t middleHigh = (lowHighProduct >> 32) + (highLowProduct >> 32) + highProduct + midLowOverflow;

  // combine low and high of middle bits to get 64 bit middle portion
  uint64_t middle64 = (middleHigh << 32) | (middleLow & 0xFFFFFFFF);

  // split middle 64 into whole and frac parts
  result->whole = (uint32_t)(middle64 >> 32);
  result->frac = (uint32_t)(middle64 & 0xFFFFFFFF);

  // determine sign of product
  bool resultSign = (left->negative != right->negative);
  result->negative = resultSign;

  if (middleHigh >> 32 && (lowProduct & 0xFFFFFFFF) != 0)
  {
    return RESULT_OVERFLOW | RESULT_UNDERFLOW;
  }

  // if high middle bits are nonzero, overflow
  if (middleHigh >> 32)
  {
    return RESULT_OVERFLOW;
  }

  // if low bits are nonzero, underflow
  if ((lowProduct & 0xFFFFFFFF) != 0)
  {
    return RESULT_UNDERFLOW;
  }

  // true zero cannot be negative, but truncated zero maintains it's respective sign
  if (result->whole == 0 && result->frac == 0 && !(middleHigh >> 32) && !(lowProduct & 0xFFFFFFFF))
  {
    result->negative = false;
  }

  return RESULT_OK;
}

int fixpoint_compare(const fixpoint_t *left, const fixpoint_t *right)
{

  if (left->whole == 0 && right->whole == 0 && left->frac == 0 && right->frac == 0)
  {
    return 0;
  }

  if (left->negative && !right->negative)
  {
    return -1;
  }
  if (right->negative && !left->negative)
  {
    return 1;
  }

  int sign;
  if (left->negative == 1)
  {
    sign = -1;
  }
  else
  {
    sign = 1;
  }

  if (left->whole > right->whole)
  {
    return 1 * sign;
  }

  if (left->whole < right->whole)
  {
    return -1 * sign;
  }

  if (left->frac > right->frac)
  {
    return 1 * sign;
  }

  if (left->frac < right->frac)
  {
    return -1 * sign;
  }

  return 0;
}

void fixpoint_format_hex(fixpoint_str_t *s, const fixpoint_t *val)
{
  // array to build format with
  char temp[FIXPOINT_STR_MAX_SIZE];

  // if zero, then format should be "0.0"
  if (val->whole == 0 && val->frac == 0)
  {
    strncpy(s->str, "0.0", FIXPOINT_STR_MAX_SIZE);
    return;
  }

  // populates array, uses lowercase hex, includes 8 digits for frac
  if (val->negative)
  {
    snprintf(temp, sizeof(temp), "-%x.%08x", val->whole, val->frac);
  }
  else
  {
    snprintf(temp, sizeof(temp), "%x.%08x", val->whole, val->frac);
  }

  // trimming of zeros from frac
  char *decimal = strchr(temp, '.');
  if (decimal)
  {
    char *endPtr = temp + strlen(temp) - 1;
    while (endPtr > decimal && *endPtr == '0')
    {
      *endPtr = '\0';
      endPtr--;
    }
    // if all zeros removed, make sure there is a zero after the dot to maintain format
    if (*endPtr == '.')
    {
      *(endPtr + 1) = '0';
      *(endPtr + 2) = '\0';
    }
  }

  // copy string into s->str
  strncpy(s->str, temp, FIXPOINT_STR_MAX_SIZE);
  s->str[FIXPOINT_STR_MAX_SIZE - 1] = '\0';
}

bool fixpoint_parse_hex(fixpoint_t *val, const fixpoint_str_t *s)
{
  const char *str = s->str;
  val->negative = false;

  // if first character is -, mark as negative and continue
  if (*str == '-')
  {
    val->negative = true;
    str++;

    if (*str == '-') {
      return false;
    }
  }

  if (*str == ' ') {
    return false;
  }

  //check if there are '-' anywhere
  if (strchr(str, '-') != NULL) {
    return false;
  }

  // variables to hold parsed whole portion as well as num read characters
  unsigned int wholeParse = 0;
  int numRead = 0;

  if (*str != '.')
  {

    //%n tracks hex digits like instructions said
    if (sscanf(str, "%x%n", &wholeParse, &numRead) != 1)
    {
      // parse failed
      return false;
    }

    // can't have more than 8 hex digits
    if (numRead > 8)
    {
      return false;
    }

    // save val into parse, advance pointer on string by using num chars read, and initialize frac portion to 0
    val->whole = wholeParse;
    str += numRead;
  }
  else
  {
    // no digits before decimal makes whole 0
    val->whole = 0;
  }

  val->frac = 0;

  // if there is decimal parse frac portion
  if (*str == '.')
  {
    // advance past decimal point and get num chars after decimal in len
    str++;

    unsigned int frac = 0;
    int fracDigitsRead = 0;

    // parse frac portion only if present
    if (*str != '\0' && sscanf(str, "%x%n", &frac, &fracDigitsRead) == 1)
    {
      // can't have more than 8 hex digits
      if (fracDigitsRead > 8)
      {
        return false;
      }

      // shift parsed portion into high bits to populate entire frac part (like instructions said)
      val->frac = frac << (4 * (8 - fracDigitsRead));
      str += fracDigitsRead;
    }

    // fail parse if extra chars after frac portion
    if (*str != '\0')
    {
      return false;
    }
  }

  //reject -0
  if (val->whole == 0 && val->frac == 0 && val->negative)
    {
        return false;   
    }

  // if number 0, make it positive
  if (val->whole == 0 && val->frac == 0)
  {
    val->negative = false;
  }

  return true;
}
