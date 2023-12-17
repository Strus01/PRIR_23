/*
 * SimpleDataSupplier.cpp
 *
 *  Created on: 11 gru 2023
 *      Author: oramus
 */

#include "SimpleDataSupplier.h"
#include <iostream>

using namespace std;

SimpleDataSupplier::SimpleDataSupplier( int _pointsSqrt, double _distance, double _mass )  : DataSupplier( _pointsSqrt * _pointsSqrt ),
distance( _distance ), pointsSqrt( _pointsSqrt ), mass( _mass ) {
}

void SimpleDataSupplier::setData() {
	int idx {};
	for ( int i = 0; i < pointsSqrt; i++ )
		for ( int j = 0; j < pointsSqrt; j++ ) {
			xptr[idx] = j * distance;
			yptr[idx] = i * distance;
			mptr[idx] = mass;
			idx++;
		}
}

SimpleDataSupplier::~SimpleDataSupplier() {
}

