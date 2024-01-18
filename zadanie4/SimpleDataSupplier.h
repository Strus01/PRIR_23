/*
 * SimpleDataSupplier.h
 *
 *  Created on: 11 gru 2023
 *      Author: oramus
 */

#ifndef SIMPLEDATASUPPLIER_H_
#define SIMPLEDATASUPPLIER_H_

#include "DataSupplier.h"

class SimpleDataSupplier: public DataSupplier {
private:
	const double distance;
	const int pointsSqrt;
	const double mass;
protected:
	void setData();
public:
	SimpleDataSupplier( int pointsSqrt, double distance, double mass );
	virtual ~SimpleDataSupplier();
};

#endif /* SIMPLEDATASUPPLIER_H_ */
