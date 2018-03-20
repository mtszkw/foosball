//Example of usage catch.hpp library

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

int Factorial( int number ) 
{
    return number <= 1 ? number : Factorial( number - 1 ) * number;  // fail
//  return number <= 1 ? 1      : Factorial( number - 1 ) * number;  // pass
}

TEST_CASE( "Factorial of 0 is 1 (fail)", "[single-file]" ) 
{
    REQUIRE( Factorial(0) == 1 );
}