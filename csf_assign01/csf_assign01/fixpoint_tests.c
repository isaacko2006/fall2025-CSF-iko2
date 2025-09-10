#include <stdlib.h>
#include <string.h>
#include "tctest.h"
#include "fixpoint.h"

// Test fixture: defines some fixpoint_t instances
// that can be used by test functions
typedef struct {
  fixpoint_t zero;
  fixpoint_t one;
  fixpoint_t one_half;
  fixpoint_t max;
  fixpoint_t neg_three_eighths;
  fixpoint_t min;
  fixpoint_t one_and_one_half;
  fixpoint_t one_hundred;
  fixpoint_t neg_eleven;

  fixpoint_t neg_one_fourth;
  fixpoint_t max_neg;
  fixpoint_t one_third;
  fixpoint_t random_pattern;
  fixpoint_t mid;
  fixpoint_t one_hundred_neg;

} TestObjs;

// Functions to create and destroy the text fixture
// (each test function gets a "fresh" instance of the
// test fixture)
TestObjs *setup( void );
void cleanup( TestObjs *objs );

// The setup() function uses this macro to initialize
// the fixpoint_t objects in the test fixture, to avoid
// being dependent on the fixpoint_init() function.
#define TEST_FIXPOINT_INIT( val, w, f, n ) \
do { \
  (val)->frac = f; \
  (val)->whole = w; \
  (val)->negative = n; \
} while ( 0 )

// Macro to check two fixpoint_t instances for exact equality
#define TEST_EQUAL( val1, val2 ) \
do { \
  ASSERT( (val1)->whole == (val2)->whole ); \
  ASSERT( (val1)->frac == (val2)->frac ); \
  ASSERT( (val1)->negative == (val2)->negative ); \
} while ( 0 )

// Convenience macro to turn a string literal into a const pointer
// to a temporary instance of fixpoint_str_t
#define FIXPOINT_STR( strlit ) &( ( fixpoint_str_t ) { .str = (strlit) } )

// Prototypes for test functions
void test_init( TestObjs *objs );
void test_get_whole( TestObjs *objs );
void test_get_frac( TestObjs *objs );
void test_is_negative( TestObjs *objs );
void test_negate( TestObjs *objs );
void test_add( TestObjs *objs );
void test_sub( TestObjs *objs );
void test_mul( TestObjs *objs );
void test_compare( TestObjs *objs );
void test_format_hex( TestObjs *objs );
void test_parse_hex( TestObjs *objs );

void test_init_2( TestObjs *objs );
void test_get_whole_2( TestObjs *objs );
void test_get_frac_2( TestObjs *objs );
void test_is_negative_2( TestObjs *objs );
void test_negate_2( TestObjs *objs );
void test_add_2( TestObjs *objs );
void test_sub_2( TestObjs *objs );
void test_mul_2( TestObjs *objs );
void test_compare_2( TestObjs *objs );
void test_format_hex_2( TestObjs *objs );
void test_parse_hex_2( TestObjs *objs );


int main( int argc, char **argv ) {
  if ( argc > 1 )
    tctest_testname_to_execute = argv[1];

  TEST_INIT();

  TEST( test_init );
  TEST( test_get_whole );
  TEST( test_get_frac );
  TEST( test_is_negative );
  TEST( test_negate );
  TEST( test_add );
  TEST( test_sub );
  TEST( test_mul );
  TEST( test_compare );
  TEST( test_format_hex );
  TEST( test_parse_hex );

  //NEW UNIT TESTS

  TEST( test_init_2 );
  TEST( test_get_whole_2 );
  TEST( test_get_frac_2 );
  TEST( test_is_negative_2 );
  TEST( test_negate_2 );
  TEST( test_add_2 );
  TEST( test_sub_2 );
  TEST( test_mul_2 );
  TEST( test_compare_2 );
  TEST( test_format_hex_2 );
  TEST( test_parse_hex_2 );


  TEST_FINI();
}



TestObjs *setup( void ) {
  TestObjs *objs = (TestObjs *) malloc( sizeof( TestObjs ) );

  TEST_FIXPOINT_INIT( &objs->zero, 0, 0, false );
  TEST_FIXPOINT_INIT( &objs->one, 1, 0, false );
  TEST_FIXPOINT_INIT( &objs->one_half, 0, 0x80000000, false );
  TEST_FIXPOINT_INIT( &objs->max, 0xFFFFFFFF, 0xFFFFFFFF, false );
  TEST_FIXPOINT_INIT( &objs->neg_three_eighths, 0, 0x60000000, true );
  TEST_FIXPOINT_INIT( &objs->min, 0, 1, false );
  TEST_FIXPOINT_INIT( &objs->one_and_one_half, 1, 0x80000000, false );
  TEST_FIXPOINT_INIT( &objs->one_hundred, 100, 0, false );
  TEST_FIXPOINT_INIT( &objs->neg_eleven, 11, 0, true );

  // TODO: initialize additional fixpoint_t instances
  TEST_FIXPOINT_INIT( &objs->neg_one_fourth, 0,  0x40000000u, true);
  TEST_FIXPOINT_INIT( &objs->max_neg, 0xFFFFFFFF, 0xFFFFFFFF, true );
  TEST_FIXPOINT_INIT( &objs->one_third, 0x55555555u, 0x55555555u, false);
  TEST_FIXPOINT_INIT( &objs->random_pattern, 0x12345666u, 0x9abcdef0u, false);
  TEST_FIXPOINT_INIT( &objs->mid, 0x80000000u,  0x80000000u, false);
  TEST_FIXPOINT_INIT( &objs->one_hundred_neg, 100, 0, true );

  

  return objs;
}

void cleanup( TestObjs *objs ) {
  free( objs );
}

void test_init( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  fixpoint_t val;

  fixpoint_init( &val, 0, 0, false );
  ASSERT( val.whole == 0 );
  ASSERT( val.frac == 0 );
  ASSERT( val.negative == false );

  fixpoint_init( &val, 0xad2b55b1, 0xcf5f4470, true );
  ASSERT( val.whole == 0xad2b55b1 );
  ASSERT( val.frac == 0xcf5f4470 );
  ASSERT( val.negative == true );
}

void test_get_whole( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  ASSERT( fixpoint_get_whole( &objs->zero ) == 0 );
  ASSERT( fixpoint_get_whole( &objs->one ) == 1 );
  ASSERT( fixpoint_get_whole( &objs->one_half ) == 0 );
  ASSERT( fixpoint_get_whole( &objs->max ) == 0xFFFFFFFF );
  ASSERT( fixpoint_get_whole( &objs->neg_three_eighths ) == 0 );
  ASSERT( fixpoint_get_whole( &objs->min ) == 0 );
  ASSERT( fixpoint_get_whole( &objs->one_and_one_half ) == 1 );
  ASSERT( fixpoint_get_whole( &objs->one_hundred ) == 100 );
  ASSERT( fixpoint_get_whole( &objs->neg_eleven ) == 11 );
}

void test_get_frac( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  ASSERT( fixpoint_get_frac( &objs->zero ) == 0 );
  ASSERT( fixpoint_get_frac( &objs->one ) == 0 );
  ASSERT( fixpoint_get_frac( &objs->one_half ) == 0x80000000 );
  ASSERT( fixpoint_get_frac( &objs->max ) == 0xFFFFFFFF );
  ASSERT( fixpoint_get_frac( &objs->neg_three_eighths ) == 0x60000000 );
  ASSERT( fixpoint_get_frac( &objs->min ) == 1 );
  ASSERT( fixpoint_get_frac( &objs->one_and_one_half ) == 0x80000000 );
  ASSERT( fixpoint_get_frac( &objs->one_hundred ) == 0 );
  ASSERT( fixpoint_get_frac( &objs->neg_eleven ) == 0 );
}

void test_is_negative( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  ASSERT( fixpoint_is_negative( &objs->zero ) == false );
  ASSERT( fixpoint_is_negative( &objs->one ) == false );
  ASSERT( fixpoint_is_negative( &objs->one_half ) == false );
  ASSERT( fixpoint_is_negative( &objs->max ) == false );
  ASSERT( fixpoint_is_negative( &objs->neg_three_eighths ) == true );
  ASSERT( fixpoint_is_negative( &objs->min ) == false );
  ASSERT( fixpoint_is_negative( &objs->one_and_one_half ) == false );
  ASSERT( fixpoint_is_negative( &objs->one_hundred ) == false );
  ASSERT( fixpoint_is_negative( &objs->neg_eleven ) == true );
}

void test_negate( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  fixpoint_t result;

  // Negating 0 shouldn't cause it to become negative
  result = objs->zero;
  ASSERT( false == result.negative );
  fixpoint_negate( &result );
  ASSERT( result.whole == objs->zero.whole );
  ASSERT( result.frac == objs->zero.frac );
  ASSERT( false == result.negative );

  // Non-zero values should have their sign flip when negated,
  // but the magnitude should stay the same

  result = objs->one;
  fixpoint_negate( &result );
  ASSERT( result.whole == objs->one.whole );
  ASSERT( result.frac == objs->one.frac );
  ASSERT( true == result.negative );

  result = objs->max;
  fixpoint_negate( &result );
  ASSERT( result.whole == objs->max.whole );
  ASSERT( result.frac == objs->max.frac );
  ASSERT( true == result.negative );

  result = objs->neg_three_eighths;
  fixpoint_negate( &result );
  ASSERT( result.whole == objs->neg_three_eighths.whole );
  ASSERT( result.frac == objs->neg_three_eighths.frac );
  ASSERT( false == result.negative );
}

void test_add( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  fixpoint_t result;
  
  ASSERT( fixpoint_add( &result, &objs->zero, &objs->zero ) == RESULT_OK );
  ASSERT( 0 == result.frac );
  ASSERT( 0 == result.whole );
  ASSERT( false == result.negative );

  ASSERT( fixpoint_add( &result, &objs->zero, &objs->one ) == RESULT_OK );
  ASSERT( 0 == result.frac );
  ASSERT( 1 == result.whole );
  ASSERT( false == result.negative );

  ASSERT( fixpoint_add( &result, &objs->max, &objs->one ) == RESULT_OVERFLOW );

  ASSERT( fixpoint_add( &result, &objs->max, &objs->min ) == RESULT_OVERFLOW );
  
  ASSERT( fixpoint_add( &result, &objs->zero, &objs->neg_three_eighths ) == RESULT_OK );
  ASSERT( 0x60000000 == result.frac );
  ASSERT( 0 == result.whole );
  ASSERT( true == result.negative );

  fixpoint_t neg_max = objs->max;
  neg_max.negative = true;

  fixpoint_t neg_min = objs->min;
  neg_min.negative = true;

  ASSERT( fixpoint_add( &result, &neg_max, &neg_min ) == RESULT_OVERFLOW );
}

void test_sub( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  fixpoint_t result;

  ASSERT( fixpoint_sub( &result, &objs->one, &objs->zero ) == RESULT_OK );
  ASSERT( 1 == result.whole );
  ASSERT( 0 == result.frac );
  ASSERT( false == result.negative );

  ASSERT( fixpoint_sub( &result, &objs->zero, &objs->one ) == RESULT_OK );
  ASSERT( 1 == result.whole );
  ASSERT( 0 == result.frac );
  ASSERT( true == result.negative );

  fixpoint_t neg_min = objs->min;
  fixpoint_negate( &neg_min );
  ASSERT( fixpoint_sub( &result, &neg_min, &objs->max ) == RESULT_OVERFLOW );
}

void test_mul( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  fixpoint_t result;

  ASSERT( fixpoint_mul( &result, &objs->one, &objs->zero ) == RESULT_OK );
  ASSERT( 0 == result.whole );
  ASSERT( 0 == result.frac );
  ASSERT( false == result.negative );

  ASSERT( fixpoint_mul( &result, &objs->one_and_one_half, &objs->one_hundred ) == RESULT_OK );
  ASSERT( 150 == result.whole );
  ASSERT( 0 == result.frac );
  ASSERT( false == result.negative );
}

void test_compare( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  ASSERT( 0 == fixpoint_compare( &objs->zero, &objs->zero ) );
  ASSERT( 1 == fixpoint_compare( &objs->one, &objs->zero ) );
  ASSERT( -1 == fixpoint_compare( &objs->zero, &objs->one ) );
  ASSERT( -1 == fixpoint_compare( &objs->neg_three_eighths, &objs->one_half ) );
  ASSERT( 1 == fixpoint_compare( &objs->one_half, &objs->neg_three_eighths ) );
}

void test_format_hex( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  fixpoint_str_t s;

  fixpoint_format_hex( &s, &objs->zero );
  ASSERT( 0 == strcmp( "0.0", s.str ) );

  fixpoint_format_hex( &s, &objs->one );
  ASSERT( 0 == strcmp( "1.0", s.str ) );

  fixpoint_format_hex( &s, &objs->one_half );
  ASSERT( 0 == strcmp( "0.8", s.str ) );

  fixpoint_format_hex( &s, &objs->max );
  ASSERT( 0 == strcmp( "ffffffff.ffffffff", s.str ) );

  fixpoint_format_hex( &s, &objs->neg_three_eighths );
  ASSERT( 0 == strcmp( "-0.6", s.str ) );

  fixpoint_format_hex( &s, &objs->min );
  ASSERT( 0 == strcmp( "0.00000001", s.str ) );

  fixpoint_format_hex( &s, &objs->one_and_one_half );
  ASSERT( 0 == strcmp( "1.8", s.str ) );

  fixpoint_format_hex( &s, &objs->one_hundred );
  ASSERT( 0 == strcmp( "64.0", s.str ) );

  fixpoint_format_hex( &s, &objs->neg_eleven );
  ASSERT( 0 == strcmp( "-b.0", s.str ) );
}

void test_parse_hex( TestObjs *objs ) {
  // Note: don't modify the provided test functions.
  // Instead, add new test functions containing your new tests.

  fixpoint_t val;

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "0.0" ) ) );
  TEST_EQUAL( &objs->zero, &val );

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "1.0" ) ) );
  TEST_EQUAL( &objs->one, &val );

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "0.8" ) ) );
  TEST_EQUAL( &objs->one_half, &val );

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "ffffffff.ffffffff" ) ) );
  TEST_EQUAL( &objs->max, &val );

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "-0.6" ) ) );
  TEST_EQUAL( &objs->neg_three_eighths, &val );

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "0.00000001" ) ) );
  TEST_EQUAL( &objs->min, &val );

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "1.8" ) ) );
  TEST_EQUAL( &objs->one_and_one_half, &val );

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "1.8" ) ) );
  TEST_EQUAL( &objs->one_and_one_half, &val );

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "64.0" ) ) );
  TEST_EQUAL( &objs->one_hundred, &val );

  ASSERT( true == fixpoint_parse_hex( &val, FIXPOINT_STR( "-b.0" ) ) );
  TEST_EQUAL( &objs->neg_eleven, &val );

  // Note: this test function doesn't have any tests for invalid
  // hex strings. You should add tests for invalid strings in one of
  // your own test functions.
}

// TODO: define additional test functions

void test_init_2( TestObjs *objs ) {

  fixpoint_t val;

  // test max 32bit value
  fixpoint_init( &val, 0xFFFFFFFF, 0xFFFFFFFF, false );
  ASSERT( val.whole == 0xFFFFFFFF );
  ASSERT( val.frac == 0xFFFFFFFF );
  ASSERT( val.negative == false );
}

void test_get_whole_2( TestObjs *objs ) {
  ASSERT( fixpoint_get_whole(&objs->neg_one_fourth) == 0);
  ASSERT( fixpoint_get_whole(&objs->random_pattern) == 305419878);
  ASSERT( fixpoint_get_whole(&objs->one_third) == 1431655765);
  ASSERT( fixpoint_get_whole( &objs->mid) == 2147483648);
}

void test_get_frac_2( TestObjs *objs ) {
  ASSERT( fixpoint_get_frac( &objs->neg_one_fourth) == 1073741824);
  ASSERT( fixpoint_get_frac( &objs->max_neg) == 4294967295);
  ASSERT( fixpoint_get_frac( &objs->one_third) == 1431655765);
  ASSERT( fixpoint_get_frac( &objs->random_pattern) == 2596069104);
  ASSERT( fixpoint_get_frac( &objs->mid) == 2147483648);
}

void test_is_negative_2( TestObjs *objs ) {
  ASSERT( fixpoint_is_negative( &objs->neg_one_fourth) == true);
  ASSERT( fixpoint_is_negative( &objs->max_neg) == true);
  ASSERT( fixpoint_is_negative( &objs->one_third) == false);
  ASSERT( fixpoint_is_negative( &objs->random_pattern) == false);
  ASSERT( fixpoint_is_negative( &objs->mid) == false);
}

void test_negate_2( TestObjs *objs ) {

  fixpoint_t result;

  result = objs->neg_one_fourth;
  fixpoint_negate( &result );
  ASSERT( result.whole == objs->neg_one_fourth.whole );
  ASSERT( result.frac == objs->neg_one_fourth.frac );
  ASSERT( false == result.negative );

  result = objs->neg_one_fourth;
  fixpoint_negate( &result );
  ASSERT( result.whole == objs->neg_one_fourth.whole );
  ASSERT( result.frac == objs->neg_one_fourth.frac );
  ASSERT( false == result.negative );

  result = objs->one_third;
  fixpoint_negate( &result );
  ASSERT( result.whole == objs->one_third.whole );
  ASSERT( result.frac == objs->one_third.frac );
  ASSERT( true == result.negative );
}

void test_add_2( TestObjs *objs ) {

  fixpoint_t result;

  ASSERT( fixpoint_add( &result, &objs->mid, &objs->neg_one_fourth ) == RESULT_OK );
  ASSERT( result.frac == 0x40000000 );
  ASSERT( result.whole == 0x80000000 );
  ASSERT( false == result.negative );

  ASSERT( fixpoint_add( &result, &objs->random_pattern, &objs->neg_one_fourth ) == RESULT_OK );
  ASSERT( result.frac == 0x5abcdef0 );
  ASSERT( result.whole == 0x12345666 );
  ASSERT( false == result.negative );

  ASSERT( fixpoint_add( &result, &objs->mid, &objs->max) == RESULT_OVERFLOW );

  ASSERT( fixpoint_add( &result, &objs->max_neg, &objs->max) == RESULT_OK );
  ASSERT( result.frac == 0 );
  ASSERT( result.whole == 0 );
  ASSERT( false == result.negative );

  ASSERT( fixpoint_add( &result, &objs->max, &objs->neg_one_fourth ) == RESULT_OK );
  ASSERT( result.frac == 0xbfffffff );
  ASSERT( result.whole == 0xffffffff );
  ASSERT( false == result.negative );

  ASSERT( fixpoint_add( &result, &objs->mid, &objs->mid ) == RESULT_OVERFLOW );

  ASSERT( fixpoint_add( &result, &objs->max_neg, &objs->one_hundred_neg) == RESULT_OVERFLOW);
}

void test_sub_2( TestObjs *objs ) {

  fixpoint_t result;

  ASSERT( fixpoint_sub( &result, &objs->random_pattern, &objs->neg_one_fourth ) == RESULT_OK );
  ASSERT( result.frac == 0xdabcdef0 );
  ASSERT( result.whole == 0x12345666 );
  ASSERT( result.negative == false );

  ASSERT( fixpoint_sub( &result, &objs->neg_three_eighths, &objs->one_half ) == RESULT_OK );
  ASSERT( result.frac == 0xe0000000 );
  ASSERT( result.whole == 0x00000000 );
  ASSERT( result.negative == true );

  ASSERT( fixpoint_sub( &result, &objs->mid, &objs->max_neg) == RESULT_OVERFLOW );
}

void test_mul_2( TestObjs *objs ) {

  fixpoint_t result;

  ASSERT( fixpoint_mul( &result, &objs->zero, &objs->random_pattern ) == RESULT_OK );
  ASSERT( result.frac == 0x00000000 );
  ASSERT( result.whole == 0x00000000 );
  ASSERT( result.negative == false );

  ASSERT( fixpoint_mul( &result, &objs->mid, &objs->neg_one_fourth ) == RESULT_OK );
  ASSERT( result.frac == 0x20000000);
  ASSERT( result.whole == 0x20000000);
  ASSERT( result.negative == true);

  ASSERT( fixpoint_mul( &result, &objs->min, &objs->one_half ) == RESULT_UNDERFLOW );
  ASSERT( fixpoint_mul( &result, &objs->min, &objs->min ) == RESULT_UNDERFLOW );

  ASSERT( fixpoint_mul( &result, &objs->max, &objs->max ) == RESULT_OVERFLOW );
  ASSERT( fixpoint_mul( &result, &objs->mid, &objs->max ) == RESULT_OVERFLOW );

}

void test_compare_2( TestObjs *objs ) {

  ASSERT( fixpoint_compare(&objs->max, &objs->max ) == 0 );
  ASSERT( fixpoint_compare(&objs->max, &objs->mid) == 1 );
  ASSERT( fixpoint_compare(&objs->mid, &objs->max) == -1 );
  ASSERT( fixpoint_compare(&objs->random_pattern, &objs->max) == -1 );

}

void test_format_hex_2( TestObjs *objs ) {

  fixpoint_str_t s;

  TEST_FIXPOINT_INIT( &objs->neg_one_fourth, 0,  0x40000000u, true);

  fixpoint_format_hex(&s, &objs->random_pattern );
  ASSERT( strcmp("12345666.9abcdef", s.str) == 0 );

  fixpoint_format_hex(&s, &objs->random_pattern );
  ASSERT( strcmp("12345666.9abcdef", s.str) == 0 );

  fixpoint_format_hex(&s, &objs->one_third );
  ASSERT( strcmp("55555555.55555555", s.str) == 0 );

}

void test_parse_hex_2( TestObjs *objs ) {

  fixpoint_str_t s;

  fixpoint_format_hex(&s, &objs->max_neg);
  ASSERT( strcmp("-ffffffff.ffffffff", s.str) == 0);

  fixpoint_format_hex(&s, &objs->one_third);
  ASSERT( strcmp("55555555.55555555", s.str) == 0 );

  fixpoint_format_hex(&s, &objs->random_pattern);
  ASSERT( strcmp("12345666.9abcdef", s.str) == 0 );

  fixpoint_format_hex(&s, &objs->mid);
  ASSERT( strcmp("80000000.8", s.str) == 0 );

}