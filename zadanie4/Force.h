/*
 * Force.h
 *
 *  Created on: 10 gru 2023
 *      Author: oramus
 */

#ifndef FORCE_H_
#define FORCE_H_

class Force {
public:
	Force();

	virtual double value( double distance ) = 0;
};

#endif /* FORCE_H_ */
