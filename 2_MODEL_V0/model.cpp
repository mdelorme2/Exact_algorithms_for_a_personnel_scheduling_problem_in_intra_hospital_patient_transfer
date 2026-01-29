#include "model.h"

void MODEL(const Instance& inst, Solution& sol, const int& level) {
	
	// create a model
    GRBEnv env = GRBEnv();              	
    GRBModel model = GRBModel(env);         

    // declaration of the variables for the model
    vector<vector<GRBVar> > x;	
	x.resize(inst.nbE, vector<GRBVar>(inst.shifts.size())); 
	vector<GRBVar> y(inst.nbHours * (1+inst.sdmx-inst.sdmn));
	GRBLinExpr obj = 0;
	GRBLinExpr obj2 = 0;
	
    // initizalization of the variables for the model
    for (int i = 0; i < inst.nbE; i++) { 
		for (int j = 0; j < inst.shifts.size(); j++) {
            x[i][j] = model.addVar(0, 1, 0, GRB_BINARY);
        }
    }
	if (level == 2){
		for (int i = 0; i < inst.nbHours * (1+inst.sdmx-inst.sdmn); i++) {
			y[i] = model.addVar(0, 1, 0, GRB_BINARY);		
		}
	}
    model.update();

	// compute big-M
	vector<int> bigM(inst.nbHours * (1+inst.sdmx-inst.sdmn),0);
	for (int j = 0; j < inst.shifts.size(); j++) {
		int maxV = 0;
		for (int k = 0; k < inst.nbTS; k++) {
			if(inst.shifts[j][k] == 1){
				maxV = max(maxV, inst.demand[k]);
			}
		}
		bigM[inst.shifts[j][inst.nbTS]] += maxV;
    }
	
    // create linear expressions
	vector<GRBLinExpr> wf(inst.nbTS, 0); 
	vector<vector<GRBLinExpr> > occupied (inst.nbE, vector<GRBLinExpr> (inst.nbTS,0)); 
    vector<GRBLinExpr> totS(inst.nbE, 0); 
	vector<GRBLinExpr> totH(inst.nbE, 0);    
	vector<GRBLinExpr> sA(inst.nbHours * (1+inst.sdmx-inst.sdmn),0);
    
	for (int i = 0; i < inst.nbE; i++) { 
		for (int j = 0; j < inst.shifts.size(); j++) {
			totS[i] += x[i][j];
			totH[i] += x[i][j] *inst.shifts[j][inst.nbTS+1];
			if(level == 2) sA[inst.shifts[j][inst.nbTS]] += x[i][j];
			obj += x[i][j] *inst.shifts[j][inst.nbTS+1];
			for (int k = 0; k < inst.nbTS; k++) {
				if(inst.shifts[j][k] == 1){
					wf[k] += x[i][j];
				}
				if(inst.shifts[j][k] > 0){
					occupied[i][k] += x[i][j];
				}
			}
		}
    }   
    model.update();

    // create constraints
	for (int k = 0; k < inst.nbTS; k++) {  										
		model.addConstr(wf[k] >= inst.demand[k]);
	}
	for (int i = 0; i < inst.nbE; i++) { 
		model.addConstr(totS[i] <= inst.maxS);
		model.addConstr(totH[i] <= inst.maxH);
		for (int k = 0; k < inst.nbTS; k++){
			model.addConstr(occupied[i][k] <= 1);
		}
	}
	if (level == 2){
		model.addConstr(obj <= sol.UB.back());
		for (int i = 0; i < inst.nbHours * (1+inst.sdmx-inst.sdmn); i++) {
			model.addConstr(sA[i] <= bigM[i] * y[i]);
			obj2+= y[i];
		}
	}

	// set the objective: minimize obj
    if(level == 1) model.setObjective(obj, GRB_MINIMIZE);
	if(level == 2) model.setObjective(obj2, GRB_MINIMIZE);
    
	// change some settings
    model.getEnv().set(GRB_DoubleParam_MIPGap, 0);
    model.getEnv().set(GRB_IntParam_Threads, 1);
    model.getEnv().set(GRB_DoubleParam_TimeLimit, 3600);

    // find the optimal solution	
    model.optimize();

    // store the results in a Solution object
    sol.Nvar = model.get(GRB_IntAttr_NumVars);       
    sol.Nconstr = model.get(GRB_IntAttr_NumConstrs); 
    sol.Ncoeff = model.get(GRB_IntAttr_NumNZs);      	
	sol.opt = 0;
	sol.LB.push_back(ceil(model.get(GRB_DoubleAttr_ObjBound) - EPSILON)); 
	
	// if a solution has been found
    if (model.get(GRB_IntAttr_SolCount) >= 1) { 		
		sol.UB.push_back(ceil(model.get(GRB_DoubleAttr_ObjVal) - EPSILON));
		if(sol.UB.back() == sol.LB.back()) sol.opt = 1;
		
        // get worked hour for each employee for each item and count the number of shifts
		if (level == 2){
			vector<int> nbSh (inst.nbHours * (1+inst.sdmx-inst.sdmn),0);
			for (int i = 0; i < inst.nbE; i++){
				vector<int> shift (inst.nbTS,0);
				int nbH = 0;
				int nbS = 0;
				for (int j = 0; j < inst.shifts.size(); j++){
					if(ceil(x[i][j].get(GRB_DoubleAttr_X) - EPSILON) == 1){
						nbSh[inst.shifts[j][inst.nbTS]] += 1;
						nbS += 1;
						nbH += inst.shifts[j][inst.nbTS+1];
						for (int k = 0; k < inst.nbTS; k++) {
							shift[k] += inst.shifts[j][k];
						}
					}
				}
				shift.push_back(nbH);
				shift.push_back(nbS);
				sol.isEwTS.push_back(shift);
			}
			int nbDiffS = 0;
			for(int i = 0; i<nbSh.size();i++){
				if(nbSh[i] > 0)
					nbDiffS += 1;
			}
			cout << "There are " << nbDiffS << " shift families used " << endl;
		}
    }
}

