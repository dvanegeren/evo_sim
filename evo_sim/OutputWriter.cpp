//
//  OutputWriter.cpp
//  evo_sim
//
//  Created by Debra Van Egeren on 2/21/17.
//  Copyright © 2017 Debra Van Egeren. All rights reserved.
//

#include "OutputWriter.h"
#include "MutationHandler.h"
#include "main.h"
#include "Clone.h"
#include "CList.h"
#include <vector>
#include <string>
#include <fstream>
#include <cmath>

using namespace std;

OutputWriter::~OutputWriter(){}
FinalOutputWriter::~FinalOutputWriter(){}
IndexedWriter::~IndexedWriter(){}
DuringOutputWriter::~DuringOutputWriter(){}

OutputWriter::OutputWriter(string ofile){
    ofile_loc = ofile;
    sim_number = 1;
}

FinalOutputWriter::FinalOutputWriter(string ofile): OutputWriter(ofile) {}

IndexedWriter::IndexedWriter(string ofile, int i): OutputWriter(ofile) {
    index = i;
}

IndexedWriter::IndexedWriter(string ofile): OutputWriter(ofile) {
    index = -1;
}

DuringOutputWriter::DuringOutputWriter(string ofile, int period): OutputWriter(ofile) {
    last_written = 0;
    writing_period = period;
}

DuringOutputWriter::DuringOutputWriter(string ofile): OutputWriter(ofile) {
    last_written = 0;
    writing_period = 0;
}

bool DuringOutputWriter::shouldWrite(CList& clone_list){
    if (writing_period == 0){
        return true;
    }
    int floored_time = floor(clone_list.getCurrTime());
    if ((floored_time % writing_period == 0) && (floored_time != last_written)){
        last_written = floored_time;
        return true;
    }
    return false;
}

TypeStructureWriter::TypeStructureWriter(string ofile):OutputWriter(ofile), FinalOutputWriter(ofile){
    ofile_name = "type_tree.oevo";
}

void TypeStructureWriter::beginAction(CList &clone_list){
    string ofile_middle = "sim_"+to_string(sim_number);
    outfile.open(ofile_loc + ofile_middle + ofile_name, ios::app);
}

TypeStructureWriter::~TypeStructureWriter(){
    outfile.flush();
    outfile.close();
}

void TypeStructureWriter::finalAction(CList& clone_list){
    std::vector<CellType *> roots = clone_list.getRootTypes();
    for (int i=0; i<int(roots.size()); i++){
        clone_list.walkTypesAndWrite(outfile, *roots[i]);
    }
    outfile.flush();
    outfile.close();
    sim_number++;
}

CellCountWriter::CellCountWriter(string ofile, int period, int i, int sim):OutputWriter(ofile), IndexedWriter(ofile, i), DuringOutputWriter(ofile, period){
    ofile_name = "type_" + to_string(i) + ".oevo";
    sim_number = sim;
}

CellCountWriter::CellCountWriter(string ofile): OutputWriter(ofile), IndexedWriter(ofile), DuringOutputWriter(ofile){}

NumMutationsWriter::NumMutationsWriter(string ofile): OutputWriter(ofile), IndexedWriter(ofile), DuringOutputWriter(ofile){}

CountStepWriter::CountStepWriter(string ofile): OutputWriter(ofile), IndexedWriter(ofile), DuringOutputWriter(ofile){
    timestep = 0;
}

bool CountStepWriter::readLine(vector<string>& parsed_line){
    if (parsed_line.size() != 2){
        return false;
    }
    try{
        writing_period =stoi(parsed_line[0]);
        index =stoi(parsed_line[1]);
    }
    catch (...){
        return false;
    }
    ofile_name = "type_" + to_string(index) + ".oevo";
    return true;
}

bool NumMutationsWriter::readLine(vector<string>& parsed_line){
    if (parsed_line.size() != 1){
        return false;
    }
    try{
        index =stoi(parsed_line[0]);
    }
    catch (...){
        return false;
    }
    ofile_name = "type_" + to_string(index) + ".oevo";
    return true;
}

bool CellCountWriter::readLine(vector<string>& parsed_line){
    if (parsed_line.size() != 2){
        return false;
    }
    try{
        writing_period =stoi(parsed_line[0]);
        index =stoi(parsed_line[1]);
    }
    catch (...){
        return false;
    }
    ofile_name = "type_" + to_string(index) + ".oevo";
    return true;
}

void CellCountWriter::beginAction(CList& clone_list){
    string ofile_middle = "count_sim_"+to_string(sim_number);
    outfile.open(ofile_loc + ofile_middle + ofile_name, ios::app);
    outfile << "data for cell type " << index << " sim number " << sim_number << endl;
    if (clone_list.hasCellType(index)){
        outfile << clone_list.getCurrTime() << ", " << clone_list.getTypeByIndex(index)->getNumCells() << endl;
    }
}

void NumMutationsWriter::beginAction(CList& clone_list){
    string ofile_middle = "muts_sim_"+to_string(sim_number);
    outfile.open(ofile_loc + ofile_middle + ofile_name, ios::app);
    outfile << "data for cell type " << index << " sim number " << sim_number << endl;
}

void MotherDaughterWriter::beginAction(CList& clone_list){
    string ofile_middle = "mother_daughter_"+to_string(sim_number);
    outfile.open(ofile_loc + ofile_middle + ofile_name, ios::app);
    outfile << "data for cell type " << index << " sim number " << sim_number << endl;
}

void MotherDaughterWriter::duringSimAction(CList &clone_list){
    if (shouldWrite(clone_list) && clone_list.hasCellType(index) && clone_list.getTypeByIndex(index)->getNumCells() > 0){
        outfile << clone_list.getCurrTime() << ", " << clone_list.getMotherBirth() << ", " << clone_list.getDaughterBirth() << endl;
    }
}

void AllTypesWideWriter::write_pop_line(ofstream& outfile, CList& clone_list){
    outfile << clone_list.getCurrTime();
    for (int i=0; i<clone_list.getMaxTypes(); i++){
        if (clone_list.hasCellType(i)){
            outfile << ", " << clone_list.getTypeByIndex(i)->getNumCells();
        }
        else {
            outfile << ", " << 0;
        }
    }
    outfile << endl;
}

void AllTypesWideWriter::beginAction(CList& clone_list){
    string ofile_middle = "all_types_wide_"+to_string(sim_number);
    outfile.open(ofile_loc + ofile_middle + ".oevo", ios::app);
    write_pop_line(outfile, clone_list);
}

void AllTypesWideWriter::duringSimAction(CList &clone_list){
    if (shouldWrite(clone_list)){
        write_pop_line(outfile, clone_list);
    }
}

void AllTypesWideWriter::finalAction(CList& clone_list){
    write_pop_line(outfile, clone_list);
    outfile.flush();
    outfile.close();
    resetWriter();
}

void CountStepWriter::beginAction(CList& clone_list){
    string ofile_middle = "count_step_sim_"+to_string(sim_number);
    outfile.open(ofile_loc + ofile_middle + ofile_name, ios::app);
    outfile << "data for cell type " << index << " sim number " << sim_number << endl;
    outfile << timestep << ", " << clone_list.getTypeByIndex(index)->getNumCells() << endl;
}

CountStepWriter::~CountStepWriter(){
    outfile.flush();
    outfile.close();
}

NumMutationsWriter::~NumMutationsWriter(){
    outfile.flush();
    outfile.close();
}

CellCountWriter::~CellCountWriter(){
    outfile.flush();
    outfile.close();
}

void CountStepWriter::duringSimAction(CList& clone_list){
    timestep ++;
    if (shouldWrite(clone_list) && clone_list.getTypeByIndex(index)->getNumCells() > 0){
        outfile << timestep << ", " << clone_list.getTypeByIndex(index)->getNumCells() << endl;
    }
}

void CellCountWriter::duringSimAction(CList& clone_list){
    if (shouldWrite(clone_list) && clone_list.hasCellType(index) && clone_list.getTypeByIndex(index)->getNumCells() > 0){
        outfile << clone_list.getCurrTime() << ", " << clone_list.getTypeByIndex(index)->getNumCells() << endl;
    }
}

void NumMutationsWriter::duringSimAction(CList& clone_list){
    MutationHandler* temp_mut = &(clone_list.getMutHandler());
    if (shouldWrite(clone_list) && temp_mut->has_mut() && temp_mut->getNewType().getIndex() == index){
        outfile << clone_list.getCurrTime() << endl;
    }
}

void CellCountWriter::finalAction(CList& clone_list){
    if (clone_list.hasCellType(index)){
        outfile << clone_list.getCurrTime() << ", " << clone_list.getTypeByIndex(index)->getNumCells() << endl;
    }
    else{
        outfile << clone_list.getCurrTime() << ", " << 0 << endl;
    }
    outfile.flush();
    outfile.close();
    resetWriter();
}

void CountStepWriter::finalAction(CList& clone_list){
    outfile.flush();
    outfile.close();
    sim_number++;
}

void NumMutationsWriter::finalAction(CList& clone_list){
    outfile.flush();
    outfile.close();
    sim_number++;
}

void MotherDaughterWriter::finalAction(CList& clone_list){
    outfile.flush();
    outfile.close();
}

AllTypesWideWriter::AllTypesWideWriter(string ofile, int period, int sim): OutputWriter(ofile), DuringOutputWriter(ofile, period){
    sim_number = sim;
}

AllTypesWideWriter::AllTypesWideWriter(string ofile): OutputWriter(ofile), DuringOutputWriter(ofile){}

TunnelWriter::TunnelWriter(string ofile): OutputWriter(ofile), IndexedWriter(ofile), DuringOutputWriter(ofile){
    writing_period = 0;
    tunneled = true;
}

void TunnelWriter::beginAction(CList& clone_list){}

void TunnelWriter::duringSimAction(CList& clone_list){
    if (!clone_list.getTypeByIndex(index)){
        return;
    }
    if (clone_list.getTypeByIndex(index)->getNumCells() == clone_list.getNumCells()){
        tunneled = false;
    }
}

void TunnelWriter::finalAction(CList& clone_list){
    bool is_2 = clone_list.getTypeByIndex(2);
    is_2 = is_2 && (clone_list.getTypeByIndex(2)->getNumCells() == clone_list.getNumCells());
    tunneled = is_2 && tunneled;
    outfile << sim_number << ", " << tunneled << endl;
    outfile.flush();
    sim_number ++;
    tunneled = true;
}

bool TunnelWriter::readLine(vector<string>& parsed_line){
    if (parsed_line.size() != 1){
        return false;
    }
    try{
        index =stoi(parsed_line[0]);
    }
    catch (...){
        return false;
    }
    ofile_name = "type_" + to_string(index) + "_tunnel.oevo";
    outfile.open(ofile_loc + ofile_name, ios::app);
    return true;
}

bool AllTypesWideWriter::readLine(vector<string>& parsed_line){
    if (parsed_line.size() != 1){
        return false;
    }
    try{
        writing_period =stoi(parsed_line[0]);
    }
    catch (...){
        return false;
    }
    return true;
}

IsExtinctWriter::IsExtinctWriter(string ofile): OutputWriter(ofile), FinalOutputWriter(ofile){
    ofile_name = "extinction.oevo";
    outfile.open(ofile_loc+ofile_name, ios::app);
}

void IsExtinctWriter::finalAction(CList& clone_list){
    outfile << sim_number << ", " << clone_list.isExtinct() << endl;
    outfile.flush();
    sim_number++;
}

IsExtinctWriter::~IsExtinctWriter(){
    outfile.flush();
    outfile.close();
}

EndTimeWriter::EndTimeWriter(string ofile): OutputWriter(ofile), FinalOutputWriter(ofile){
    ofile_name = "end_time.oevo";
    outfile.open(ofile_loc+ofile_name, ios::app);
}

void EndTimeWriter::finalAction(CList& clone_list){
    outfile << sim_number << ", " << clone_list.getCurrTime() << endl;
    outfile.flush();
    sim_number++;
}

EndTimeWriter::~EndTimeWriter(){
    outfile.flush();
    outfile.close();
}

MotherDaughterWriter::~MotherDaughterWriter(){
    outfile.flush();
    outfile.close();
}

EndPopWriter::EndPopWriter(string ofile): OutputWriter(ofile), FinalOutputWriter(ofile){
    ofile_name = "end_pop.oevo";
    outfile.open(ofile_loc+ofile_name, ios::app);
}

EndPopTypesWriter::EndPopTypesWriter(string ofile): OutputWriter(ofile), FinalOutputWriter(ofile){
    ofile_name = "end_pop_types.oevo";
    outfile.open(ofile_loc+ofile_name, ios::app);
}

void EndPopWriter::finalAction(CList& clone_list){
    outfile << sim_number << ", " << clone_list.getNumCells() << endl;
    outfile.flush();
    sim_number++;
}

void EndPopTypesWriter::finalAction(CList& clone_list){
    outfile << sim_number << endl;
    for (int i=0; i<clone_list.getMaxTypes(); i++){
        if (clone_list.hasCellType(i)){
             outfile << i << ", " << clone_list.getTypeByIndex(i)->getNumCells() << endl;
        }
    }
    outfile << endl;
    outfile.flush();
}

EndPopWriter::~EndPopWriter(){
    outfile.flush();
    outfile.close();
}

EndPopTypesWriter::~EndPopTypesWriter(){
    outfile.flush();
    outfile.close();
}

IfType2Writer::IfType2Writer(string ofile): OutputWriter(ofile), FinalOutputWriter(ofile){
    ofile_name = "iftype2.oevo";
    outfile.open(ofile_loc+ofile_name, ios::app);
}

IfTypeWriter::IfTypeWriter(string ofile): OutputWriter(ofile), IndexedWriter(ofile), FinalOutputWriter(ofile){
    ofile_name = "iftype.oevo";
}

void IfTypeWriter::beginAction(CList &clone_list){
    string ofile_middle = "type_" + to_string(index) + "_";
    outfile.open(ofile_loc+ofile_name, ios::app);
}

void IfType2Writer::finalAction(CList& clone_list){
    bool is_2 = clone_list.getTypeByIndex(2);
    is_2 = is_2 && (clone_list.getTypeByIndex(2)->getNumCells() == clone_list.getNumCells());
    outfile << sim_number << ", " << is_2 << endl;
    outfile.flush();
}

void IfTypeWriter::finalAction(CList& clone_list){
    bool is_2 = clone_list.getTypeByIndex(index);
    is_2 = is_2 && (clone_list.getTypeByIndex(index)->getNumCells() == clone_list.getNumCells());
    outfile << sim_number << ", " << is_2 << endl;
    outfile.flush();
}

IfType2Writer::~IfType2Writer(){
    outfile.flush();
    outfile.close();
}

IfTypeWriter::~IfTypeWriter(){
    outfile.flush();
    outfile.close();
}

FitnessDistWriter::FitnessDistWriter(string ofile, int period, int i, int sim):OutputWriter(ofile), IndexedWriter(ofile, i), DuringOutputWriter(ofile, period){
    ofile_name = "type_" + to_string(i) + ".oevo";
    sim_number = sim;
}

MeanFitWriter::MeanFitWriter(string ofile, int period, int i, int sim):OutputWriter(ofile), IndexedWriter(ofile, i), DuringOutputWriter(ofile, period){
    ofile_name = "type_" + to_string(i) + ".oevo";
    sim_number = sim;
}

FitnessDistWriter::FitnessDistWriter(string ofile): OutputWriter(ofile), IndexedWriter(ofile), DuringOutputWriter(ofile){}

MeanFitWriter::MeanFitWriter(string ofile): OutputWriter(ofile), IndexedWriter(ofile), DuringOutputWriter(ofile){}

MotherDaughterWriter::MotherDaughterWriter(string ofile): OutputWriter(ofile), IndexedWriter(ofile), DuringOutputWriter(ofile){}

bool MotherDaughterWriter::readLine(vector<string>& parsed_line){
    if (parsed_line.size() != 2){
        return false;
    }
    try{
        writing_period =stoi(parsed_line[0]);
        index =stoi(parsed_line[1]);
    }
    catch (...){
        return false;
    }
    ofile_name = "type_" + to_string(index) + ".oevo";
    return true;
}

bool IfTypeWriter::readLine(vector<string>& parsed_line){
    if (parsed_line.size() != 1){
        return false;
    }
    try{
        index =stoi(parsed_line[0]);
    }
    catch (...){
        return false;
    }
    return true;
}

bool FitnessDistWriter::readLine(vector<string>& parsed_line){
    if (parsed_line.size() != 2){
        return false;
    }
    try{
        writing_period =stoi(parsed_line[0]);
        index =stoi(parsed_line[1]);
    }
    catch (...){
        return false;
    }
    ofile_name = "type_" + to_string(index) + ".oevo";
    return true;
}

bool MeanFitWriter::readLine(vector<string>& parsed_line){
    if (parsed_line.size() != 2){
        return false;
    }
    try{
        writing_period =stoi(parsed_line[0]);
        index =stoi(parsed_line[1]);
    }
    catch (...){
        return false;
    }
    ofile_name = "type_" + to_string(index) + ".oevo";
    return true;
}

void FitnessDistWriter::beginAction(CList& clone_list){
    string ofile_middle = "fit_sim_"+to_string(sim_number);
    outfile.open(ofile_loc + ofile_middle + ofile_name, ios::app);
    outfile << "data for cell type " << index << " sim number " << sim_number << endl;
    
}

void MeanFitWriter::beginAction(CList& clone_list){
    string ofile_middle = "mean_fit_sim_"+to_string(sim_number);
    outfile.open(ofile_loc + ofile_middle + ofile_name, ios::app);
    outfile << "data for cell type " << index << " sim number " << sim_number << endl;
}

FitnessDistWriter::~FitnessDistWriter(){
    outfile.flush();
    outfile.close();
}

MeanFitWriter::~MeanFitWriter(){
    outfile.flush();
    outfile.close();
}

TunnelWriter::~TunnelWriter(){
    outfile.flush();
    outfile.close();
}

AllTypesWideWriter::~AllTypesWideWriter(){
    outfile.flush();
    outfile.close();
}

void FitnessDistWriter::write_dist(ofstream& outfile, CList& clone_list){
    Clone *curr_clone = (clone_list.getTypeByIndex(index)->getRoot());
    while (curr_clone){
        int num_cells = curr_clone->getCellCount();
        for (int i=0; i<num_cells; i++){
            outfile << ", " << curr_clone->getBirthRate();
        }
        curr_clone = &(curr_clone->getNextWithinType());
    }
}

void FitnessDistWriter::duringSimAction(CList& clone_list){
    if (shouldWrite(clone_list) && clone_list.getTypeByIndex(index) && clone_list.getTypeByIndex(index)->getNumCells() > 0){
        outfile << clone_list.getCurrTime();
        write_dist(outfile, clone_list);
        outfile << endl;
    }
}

void MeanFitWriter::duringSimAction(CList& clone_list){
    if (shouldWrite(clone_list) && clone_list.getTypeByIndex(index) && clone_list.getTypeByIndex(index)->getNumCells() > 0){
        outfile << clone_list.getCurrTime() << ", ";
        outfile << (clone_list.getTypeByIndex(index)->getBirthRate())/clone_list.getTypeByIndex(index)->getNumCells() << endl;
    }
}

void MeanFitWriter::finalAction(CList& clone_list){
    outfile << clone_list.getCurrTime() << ", ";
    outfile.flush();
    outfile.close();
    resetWriter();
}

void FitnessDistWriter::finalAction(CList& clone_list){
    outfile.flush();
    outfile.close();
    resetWriter();
}

NewMutantWriter::NewMutantWriter(string ofile): OutputWriter(ofile), IndexedWriter(ofile), DuringOutputWriter(ofile){
    has_mutant = false;
}

bool NewMutantWriter::readLine(vector<string>& parsed_line){
    if (parsed_line.size() != 1){
        return false;
    }
    try{
        index =stoi(parsed_line[0]);
    }
    catch (...){
        return false;
    }
    ofile_name = "sim_num_" + to_string(sim_number) + "_new_mutant_" + to_string(index) + ".oevo";
    return true;
}

NewMutantWriter::~NewMutantWriter(){
    outfile.flush();
    outfile.close();
}

void NewMutantWriter::beginAction(CList& clone_list){
    ofile_name = "sim_num_" + to_string(sim_number) + "_new_mutant_" + to_string(index) + ".oevo";
    outfile.open(ofile_loc + ofile_name, ios::app);
    if (clone_list.hasCellType(index) && clone_list.getTypeByIndex(index)->getNumCells() > 0){
        has_mutant = true;
    }
    //to_write = new vector<string>();
}

void NewMutantWriter::finalAction(CList &clone_list){
    outfile.flush();
    outfile.close();
    //delete to_write;
    //to_write = NULL;
}

void NewMutantWriter::duringSimAction(CList &clone_list){
    if (clone_list.getMutType() == index){
        if (clone_list.getMutHandler().has_mut()){
            string new_line = to_string(sim_number) + ", " + to_string(clone_list.getCurrTime());
            new_line += ", " + to_string(clone_list.getDaughterBirth());
            new_line += ", " + to_string(clone_list.getTotalBirth());
            outfile << new_line << endl;
            has_mutant = true;
        }
    }
    else{
        has_mutant = false;
    }
}
