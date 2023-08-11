#pragma once
#include "symtab.hh"

// SymTab
SymbTab::SymbTab(){
  this->numParams = 0;
}

void SymbTab::insert(string fname, SymbTabEntry* te){
  stes[fname] = te;
}
void SymbTab::insertParam(string fname, SymbTabEntry* te){
  stes[fname] = te;
  params[numParams] = fname;
  numParams++;
}

SymbTabEntry* SymbTab::find(string fname){
  if(stes.find(fname) != stes.end()){
    return stes[fname];
  }else{
    return NULL;
  }
}

// SymbTabEntry
SymbTabEntry::SymbTabEntry(){
  
}
SymbTabEntry::SymbTabEntry(string name, string var_fun_type, string scope){
  this->name = name;
  this->var_fun_type = var_fun_type;
  this->scope = scope;
  this->size = 0;
  this->offset = 0;
  this->symtab = NULL;
  this->type_returnType = "-";
}