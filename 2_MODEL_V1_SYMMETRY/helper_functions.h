#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

using namespace std;
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <math.h> 
#include <fstream>
#include <sstream> 
#include <time.h>
#include <sys/time.h>

const double EPSILON = 0.00001; 	// small constant
const int M = 1000000;				// big constant

struct Instance
{
	int nbDays; 				// number of days per week
	int nbHours; 				// number of hours per day
	int nbTS;					// number of time slots
	int nbE;						// number of employees
	int maxH, maxS;					// maximum number of hours/shifts worked by an employee
	int sdmn, sdmx;					// minimum/maximum shift duration
	int nbR;						// number of resting hours between two shifts
	vector<int> demand;				// vector of demand
	vector<vector<int> > shifts;	// table of shifts
	void print();
};

struct Solution
{
	int opt, Nvar, Nconstr, Ncoeff, valid;
	vector<int> LB, UB;
	double time;
    vector<vector<int> > isEwTS;
};

double getCPUTime();
Instance readInstance(const string& filename, Solution& sol);
void printInfo(const string& pathAndFileout, const Solution& sol, const string& filein);
void printSInfo(Solution& sol, const Instance& inst);

#endif 