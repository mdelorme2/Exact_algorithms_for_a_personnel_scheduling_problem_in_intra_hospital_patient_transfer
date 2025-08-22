#include "main.h"

int main(int argc, char **argv){          

	// Read input and output paths
	double start = getCPUTime(); 	
	string path = argv[1];	
	string filein = argv[2];
	string pathAndFileout = argv[3];
	for(int i = 0; i < 5; i++){
		Solution sol; 
		
		// initialize the input variables from a file
		Instance inst = readInstance(path + filein, sol);
		inst.nbE += i;
		inst.print();

		// find the MODEL solution
		do{
			MODEL(inst,sol);
			SLAVE(inst,sol,sol.cuts.back());
		} while (sol.opt == 0);
		sol.time = getCPUTime() - start;	
		printSInfo(sol, inst);
		printInfo(pathAndFileout, sol, filein);
	}
}

