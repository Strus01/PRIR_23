/*
 * MyForce.h
 *
 *  Created on: 10 gru 2023
 *      Author: oramus
 */

#ifndef MYFORCE_H_
#define MYFORCE_H_

#include"Force.h"

class MyForce : public Force {
private:
	static constexpr double AMPLITUDE {1.0};
	static constexpr double DECAY {0.3};
	static constexpr double LENGTH {5.0/3.141592};
public:
	MyForce();

	double value( double x );

	virtual ~MyForce();
};

#endif /* MYFORCE_H_ */
