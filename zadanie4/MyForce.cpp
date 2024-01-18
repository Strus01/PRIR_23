/*
 * MyForce.cpp
 *
 *  Created on: 10 gru 2023
 *      Author: oramus
 */

#include "MyForce.h"
#include <math.h>

MyForce::MyForce() {
}

MyForce::~MyForce() {
}

double MyForce::value( double x ) {
	return -AMPLITUDE * exp( -x * x * DECAY ) * cos( x / LENGTH );
}
