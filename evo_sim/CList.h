//
//  CList.hpp
//  variable fitness branching process
//
//  Created by Debra Van Egeren on 2/17/17.
//  Copyright © 2017 Debra Van Egeren. All rights reserved.
//

#ifndef clist_h
#define clist_h

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "Clone.h"
#include "main.h"

using namespace std;

class CList {
    friend class CellType;
    /* represents a population of cells to be simulated. encodes the mechanism for advancing the simulation forward in time (the simulation structure, cell type hierarchy, and cells to be simulated).
     */
protected:
    //these both will be NULL iff there are no cell types in the population.
    CellType *root;
    CellType *end_node;
    
    double d;
    double tot_rate;
    double time;
    int max_types;
    int num_types;
    bool death_var;
    bool recalc_birth;
    double prev_fit;
    double new_fit;
    int new_type;
    
    // stores pointers to CellTypes that have been initialized in this simulation run. may include extinct types. no Clones in the simulation should have a CellType not included in this vector.
    std::vector<CellType *> curr_types;
    // root types are CellTypes present at the start of the simulation. they will be roots of a phylogeny of types. this is distinct from the root clone of the CList- the root clone is just the start of the linked list containing all of the Clones.
    std::vector<CellType *> root_types;
    
    long long tot_cell_count;
    MutationHandler *mut_model;
    
    virtual Clone& chooseReproducer();
    Clone& chooseDead();
    Clone& chooseDeadVar(double total_death);
    void deleteList();
    void clearClones();
    virtual bool checkInit();
    virtual double nextEventTime();
    virtual void nextEventExecute();
    
    /* adds cells to population. should NOT be used when a new clone is added, only when cells are added to an existing clone.
     use insertNode if a new clone should be added.
     @param b TOTAL birth rate of new cells to be added
     @param num_cells number of cells to be added
     */
    void addCells(int num_cells, double b);
    
    /* removes EXACTLY ONE cell from the population
     */
    void removeCell(double b);
    
    void setEnd(CellType& new_end){
        end_node = &new_end;
    }
    void setRoot(CellType& new_root){
        root = &new_root;
    }
    double getTotalDeath();
    
    void killCell(Clone& dead);
    
    
public:
    CList();
    virtual ~CList(){}
    CList(double death, MutationHandler& mut_handle, int max);
    
    /* adds a new type to the simulation. type must not already be present in the simulation.
     */
    void insertCellType(CellType& new_type);
    
    /* advances the simulation by one event (birth or death). MODIFIES the CList and Clones inside.
     */
    virtual void advance();
    
    double getTotalBirth();
    
    /* get the index of a new cell type that doesn't conflict with the current typespace.
     SHOULD NEVER be called when their are no free types left (max_types = num_types).
     @return index of next free type.
     */
    int getNextType();
    
    int getMutType(){return new_type;}
    
    double getMotherBirth(){
        return prev_fit;
    }
    
    double getDaughterBirth(){
        return new_fit;
    }
    
    double getDeathRate(){
        return d;
    }
    
    CellType* getTypeByIndex(int i){
        return curr_types.at(i);
    }
    
    bool hasCellType(int i){
        try{
            CellType* test = curr_types.at(i);
            if (!test){
                return false;
            }
        }
        catch (std::out_of_range){
            return false;
        }
        return true;
    }
    
    double getCurrTime(){
        return time;
    }
    MutationHandler& getMutHandler(){
        return *mut_model;
    }
    
    void setMutHandler(MutationHandler& mut_handle){
        mut_model = &mut_handle;
    }
    
    void addRootType(CellType& new_root){
        root_types.push_back(&new_root);
    }
    std::vector<CellType *>& getRootTypes(){
        return root_types;
    }
    virtual void refreshSim();
    
    virtual bool isExtinct(){
        return tot_cell_count == 0;
    }
    
    int newestType();
    
    bool noTypesLeft(){
        return num_types == max_types;
    }
    
    void walkTypesAndWrite(ofstream& outfile);
    
    virtual bool handle_line(vector<string>& parsed_line);
    
    bool read_clones(ifstream& infile);
    
    long long getNumCells(){return tot_cell_count;}
    
    bool isOneType();
    
    int getMaxTypes(){
        return max_types - 1;
    }
    
    virtual void addMaleType(int type_index){};
    virtual void addFemaleType(int type_index){};
};

class MoranPop: public CList{
public:
    MoranPop();
    virtual void advance();
};

class PassagePop: public CList{
private:
    std::vector<double> frozen_passage_times;
    std::vector<int> frozen_passage_cellnums;
    std::queue<double> passage_times;
    std::queue<int> passage_cellnums;
    void clear_queue(std::queue<int> &q);
protected:
    bool checkInit();
    virtual void passage();
public:
    PassagePop();
    virtual void advance();
    virtual void refreshSim();
    bool handle_line(vector<string>& parsed_line);
};

class UpdateAllPop: public CList{
private:
    double timestep_length;
protected:
    bool checkInit();
public:
    UpdateAllPop();
    void advance();
    bool handle_line(vector<string>& parsed_line);
};

class SexReprPop: public CList{
private:
    std::vector<int> male_types;
    std::vector<int> female_types;
    bool is_extinct;
protected:
    SexReprClone& chooseReproducerVector(vector<int> possible_types);
    SexReprClone& chooseMother();
    SexReprClone& chooseFather();
    bool checkInit();
public:
    SexReprPop();
    void advance();
    bool isExtinct(){return is_extinct;};
    void addMaleType(int type_index);
    void addFemaleType(int type_index);
    void refreshSim();
    bool handle_line(vector<string>& parsed_line);
};

#endif /* clist_h */
