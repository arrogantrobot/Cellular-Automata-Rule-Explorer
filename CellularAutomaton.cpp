#include "CellularAutomaton.h"
#include <vector>
#include <iostream>
#include <math.h>
using namespace std;

//constructor - no parameters
CellularAutomaton::CellularAutomaton() {
    seeds = new vector<bool>;
    current = new vector<bool>;
    past = new vector<bool>;
    for (unsigned int i = 0; i < 8; i++) {
        mask.push_back(pow(2, i));
    }
}

//destructor
CellularAutomaton::~CellularAutomaton() {
    if(seeds) delete seeds;
    if(current) delete current;
    if(past) delete past;
}

//mutator function for the rule number
void CellularAutomaton::setRule(const int& ruleNum) {
    rule = ruleNum;
}

//re-initialize the cells
void CellularAutomaton::resetCells() {
    current->clear();
    current->insert(current->end(), seeds->begin(), seeds->end());
}

//pass the seed data to the cells and initialize them
void CellularAutomaton::seedCells(std::vector<bool>* seedData) {
    seeds->clear();
    width = seedData->size();
    seeds->insert(seeds->begin(), seedData->begin(), seedData->end());
}

//iterate the automaton
void CellularAutomaton::calcNewRow() {
    past->clear();
    //copy the current cell contents to the past buffer
    past->insert(past->begin(), current->begin(), current->end());
    int index = 0;

    //for each cell, determine the state of its neighborhood and then
    // set the new value of that cell
    for (unsigned int i = 0; i < width; i++) {
        index = 0;

        if (i == 0) {
            if ((*past)[width - 1]) {
                index += 1;
            }
        } else {
            if ((*past)[(i - 1) % width]) {
                index += 1;
            }
        }
        if ((*past)[i % width]) {
            index += 2;
        }

        if(i == (width - 1)) {
            if ((*past)[0]) {
                index += 4;
            }
        } else {
            if ((*past)[(i + 1) % width]) {
                index += 4;
            }
        }

        //"bitwise and" the rule number with the appropriate mask
        // to obtain the new value of cell i
        if (rule & mask[index]) {
            (*current)[i] = 1;
        } else {
            (*current)[i] = 0;
        }
    }
}

//accessor function for the current cell states
std::vector<bool> * CellularAutomaton::getNewRow() { 
    return current;
}

//used in debugging, this displays the cell states to stdout
void CellularAutomaton::display() {
    for (unsigned int i = 0; i < current->size(); i++) {
        if ((*current)[i]) {
            cout << ".";
        } else {
            cout << " ";
        }
    }
    cout << endl;
}

//accessor function for automaton width
unsigned int CellularAutomaton::getWidth() {
    return width;
}

//accessor function used to determine if the automaton has died out
// to either all black or all white cells
int CellularAutomaton::getRowSum() {
    int count = 0;
    for (int i = 0; i < current->size(); i++) {
        if ((*current)[i]) {
            count++;
        }
    }
    return count;
}
