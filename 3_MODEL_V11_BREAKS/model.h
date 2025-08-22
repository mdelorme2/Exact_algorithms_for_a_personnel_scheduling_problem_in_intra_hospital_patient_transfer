#ifndef MODEL_H
#define MODEL_H

using namespace std;
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <math.h> 
#include "gurobi_c++.h"
#include "helper_functions.h"

void MODEL(const Instance& inst, Solution& sol);
void SLAVE(const Instance& inst, Solution& sol, vector<vector<int> > cut);
#endif 