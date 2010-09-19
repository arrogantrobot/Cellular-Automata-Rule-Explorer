#ifndef _CellularAutomaton_H
#define _CellularAutomaton_H

//use the STL vector
#include <vector>

/*Class Declaration CellularAutomaton

This class contains the cells of the one-dimensional
automaton. The number of cells is determined by the 
size of the vector of bools passed to seedCells().
The cells must be initialized by passing seed data
before the automaton can be used.

This automaton is based on Wolfram Notation for one
dimensional, binary automata. The rule may be from 0
to 255. */

class CellularAutomaton
{
	public:
		
		CellularAutomaton();
		~CellularAutomaton();
		void setRule(const int& ruleNum);
		void resetCells();
		void seedCells(std::vector<bool>* seedData);
		void display();
		void calcNewRow();
		int getRowSum();
		std::vector<bool>* getNewRow();
		unsigned int getWidth();
		
	private:
	
		std::vector<bool> 		*seeds,
								*past,
								*current;
		std::vector<int>		mask;
		unsigned int 			width,
								rule;
};


#endif
