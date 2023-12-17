/*
 * DataSupplier.cpp
 *
 *  Created on: 11 gru 2023
 *      Author: oramus
 */

#include "DataSupplier.h"

DataSupplier::DataSupplier(int points) : pts( points ){
	xptr = new double[points];
	yptr = new double[points];
	mptr = new double[points];
}

int DataSupplier::points() {
	return pts;
}

double DataSupplier::x(int idx) {
	return xptr[idx];
}

double DataSupplier::y(int idx) {
	return yptr[idx];
}

double DataSupplier::m(int idx) {
	return mptr[idx];
}

void DataSupplier::initializeData() {
	setData();
}

DataSupplier::~DataSupplier() {
}
