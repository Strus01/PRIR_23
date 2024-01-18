/*
 * DataSupplier.h
 *
 *  Created on: 11 gru 2023
 *      Author: oramus
 */

#ifndef DATASUPPLIER_H_
#define DATASUPPLIER_H_

class DataSupplier {
protected:
	const int pts;
	double *xptr;
	double *yptr;
	double *mptr;

	virtual void setData() = 0;
public:
	DataSupplier(int points);

	void initializeData();

	int points();

	double x(int i);
	double y(int i);
	double m(int i);

	virtual ~DataSupplier();
};

#endif /* DATASUPPLIER_H_ */
