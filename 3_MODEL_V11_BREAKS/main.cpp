#include "main.h"

int main(int argc, char **argv){          

	// Read input and output paths
	double start = getCPUTime(); 	
	string path = argv[1];	
	string filein = argv[2];
	string pathAndFileout = argv[3];
	Solution sol; 
	
    // initialize the input variables from a file
    Instance inst = readInstance(path + filein, sol);
	inst.print();

    // find the MODEL solution
    do{
		MODEL(inst,sol);
		SLAVE(inst,sol,sol.cuts.back());
	} while (sol.opt == 0 && getCPUTime() - start < 3600);
	sol.time = getCPUTime() - start;	
	if(sol.time < 3600)
		printSInfo(sol, inst);
	else{
		sol.LB.resize(0); sol.LB.push_back(0); sol.LB.push_back(0);
		sol.UB.resize(0); sol.UB.push_back(999); sol.UB.push_back(999);
		sol.opt = -1;
	}
	printInfo(pathAndFileout, sol, filein);
}

