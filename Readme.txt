This repository contains the code for all algorithms discussed in the paper "Exact algorithms for a personal scheduling problem in intra-hospital patient transfer with hierarchical objectives" by Marco Claps, Maxence Delorme, Manuel Iori, and Giorgio Zucchi.

Our algorithms are coded in C++ and use the commercial solver Gurobi for the ILP models. 
The code is divided over 12 folders, each containing the code of one method. The different folders correspond to the following methods in our paper:
- 1_MODEL_V1					| Model M1 + (6)
- 1_MODEL_V2					| Model M1 + (9)
- 2_MODEL_V1					| Model M2 + (6)
- 2_MODEL_V2					| Model M2 + (9)
- 3_MODEL_V11					| Direct MP + (18)
- 3_MODEL_V11_BREAKS			| Direct MP + (18) to compute the results displayed in Table 9
- 3_MODEL_V11_IE				| Direct MP + (18) to compute the results displayed in Table 7
- 3_MODEL_V11_IS				| Direct MP + (18) to compute the results displayed in Table 8
- 3_MODEL_V12					| Direct MP + (21)
- 3_MODEL_V21					| Sequential MP + (18)
- 3_MODEL_V21_COUNT				| Sequential MP + (18) to compute the results displayed in Table 6 
- 3_MODEL_V21_MAX				| Sequential MP + (18) to compute the results displayed in Table 6 
- 3_MODEL_V22					| Sequential MP + (21)

Each folder contains the same substructure. For example, 1_MODEL_V1	contains the following files:
- helper_functions.cpp			| Contains a number of secondary functions (this file is usually the same for each subfolder)
- helper_functions.h			| The header file corresponding to helper_functions.cpp (this file is usually the same for each subfolder)
- main.cpp						| The front-end code for using the method  
- main.h						| The header file corresponding to main.cpp 
- model.cpp						| Contains the model of the tested method
- model.h						| The header file corresponding to model.cpp 
- makefile						| Used for compiling under linux (it needs to be updated by the user)

********
Once compiled, the following command can be used to run the algorithm:
	./PROGRAM "./PATH_INSTANCE" "NAME_INSTANCE" "./PATH_AND_NAME_OUTPUT_GENERAL" 
where
- PROGRAM is the name of the compiled software 
- ./PATH_INSTANCE is the relative path of the folder where the instance to solve is located
- NAME_INSTANCE is the name of the instance to solve
- ./PATH_AND_NAME_OUTPUT_GENERAL is the name of the file (together with its relative path) where performance metrics (such as the optimality status, the CPU time required, or the number of variables) are stored after solving an instance
********

Moreover, "_INPUT.rar" contains a txt-file for each of our test instances. There are 7 main folders, each corresponding to a different dataset:
- REAL		| A set of 16 real-world instances
- O1		| A set of 240 modified real-world instances created to measure the effect of increasing the average demand (Table 10)
- O2		| A set of 160 modified real-world instances created to measure the effect of increasing the granularity of the time horizon (Table 11)
- O3		| A set of 160 modified real-world instances created to measure the effect of shuffling the demand (Table 12)
- O4		| A set of 160 modified real-world instances created to measure the effect of reducing the number of worked hours per employee (Table 13)
- O5		| The set of 16 real-world instances adapted to measure the effect of considering a lunch break (to run with 3_MODEL_V11_BREAKS to obtain the right part of Table 9)
- O5P		| The set of 16 real-world instances adapted to measure the effect of considering a lunch break (to run with 3_MODEL_V11 to obtain the left part of Table 9)
Note that 80 instances of dataset O1 are re-used in Tables 10-12.

Each txt-file is structured as follows:
- the first line contains:
		- the number of employees |E|
		- the maximum number of shifts assigned to an employee s_max
		- the maximum number of time slots worked by an employee h_max
		- the minimum number of consecutive working time slots in a shift R_min
		- the maximum number of consecutive working time slots in a shift R_max
		- the minimum number of consecutive resting time slots between two shifts r_min
		- the number of rotations |D|
		- the number of time slots in a rotation |H|
- the remaining |D|.|H| lines all contain, for each time slot:
    	- the rotation index, the time slot index, and the demand

