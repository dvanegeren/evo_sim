//
//  OutputWriter.hpp
//  evo_sim
//
//  Created by Debra Van Egeren on 2/21/17.
//  Copyright © 2017 Debra Van Egeren. All rights reserved.
//

#ifndef OutputWriter_h
#define OutputWriter_h

#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include "CList.h"

using namespace std;

class OutputWriter{
    /* writes results to files. can write/act after every timestep (call to CList::advance()) or only after each simulation instance.
     */
protected:
    string ofile_loc;
    string ofile_name;
    int sim_number;
public:
    virtual void finalAction(CList& clone_list) = 0;
    virtual void duringSimAction(CList& clone_list) = 0;
    virtual void beginAction(CList& clone_list) = 0;
    virtual bool readLine(vector<string>& parsed_line) = 0;
    void setSimNumber(int new_num){
        sim_number = new_num;
    }
    OutputWriter(string ofile);
    virtual ~OutputWriter() = 0;
};

class FinalOutputWriter: public virtual OutputWriter{
    /* only writes before or after the simulation is run. all writes are thread safe IF AND ONLY IF the write buffer is flushed at the end of each writing method.
     */
public:
    FinalOutputWriter(string ofile);
    virtual ~FinalOutputWriter() = 0;
    void duringSimAction(CList& clone_list){};
};

class DuringOutputWriter: public virtual OutputWriter{
    /* can update and/or write after every simulation timestep.
     SHOULD NOT write to files shared between multiple trials during the simulation- NOT thread safe. Use a simulation specific file for this purpose.
     */
protected:
    int writing_period;
    int last_written;
    bool shouldWrite(CList& clone_list);
    void resetWriter(){
        last_written = 0;
    };
public:
    DuringOutputWriter(string ofile, int period);
    DuringOutputWriter(string ofile);
    virtual ~DuringOutputWriter() = 0;
};

class IndexedWriter: public virtual OutputWriter{
protected:
    int index;
public:
    IndexedWriter(string ofile);
    IndexedWriter(string ofile, int i);
    virtual ~IndexedWriter() = 0;
    int getTypeIndex(){
        return index;
    }
};

template <typename WRITER_CLASS>
class AllTypesWriter: public OutputWriter {
private:
    vector<WRITER_CLASS *> writers;
    vector<string> params_line;
    void addWriter(CList& clone_list, WRITER_CLASS& new_writer, int idx);
public:
    ~AllTypesWriter();
    AllTypesWriter(string ofile);
    void finalAction(CList& clone_list);
    void duringSimAction(CList& clone_list);
    void beginAction(CList& clone_list);
    bool readLine(vector<string>& parsed_line);
};

class CountStepWriter: public IndexedWriter, public DuringOutputWriter{
private:
    ofstream outfile;
    int timestep;
public:
    CountStepWriter(string ofile);
    ~CountStepWriter();
    bool readLine(vector<string>& parsed_line);
    void finalAction(CList& clone_list);
    void duringSimAction(CList& clone_list);
    void beginAction(CList& clone_list);
    int getTypeIndex(){
        return index;
    }
};

class MotherDaughterWriter: public IndexedWriter, public DuringOutputWriter{
private:
    ofstream outfile;
public:
    MotherDaughterWriter(string ofile);
    ~MotherDaughterWriter();
    bool readLine(vector<string>& parsed_line);
    void finalAction(CList& clone_list);
    void duringSimAction(CList& clone_list);
    void beginAction(CList& clone_list);
    int getTypeIndex(){
        return index;
    }
};

class NumMutationsWriter: public IndexedWriter, public DuringOutputWriter{
private:
    ofstream outfile;
public:
    NumMutationsWriter(string ofile);
    ~NumMutationsWriter();
    bool readLine(vector<string>& parsed_line);
    void finalAction(CList& clone_list);
    void duringSimAction(CList& clone_list);
    void beginAction(CList& clone_list);
    int getTypeIndex(){
        return index;
    }
};

class TypeStructureWriter: public FinalOutputWriter{
private:
    ofstream outfile;
public:
    TypeStructureWriter(string ofile);
    ~TypeStructureWriter();
    void finalAction(CList& clone_list);
    void beginAction(CList& clone_list);
    bool readLine(vector<string>& parsed_line){return true;}
};

class CellCountWriter: public IndexedWriter, public DuringOutputWriter{
private:
    ofstream outfile;
public:
    ~CellCountWriter();
    CellCountWriter(string ofile, int period, int i, int sim);
    CellCountWriter(string ofile);
    void finalAction(CList& clone_list);
    void duringSimAction(CList& clone_list);
    void beginAction(CList& clone_list);
    bool readLine(vector<string>& parsed_line);
};

class FitnessDistWriter: public IndexedWriter, public DuringOutputWriter{
private:
    ofstream outfile;
    void write_dist(ofstream& outfile, CList& clone_list);
public:
    ~FitnessDistWriter();
    FitnessDistWriter(string ofile, int period, int i, int sim);
    FitnessDistWriter(string ofile);
    void finalAction(CList& clone_list);
    void duringSimAction(CList& clone_list);
    void beginAction(CList& clone_list);
    bool readLine(vector<string>& parsed_line);
};

class AllTypesWideWriter: public DuringOutputWriter{
private:
    ofstream outfile;
    void write_pop_line(ofstream& outfile, CList& clone_list);
public:
    ~AllTypesWideWriter();
    AllTypesWideWriter(string ofile, int period, int sim);
    AllTypesWideWriter(string ofile);
    void finalAction(CList& clone_list);
    void duringSimAction(CList& clone_list);
    void beginAction(CList& clone_list);
    bool readLine(vector<string>& parsed_line);
};

class MeanFitWriter: public IndexedWriter, public DuringOutputWriter{
private:
    ofstream outfile;
public:
    ~MeanFitWriter();
    MeanFitWriter(string ofile, int period, int i, int sim);
    MeanFitWriter(string ofile);
    void finalAction(CList& clone_list);
    void duringSimAction(CList& clone_list);
    void beginAction(CList& clone_list);
    bool readLine(vector<string>& parsed_line);
    int getTypeIndex(){
        return index;
    }
};

class TunnelWriter: public IndexedWriter, public DuringOutputWriter{
private:
    bool tunneled;
    ofstream outfile;
public:
    ~TunnelWriter();
    TunnelWriter(string ofile);
    void finalAction(CList& clone_list);
    void duringSimAction(CList& clone_list);
    void beginAction(CList& clone_list);
    bool readLine(vector<string>& parsed_line);
};

class IfType2Writer: public FinalOutputWriter{
private:
    ofstream outfile;
public:
    ~IfType2Writer();
    IfType2Writer(string ofile);
    void finalAction(CList& clone_list);
    void beginAction(CList& clone_list){};
    bool readLine(vector<string>& parsed_line){return true;}
};

class IfTypeWriter: public IndexedWriter, public FinalOutputWriter{
private:
    ofstream outfile;
public:
    ~IfTypeWriter();
    IfTypeWriter(string ofile);
    void finalAction(CList& clone_list);
    void beginAction(CList& clone_list);
    bool readLine(vector<string>& parsed_line);
};

class IsExtinctWriter: public FinalOutputWriter{
private:
    ofstream outfile;
public:
    ~IsExtinctWriter();
    IsExtinctWriter(string ofile);
    void finalAction(CList& clone_list);
    void beginAction(CList& clone_list){};
    bool readLine(vector<string>& parsed_line){return true;}
};

class EndTimeWriter: public FinalOutputWriter{
private:
    ofstream outfile;
public:
    ~EndTimeWriter();
    EndTimeWriter(string ofile);
    void finalAction(CList& clone_list);
    void beginAction(CList& clone_list){};
    bool readLine(vector<string>& parsed_line){return true;}
};

class EndPopWriter: public FinalOutputWriter{
private:
    ofstream outfile;
public:
    ~EndPopWriter();
    EndPopWriter(string ofile);
    void finalAction(CList& clone_list);
    void beginAction(CList& clone_list){};
    bool readLine(vector<string>& parsed_line){return true;}
};

class EndPopTypesWriter: public FinalOutputWriter{
private:
    ofstream outfile;
public:
    ~EndPopTypesWriter();
    EndPopTypesWriter(string ofile);
    void finalAction(CList& clone_list);
    void beginAction(CList& clone_list){};
    bool readLine(vector<string>& parsed_line){return true;}
};

class NewMutantWriter: public IndexedWriter, public DuringOutputWriter{
private:
    ofstream outfile;
    bool has_mutant;
    //vector<string> *to_write;
public:
    ~NewMutantWriter();
    NewMutantWriter(string ofile);
    void finalAction(CList& clone_list);
    void beginAction(CList& clone_list);
    void duringSimAction(CList& clone_list);
    bool readLine(vector<string>& parsed_line);
};

// AllTypesWriter template class implementations

template <class WRITER_CLASS> void AllTypesWriter<WRITER_CLASS>::addWriter(CList& clone_list, WRITER_CLASS& new_writer, int idx){
    new_writer.setSimNumber(sim_number);
    params_line.back() = to_string(idx);
    new_writer.readLine(params_line);
    writers.push_back(&new_writer);
    new_writer.beginAction(clone_list);
}

template <class WRITER_CLASS> AllTypesWriter<WRITER_CLASS>:: AllTypesWriter(string ofile): OutputWriter(ofile){}

template <class WRITER_CLASS> AllTypesWriter<WRITER_CLASS>:: ~AllTypesWriter(){
    vector<WRITER_CLASS *>().swap(writers);
}

template <class WRITER_CLASS> void AllTypesWriter<WRITER_CLASS>::beginAction(CList& clone_list){
    int type_index;
    vector<CellType *> root_types = clone_list.getRootTypes();
    for (vector<CellType *>::iterator it = root_types.begin(); it != root_types.end(); ++it){
        type_index = (*it)->getIndex();
        WRITER_CLASS *new_writer = new WRITER_CLASS(ofile_loc);
        addWriter(clone_list, *new_writer, type_index);
    }
}

template <class WRITER_CLASS> void AllTypesWriter<WRITER_CLASS>::duringSimAction(CList& clone_list){
    for (typename vector<WRITER_CLASS *>::iterator it = writers.begin(); it != writers.end(); ++it){
        (*it)->duringSimAction(clone_list);
    }
    
    if (writers.size() == (clone_list.getMaxTypes() - 1)){
        return;
    }
    
    vector<int> new_types = vector<int>();
    for (int i=0; i<clone_list.getMaxTypes(); i++){
        if (!clone_list.getTypeByIndex(i)){
            continue;
        }
        bool found = false;
        for (typename vector<WRITER_CLASS *>::iterator it = writers.begin(); it != writers.end(); ++it){
            found = found || ((*it)->getTypeIndex() == i);
        }
        if (!found){
            new_types.push_back(i);
        }
    }
    for (vector<int>::iterator it = new_types.begin(); it != new_types.end(); ++it){
        WRITER_CLASS *new_writer = new WRITER_CLASS(ofile_loc);
        addWriter(clone_list, *new_writer, (*it));
    }
}

template <class WRITER_CLASS> void AllTypesWriter<WRITER_CLASS>::finalAction(CList& clone_list){
    for (typename vector<WRITER_CLASS *>::iterator it = writers.begin(); it != writers.end(); ++it){
        (*it)->finalAction(clone_list);
    }
    vector<WRITER_CLASS *>().swap(writers);
}

template <class WRITER_CLASS> bool AllTypesWriter<WRITER_CLASS>::readLine(vector<string>& parsed_line){
    params_line = parsed_line;
    params_line.push_back("0");
    WRITER_CLASS *test = new WRITER_CLASS(ofile_loc);
    bool to_return = test->readLine(params_line);
    delete test;
    return to_return;
}


#endif /* OutputWriter_h */
