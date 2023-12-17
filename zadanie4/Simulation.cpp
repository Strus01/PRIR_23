/*
 * Simulation.cpp
 *
 *  Created on: 10 gru 2023
 *      Author: oramus
 */

#include "Simulation.h"
#include <math.h>
#include <iostream>

using namespace std;

Simulation::Simulation(Force *_force, double _dt, bool _molecularStatic) {
	force = _force;
	particles = { };
	x = y = m = Fx = Fy = Vx = Vy = { };

	dt = _dt;
	dt_2 = dt / 2.0;
	molecularStatic = _molecularStatic;
}

void Simulation::initialize(DataSupplier *supplier) {
	particles = supplier->points();
	allocateMemory();

	for (int idx = 0; idx < particles; idx++) {
		x[idx] = supplier->x(idx);
		y[idx] = supplier->y(idx);
		m[idx] = supplier->m(idx);
		Vx[idx] = Vy[idx] = Fx[idx] = Fy[idx] = { 0.0 };
	}
}

void Simulation::allocateMemory() {
	x = new double[particles];
	y = new double[particles];
	m = new double[particles];
	Fx = new double[particles];
	Fy = new double[particles];
	Vx = new double[particles];
	Vy = new double[particles];
}

void Simulation::step() {
	updateVelocity();
	updatePosition();
	if (molecularStatic)
		preventMoveAgainstForce();
}

void Simulation::updateVelocity() {

	double oldFx, oldFy;
	double dx, dy, distance, frc;

	for (int idx = 0; idx < particles; idx++) {
		oldFx = Fx[idx];
		oldFy = Fy[idx];
		Fx[idx] = Fy[idx] = 0.0;
		for (int idx2 = 0; idx2 < idx; idx2++) {
			dx = x[idx2] - x[idx];
			dy = y[idx2] - y[idx];

			distance = sqrt(dx * dx + dy * dy);

			frc = force->value(distance);

			Fx[idx] += frc * dx / distance;
			Fy[idx] += frc * dy / distance;
		}

		for (int idx2 = idx+1; idx2 < particles; idx2++) {
			dx = x[idx2] - x[idx];
			dy = y[idx2] - y[idx];

			distance = sqrt(dx * dx + dy * dy);

			frc = force->value(distance);

			Fx[idx] += frc * dx / distance;
			Fy[idx] += frc * dy / distance;
		}
		Vx[idx] += dt_2 * (Fx[idx] + oldFx) / m[idx];
		Vy[idx] += dt_2 * (Fy[idx] + oldFy) / m[idx];
	}
}

void Simulation::updatePosition() {
	for (int idx = 0; idx < particles; idx++) {
		x[idx] += dt * (Vx[idx] + dt_2 * Fx[idx] / m[idx]);
		y[idx] += dt * (Vy[idx] + dt_2 * Fy[idx] / m[idx]);
	}
}

void Simulation::preventMoveAgainstForce() {
	double dotProduct;
	for (int idx = 0; idx < particles; idx++) {
		dotProduct = Vx[idx] * Fx[idx] + Vy[idx] * Fy[idx];
		if (dotProduct < 0.0) {
			Vx[idx] = Vy[idx] = { 0.0 };
		}
	}
}

double Simulation::Ekin() {
	double ek = 0.0;

	for (int idx = 0; idx < particles; idx++) {
		ek += m[idx] * (Vx[idx] * Vx[idx] + Vy[idx] * Vy[idx]) * 0.5;
	}

	return ek;
}

void Simulation::pairDistribution(double *histogram, int size, double coef) {
	for (int i = 0; i < size; i++)
		histogram[i] = 0;

	const double maxDistanceSQ = size * coef * size * coef;
	double dx, dy;
	double distance;
	int idx;

	for (int idx1 = 0; idx1 < particles; idx1++) {
		for (int idx2 = 0; idx2 < idx1; idx2++) {
			dx = x[idx2] - x[idx1];
			dy = y[idx2] - y[idx1];
			distance = dx * dx + dy * dy;
			if (distance < maxDistanceSQ) {
				distance = sqrt(distance);
				idx = (int) (distance / coef);
				histogram[idx]++;
			}
		}
	}

	for (int i = 0; i < size; i++) {
		distance = (i + 0.5) * coef;
		histogram[i] *= 1.0 / (2.0 * M_PI * distance * coef);
	}
}

double Simulation::avgMinDistance() {
	double sum = { };

	for (int i = 0; i < particles; i++)
		sum += minDistance(i);

	return sum / particles;
}

double Simulation::minDistance(int idx) {
	double dSqMin = 10000000.0;
	double dx, dy, distanceSQ;

	double xx = x[idx];
	double yy = y[idx];

	for (int i = 0; i < idx; i++) {
		dx = xx - x[i];
		dy = yy - y[i];
		distanceSQ = dx * dx + dy * dy;
		if (distanceSQ < dSqMin)
			dSqMin = distanceSQ;
	}
	for (int i = idx + 1; i < particles; i++) {
		dx = xx - x[i];
		dy = yy - y[i];
		distanceSQ = dx * dx + dy * dy;
		if (distanceSQ < dSqMin)
			dSqMin = distanceSQ;
	}

	return sqrt(dSqMin);
}

Simulation::~Simulation() {
}

