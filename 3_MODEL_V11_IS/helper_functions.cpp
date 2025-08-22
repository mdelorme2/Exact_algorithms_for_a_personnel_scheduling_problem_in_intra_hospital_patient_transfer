#include "helper_functions.h"

void Instance::print(){
	cout << "nbE = " << nbE << " " << "maxS/maxH = " << maxS <<"/"<< maxH  << " sdmn/sdmx = " << sdmn <<"/"<< sdmx << " nbR = " << nbR << endl;
	for(int i=0; i<nbDays ;i++){
		for(int j=0; j < nbHours; j++){
			cout << i << "\t" << j << "\t" << demand[i*24+j] << endl;
		}
	}
	/*for(int i=0; i<shifts.size() ;i++){
		for(int j=0; j < shifts[i].size(); j++){
			cout << shifts[i][j]<< " ";
		}
		cout << endl;
	}*/
}

double getCPUTime(){
	return (double)clock() / CLOCKS_PER_SEC;
}

Instance readInstance(const string& filename, Solution& sol, const int& dat) {
    // define variables
	Instance inst;

    // open the file
    ifstream file(filename); 

    // read the file
    if (file.is_open()) { //if the file is open
        string line;

        // first line contains number of employees, maximum number of shifts, maximum working hours, min/max consecutive hours in a shift and resting period
		getline(file, line, '\t');
		inst.nbE = stoi(line);
		getline(file, line, '\t');
		inst.maxS = stoi(line);
		getline(file, line, '\t');
		inst.maxH = stoi(line);
		getline(file, line, '\t');
		inst.sdmn = stoi(line);
		inst.sdmn = dat;
		getline(file, line, '\t');
		inst.sdmx = stoi(line);
		getline(file, line, '\n');
		inst.nbR = stoi(line);

		// reshape the array
		inst.demand.resize(inst.nbTS,0);
		
        // the remaining lines contain the demand for each time slot
        for (int j = 0; j < inst.nbTS; j++) {
            getline(file, line, '\t');
            getline(file, line, '\t');
            getline(file, line, '\n');
            inst.demand[j] = stoi(line);
        }

        // close the file
        file.close(); 
		
		// build the table of shifts
		for(int i=0; i<inst.nbDays ;i++){
			for(int j=0; j < inst.nbHours; j++){
				for(int k=inst.sdmn; k<=inst.sdmx; k++){
					vector<int> shift (inst.nbTS,0);
					for(int l=0; l < k;l++){
						shift[(i*24+j+l) % inst.nbTS] = 1;
					}
					for(int l=0; l < inst.nbR;l++){
						shift[(i*24+j+k+l) % inst.nbTS] = 2;
					}					
					shift.push_back((k-inst.sdmn)*inst.nbHours + j);
					shift.push_back(k);
					inst.shifts.push_back(shift);
				}
			}
		}
    }
    else {
        // if the file cannot be opened: print error and return default Inst
        cout << "Unable to open file " << filename; 
    }
	
    return inst;
}

void printInfo(const string& pathAndFileout, const Solution& sol, const string& filein){
	string nameFile = pathAndFileout;
	std::ofstream file(nameFile.c_str(), std::ios::out | std::ios::app);
	file << filein << "\t"  << sol.isEwTS.size() << "\t"  << sol.opt << "\t" << sol.time << "\t";
	for(int i = 0; i < sol.LB.size(); i++){
		file << sol.LB[i] << "\t" << sol.UB[i] << "\t";
	}
	file << sol.Nvar << "\t" << sol.Nconstr << "\t" << sol.Ncoeff  << "\t" << sol.cuts.size() - 1<< "\t" << sol.valid << endl;
	file.close();
}

void printSInfo(Solution& sol, const Instance& inst){
	bool okay = true;
	for (int i = 0; i < sol.isEwTS.size();i++){
		for (int j = 0; j < sol.isEwTS[i].size();j++){
			cout << sol.isEwTS[i][j] << " ";
			if(sol.isEwTS[i][j] > 2 && j < inst.nbTS){ 
				okay = false;
				cout << "ERROR, employee " << i << " has incompatible shifts at TS " << j << endl;
			}
		}
		cout << endl;
		if(sol.isEwTS[i][inst.nbTS] > inst.maxH){
			okay = false;
			cout << "ERROR, employee " << i << " works " << sol.isEwTS[i][inst.nbTS] << " hours whereas the maximum is " << inst.maxH << endl;
		}
		if(sol.isEwTS[i][inst.nbTS + 1] > inst.maxS){
			okay = false;
			cout << "ERROR, employee " << i << " works " << sol.isEwTS[i][inst.nbTS] << " shifts whereas the maximum is " << inst.maxS << endl;
		}
	}
	sol.valid = okay;
}