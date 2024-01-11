/*
 * main.cpp
 *
 *  Created on: 12 gru 2023
 *      Author: oramus
 */

#include "Force.h"
#include "MyForce.h"
#include "Simulation.h"
#include "DataSupplier.h"
#include "SimpleDataSupplier.h"

#include <iostream>
#include <ctime>

using namespace std;


///////////////////////////////////////
//////// DANE DLA SYMULACJI ///////////
///////////////////////////////////////

constexpr int HISTOGRAM_SIZE = 20;
constexpr double HISTOGRAM_LENGTH_PER_BIN = 0.2;
constexpr double DT = 0.02;
constexpr double DISTANCE = 1.0;
constexpr double MASS = 1.0;
constexpr int STEPS = 400;
constexpr int REPORT_PERIOD = 1;
constexpr int PARTICLES_SQRT = 20;

void showReport(int i, Simulation *s, double *v);

int main(int argc, char **argv) {

    double *v = new double[HISTOGRAM_SIZE];

    Force *force = new MyForce();

    DataSupplier *supplier = new SimpleDataSupplier(PARTICLES_SQRT, DISTANCE,
                                                    MASS);
    supplier->initializeData();

    clock_t start = clock();

    Simulation *simulation = new Simulation(force, DT, true);
    simulation->initialize(supplier);

    for (int step = 0; step < STEPS; step++) {
        if (step % REPORT_PERIOD == 0) {
            showReport(step, simulation, v);
        }
        simulation->step();
    }
    showReport(STEPS, simulation, v);

    clock_t end = clock();
    double duration = double(end - start) / CLOCKS_PER_SEC;

    cout << "Simulation time: " << duration << " seconds" << endl;
}

void showReport(int step, Simulation *s, double *v) {
    s->pairDistribution(v, HISTOGRAM_SIZE, HISTOGRAM_LENGTH_PER_BIN);
    s->Ekin();
    s->avgMinDistance();
    cout << "Step: " << step << " Ekin = " << s->Ekin()
    		<< " <min(NNdistance)> = " << s->avgMinDistance() << endl;
    for (int j = 0; j < HISTOGRAM_SIZE; j++) {
    	cout << "v[" << j << "] = " << v[j] << endl;
    }
}