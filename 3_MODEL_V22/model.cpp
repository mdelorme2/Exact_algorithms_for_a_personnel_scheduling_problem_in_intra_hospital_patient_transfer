#include "model.h"

void MODEL(const Instance& inst, Solution& sol, const int& level) {
	
	// create a model
    GRBEnv env = GRBEnv();              	
    GRBModel model = GRBModel(env);         

    // declaration of the variables for the model
    vector<vector<GRBVar> > x (inst.shifts.size(), vector<GRBVar>(ceil(log2(inst.nbE)) + 1)); 
	vector<GRBVar> y(inst.nbHours * (1+inst.sdmx-inst.sdmn));
	GRBLinExpr obj = 0;
	GRBLinExpr obj2 = 0;
	
    // initizalization of the variables for the model
	for (int i = 0; i < inst.shifts.size(); i++) {
		for (int j = 0; j < ceil(log2(inst.nbE))+1; j++){
            x[i][j] = model.addVar(0, 1, 0, GRB_BINARY);
        }
    }
	if (level == 2){
		for (int i = 0; i < inst.nbHours * (1+inst.sdmx-inst.sdmn); i++) {
			y[i] = model.addVar(0, 1, 0, GRB_BINARY);		
		}
	}
    model.update();

    // create linear expressions
	vector<GRBLinExpr> wf(inst.nbTS, 0); 
	vector<GRBLinExpr> occupied (inst.shifts.size(),0); 
    GRBLinExpr totS = 0; 
	GRBLinExpr totH = 0;    
	vector<GRBLinExpr> sA(inst.nbHours * (1+inst.sdmx-inst.sdmn),0);
    
	for (int i = 0; i < inst.shifts.size(); i++) {
		for (int j = 0; j < ceil(log2(inst.nbE))+1; j++){
			totS += pow(2,j) * x[i][j];
			totH += pow(2,j) * x[i][j] * inst.shifts[i][inst.nbTS+1];
			if(level == 2) sA[inst.shifts[i][inst.nbTS]] += pow(2,j) * x[i][j];
			obj += pow(2,j) * x[i][j] * inst.shifts[i][inst.nbTS+1];
			for (int k = 0; k < inst.nbTS; k++) {
				if(inst.shifts[i][k] == 1){
					wf[k] += pow(2,j) * x[i][j];
				}
				if(inst.shifts[i][k] > 0){
					occupied[k] += pow(2,j) * x[i][j];
				}
			}
		}
	}
    model.update();

    // create constraints
	for (int k = 0; k < inst.nbTS; k++) {  										
		model.addConstr(wf[k] >= inst.demand[k]);
	}
	model.addConstr(totS <= inst.nbE * inst.maxS);
	model.addConstr(totH <= inst.nbE * inst.maxH);
	for (int k = 0; k < inst.nbTS; k++){
		model.addConstr(occupied[k] <= inst.nbE);
	}
	if (level == 2){
		model.addConstr(obj <= sol.UB.back());
		for (int i = 0; i < inst.shifts.size(); i++) {
			for (int j = 0; j < ceil(log2(inst.nbE))+1; j++){
				model.addConstr(x[i][j] <= y[inst.shifts[i][inst.nbTS]]);
			}
		}
		for (int i = 0; i < inst.nbHours * (1+inst.sdmx-inst.sdmn); i++) {
			obj2+= y[i];
		}
	}
	
	// no-good cuts
	for(int i = 0; i < sol.cuts.size();i++){
		GRBLinExpr LHS = 0;
		GRBLinExpr RHS = 0;
		for(int j = 0; j < sol.cuts[i].size();j++){
			LHS += x[sol.cuts[i][j][0]][sol.cuts[i][j][1]];
			RHS += 1;
		}
		model.addConstr(LHS <= RHS - 1);
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
	sol.LB.push_back(ceil(model.get(GRB_DoubleAttr_ObjBound) - EPSILON)); 
	
	// if a solution has been found
    if (model.get(GRB_IntAttr_SolCount) >= 1) { 		
		sol.UB.push_back(ceil(model.get(GRB_DoubleAttr_ObjVal) - EPSILON));
		if(level == 2){
			cout << "A solution of level 2 was found:" << endl;
			vector<vector<int> > cut;
			for (int i = 0; i < inst.shifts.size(); i++){
				int nb = 0;
				for (int j = 0; j < ceil(log2(inst.nbE))+1; j++){
					if(ceil(x[i][j].get(GRB_DoubleAttr_X) - EPSILON) == 1){
						nb += pow(2,j);
						cut.push_back({i,j});
					}
				}
				if(nb > 0) cout << nb << " times shift " << i << endl;
			}
			sol.cuts.push_back(cut);
		}
    }
	else{
		cout << "Solution with " << sol.UB.back() << " hours is infeasible, let's try " << sol.UB.back() + 1 << endl;
		sol.UB.back() += 1; sol.LB.back() += 1; 
	}
}

void SLAVE(const Instance& inst, Solution& sol, vector<vector<int> > cut) {
	
	cout << "Let's see if these shifts can be split over " << inst.nbE << " employees" << endl;
	
	// create the shift demand
	vector<int> demand (inst.shifts.size(),0);
	for (int i = 0; i < cut.size(); i++){
		demand[cut[i][0]] += pow(2,cut[i][1]);
	}

	// create a model
    GRBEnv env = GRBEnv();              	
    GRBModel model = GRBModel(env);         

    // declaration of the variables for the model
     vector<vector<GRBVar> > x (inst.nbE, vector<GRBVar>(inst.shifts.size())); 
	
    // initizalization of the variables for the model
    for (int i = 0; i < inst.nbE; i++) { 
		for (int j = 0; j < inst.shifts.size(); j++) {
			if(demand[j] > 0)
				x[i][j] = model.addVar(0, 1, 0, GRB_BINARY);
        }
    }
    model.update();

    // create linear expressions
	vector<vector<GRBLinExpr> > occupied (inst.nbE, vector<GRBLinExpr> (inst.nbTS,0)); 
    vector<GRBLinExpr> totS(inst.nbE, 0); 
	vector<GRBLinExpr> totH(inst.nbE, 0);    
	vector<GRBLinExpr> sC(inst.shifts.size(), 0); 
	for (int i = 0; i < inst.nbE; i++) { 
		for (int j = 0; j < inst.shifts.size(); j++) {
			if(demand[j] > 0){
				totS[i] += x[i][j];
				totH[i] += x[i][j] * inst.shifts[j][inst.nbTS+1];
				for (int k = 0; k < inst.nbTS; k++) {
					if(inst.shifts[j][k] > 0){
						occupied[i][k] += x[i][j];
					}
				}
				sC[j] += x[i][j];
			}
		}
    }   
    model.update();

    // create constraints								
	for (int k = 0; k < inst.shifts.size(); k++) {  										
		model.addConstr(sC[k] >= demand[k]);
	}
	for (int i = 0; i < inst.nbE; i++) { 
		model.addConstr(totS[i] <= inst.maxS);
		model.addConstr(totH[i] <= inst.maxH);
		for (int k = 0; k < inst.nbTS; k++){
			model.addConstr(occupied[i][k] <= 1);
		}
	}
    
	// change some settings
    model.getEnv().set(GRB_DoubleParam_MIPGap, 0);
    model.getEnv().set(GRB_IntParam_Threads, 1);
    model.getEnv().set(GRB_DoubleParam_TimeLimit, 3600);

    // find the optimal solution	
    model.optimize();

    // store the results in a Solution object     	
	sol.opt = 0;

	// if a solution has been found
    if (model.get(GRB_IntAttr_SolCount) >= 1) { 		
		cout << "Solution valid!" << endl;
		sol.opt = 1;
        // get worked hour for each employee for each item and count the number of shifts
		vector<int> nbSh (inst.nbHours * (1+inst.sdmx-inst.sdmn),0);
		for (int i = 0; i < inst.nbE; i++){
			vector<int> shift (inst.nbTS,0);
			int nbH = 0;
			int nbS = 0;
			for (int j = 0; j < inst.shifts.size(); j++){
				if(demand[j] > 0 && ceil(x[i][j].get(GRB_DoubleAttr_X) - EPSILON) == 1){
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
	else{
		cout << "Solution invalid!" << endl;
	}
}



