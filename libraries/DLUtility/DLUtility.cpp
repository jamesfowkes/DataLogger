/*
 * DLUtility.cpp
 * 
 * A collection of generic utility routines
 *
 * Author: James Fowkes
 * Adapted from original software by Matt Little
 *
 * www.re-innovation.co.uk
 */

/*
 * Standard library includes
 */

#include <stdint.h>
#include <time.h>

#include "DLUtility.h"

// Converts a decimal to BCD (binary coded decimal)
char DecToBcd(char value){
  return (value / 10 * 16 + value % 10);
}