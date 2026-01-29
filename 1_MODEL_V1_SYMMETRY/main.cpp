#include "main.h"

int main(int argc, char **argv){          

	// Read input and output paths
	double start = getCPUTime(); 	
	string path = argv[1];	
	string filein = argv[2];
	string pathAndFileout = argv[3];
	int rule = atoi(argv[4]);
	Solution sol; 
	
    // initialize the input variables from a file
    Instance inst = readInstance(path + filein, sol);
	inst.print();

    // find the MODEL solution
    MODEL(inst,sol,rule);
	sol.time = getCPUTime() - start;	
	printSInfo(sol, inst);
	printInfo(pathAndFileout, sol, filein);
}

