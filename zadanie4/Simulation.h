/*
 * Simulation.h
 *
 *  Created on: 10 gru 2023
 *      Author: oramus
 */

#ifndef SIMULATION_H_
#define SIMULATION_H_

#include"Force.h"
#include"DataSupplier.h"

class Simulation {
private:
	double *x;
	double *y;
	double *m;
	double *Vx;
	double *Vy;
	double *Fx;
	double *Fy;
	int particles;

	double dt;
	double dt_2;
	bool molecularStatic;
	Force *force;

	void allocateMemory();

	void updateVelocity();
	void updatePosition();
	void preventMoveAgainstForce();
	double minDistance( int idx );

public:
	Simulation(Force *force, double dt, bool molecularStatic);

	void step();

	void pairDistribution(double *histogram, int size, double coef);

	double Ekin();
	double avgMinDistance();

	void initialize(DataSupplier *supplier);
	virtual ~Simulation();
};

#endif /* SIMULATION_H_ */
