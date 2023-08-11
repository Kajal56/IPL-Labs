#pragma once
#include<bits/stdc++.h>
#include "dec.hh"
using namespace std;

class SymbTab;
class SymbTabEntry;

class SymbTab{
  public:
    map<string, SymbTabEntry*> stes;
    int numParams;
    map<int, string> params;
    SymbTab();
    void insert(string fname, SymbTabEntry* te);
    void insertParam(string fname, SymbTabEntry* te);    
    SymbTabEntry* find(string fname);
};

class SymbTabEntry{
  public:
    string name;
    string var_fun_type;
    string type_returnType;
    int size;
    int offset;
    string scope;
    SymbTab *symtab;
    DataType datatype;
    SymbTabEntry();
    SymbTabEntry(string name, string var_fun_type, string scope);
};

