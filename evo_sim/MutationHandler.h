//
//  MutationHandler.hpp
//  evo_sim
//
//  Created by Debra Van Egeren on 2/23/17.
//  Copyright © 2017 Debra Van Egeren. All rights reserved.
//

#ifndef MutationHandler_h
#define MutationHandler_h

#include <stdio.h>
#include <vector>
#include <string>

class CellType;

using namespace std;


class MutationHandler {
    /* chooses new parameters (mutation prob, fitness, cell type) for a new cell type
     use: first call generateMutant, then get new parameters
     ABSTRACT CLASS
     */
protected:
    double birth_rate;
    double mut_prob;
    bool has_mutated;
    CellType *new_type;
    
    /* gets the desired mutant type, either the existing type or creates a new type.
     @param index desired index of new type. SHOULD NOT be outside the range 0<=index<max_types of clone_list
     @param clone_list the population in which the new type will ultimately reside
     @param curr_type the parent of the new cell
     @return a cell type appropriate for the typespace in clone_list with the desired index. MUST BE DELETED LATER.
     */
    CellType* getNewTypeByIndex(int index, CellType& curr_type);
public:
    MutationHandler();
    
    //@return birth rate of new type, after generateMutant
    double getNewBirthRate() {return birth_rate;}
    
    //@return mutation probability of new type, after generateMutant
    double getNewMutProb() {return mut_prob;}
    
    //@return CellType of new type, after generateMutant. should not conflict with current population typespace in clone_list.
    CellType& getNewType() {return *new_type;}
    
    bool has_mut() {return has_mutated;}
    
    void reset() {has_mutated = false;}
    
    /*
     loads parameters and calculates mutant daughter mutation rate, type, and birth rate
     @param type current type of mother cell
     @param b current birth rate of mother
     @param mut current mutation rate of mother
     */
    virtual void generateMutant(CellType& type, double b, double mut) = 0;
    virtual bool read(std::vector<string>& params) = 0;
};

class SexReprMutation: public MutationHandler{
public:
    SexReprMutation();
    void generateMutant(CellType& type, double b, double mut){};
    virtual void generateMutant(CellType& mother_type, CellType& father_type, double b, double mut)=0;
    virtual bool read(std::vector<string>& params) = 0;
};

class FathersCurseMutation: public SexReprMutation{
private:
    double f_AA;
    double f_Aa;
    double f_aa;
    double f_AA_y;
    double f_Aa_y;
    double f_aa_y;
    
    double autosome_mut;
    double y_mut;
    double male_prob;
public:
    FathersCurseMutation();
    void generateMutant(CellType& mother_type, CellType& father_type, double b, double mut);
    bool read(std::vector<string>& params);
};

class ThreeTypesMutation: public MutationHandler {
    /* only forward mutation, three types {0,1,2} in typespace, constant mutation rates between types
     additive fitness changes between types
     conceptually similar to 2-hit TS model
     type 2 absorbing state
     */
protected:
    double mu2;
    double fit1;
    double fit2;
public:
    ThreeTypesMutation(){};
    ThreeTypesMutation(double m2, double f1, double f2);
    virtual void generateMutant(CellType& type, double b, double mut);
    bool read(std::vector<string>& params);
};

class ThreeTypesFlexMutation: public MutationHandler {
protected:
    double mu2;
    //probability that type 0 will go directly to type 2
    double p1;
    double fit1;
    double fit2;
public:
    ThreeTypesFlexMutation(){};
    ThreeTypesFlexMutation(double m2, double f1, double f2, double pr1);
    virtual void generateMutant(CellType& type, double b, double mut);
    bool read(std::vector<string>& params);
};

class ManyTypesFlexMutation: public MutationHandler {
protected:
    double mu2;
    //probability that type 0 will go directly to type 2
    double p1;
    double fit1;
    double fit2;
    int num_types;
public:
    ManyTypesFlexMutation(){};
    ManyTypesFlexMutation(double m2, double f1, double f2, double pr1, int ntypes);
    virtual void generateMutant(CellType& type, double b, double mut);
    bool read(std::vector<string>& params);
};

class ThreeTypesMultMutation: public ThreeTypesMutation{
public:
    ThreeTypesMultMutation(){};
    virtual void generateMutant(CellType& type, double b, double mut);
};

class NeutralMutation: public MutationHandler{
public:
    NeutralMutation(){};
    void generateMutant(CellType& type, double b, double mut);
    bool read(std::vector<string>& params){return true;}
};

class DimReturnsUnifMutation: public MutationHandler{
private:
    double dim_rate;
    double max_gain;
public:
    DimReturnsUnifMutation(){};
    void generateMutant(CellType& type, double b, double mut);
    bool read(std::vector<string>& params);
};

class NoMutation: public MutationHandler{
public:
    NoMutation(){};
    bool read(std::vector<string>& params){return true;};
    void generateMutant(CellType& type, double b, double mut){
        throw "should not be mutating- no mutation in simulation";
    }
};

class FixedSitesMutation: public MutationHandler {
private:
    int getAdjIndex(int cell_type, int adj_type);
    int max_types;
    double* fitnesses;
    int* adj_mat;
    bool is_mult;
public:
    FixedSitesMutation();
    ~FixedSitesMutation();
    void generateMutant(CellType& type, double b, double mut);
    bool read(std::vector<string>& params);
};

class ParamDistMutation: public MutationHandler{
private:
    double drawLogNorm(double mean, double var);
    double drawNorm(double mean, double var);
    double drawGamma(double mean, double var);
    double drawDoubleExp(double mean, double var);
    double drawUnif(double low, double high);
    double param1;
    double param2;
    double zero_prob;
    bool is_fixed;
    string dist_type;
public:
    ParamDistMutation();
    void generateMutant(CellType& type, double b, double mut);
    bool read(std::vector<string>& params);
};

#endif /* MutationHandler_h */
