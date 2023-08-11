#pragma once
#include<bits/stdc++.h>
#include "regMgmt.hh"
#include "symtab.hh"
#include "ast.hh"
using namespace std;

extern SymbTab gst;
extern map<string, SymbTab*> flocal_vars;
extern map<string, SymbTab*> slocal_vars;
extern RegisterDescriptor* regDsp;

int getRetOff(string fname){
  SymbTab* symtab = flocal_vars[fname];
  int off = 8;
  for(map<string,SymbTabEntry*>::iterator itr=symtab->stes.begin();itr!=symtab->stes.end();itr++){
    off = max(itr->second->offset, off);
  }
  return off;
}

int getStructIdOff(string curr_struct, string id){
  SymbTab* symtab = slocal_vars[curr_struct];
  if(symtab->stes[id]) return (symtab->stes[id]->offset+symtab->stes[id]->size)*(-1);
  // return (symtab->stes[id]->offset+symtab->stes[id]->size)*(-1);
  return 0;
  // exit(1);
}

int getOffTot(struct SynAtr syn, int off_base, AddressDescriptor* addDsp){
  string name = syn.var;
  return off_base + addDsp->getOffsetMem(name);
}

DataType getIdDt(string name, string id, bool isFunc){
  SymbTab* symtab;
  DataType dt;
  if(isFunc){
    symtab = flocal_vars[name];
  }else{
    symtab = slocal_vars[name];
  }
  if(symtab->stes[id] != NULL){
    dt = symtab->stes[id]->datatype;
  }
  return dt;
}

string genLabel(){
  static int num = 0;
  string label = ".LC"+to_string(num);
  num++;
  return label;
}

// temporaris in memeory
string newTemp(int sz, AddressDescriptor* addDsp){
  static int num = 0;
  string label = "#T"+to_string(num)+"x";
  num++;
  addDsp->addTemp(label, sz);
  return label;
}

string getTemp(){
  static int num = 0;
  string var = "!T"+to_string(num);
  num++;
  return var;
}

string getLealTemp(){
  static int num = 0;
  string var = "^T"+to_string(num);
  num++;
  return var;
}

// used in relOp
string newTempVar(){
  static int num = 0;
  string label = "@T"+to_string(num);
  num++;
  return label;
}

bool backpatch(deque<string>& code, vector<int>& list, string label){
  for(int i=0; i<(int)list.size(); i++){
    // cout<<code[list[i]]<<endl;
    code[list[i]] = code[list[i]]+"\t\t"+label+"\n";
  }
  return true;
}

bool backpatch(vector<string*> v_str, string label){
  for(int i=0; i<(int)v_str.size(); i++){
    *(v_str[i]) += label+"\n";
  }
  return true;
}

vector<string*> mergelist(vector<string*>& v1, vector<string*>& v2){
  vector<string*> v(v1.size()+v2.size());
  copy(v1.begin(), v1.end(), v.begin());
  copy(v2.begin(), v2.end(), v.begin()+v1.size());
  return v;
}

void merge(vector<string>& file, deque<string>& code){
  if(file.size() != 0) file.push_back("\n");
  for(int i=0; i<(int)code.size(); i++){
    file.push_back(code[i]);
  }
}

string getVar(struct SynAtr syn, RegisterDescriptor* regDsp, AddressDescriptor* addDsp){
  // if(syn.var.size() == 0) {
  //   cout<<"exit size 0";
  //   exit(1);
  // }
  if(isNum(syn.var)){
    string out = "$"+syn.var;
    return out;
  }
  if(syn.var[0] == '.') return "$"+syn.var; // this is for label 
  string reg = regDsp->find(syn.var);
  if(regDsp->find(syn.var) != "") return regDsp->find(syn.var);
  if(syn.var[0] == '#') return getMem(syn, addDsp);
  if(addDsp->findVar(syn.var) != "") return getMem(syn, addDsp);
  return "-8(%ebp)";
  // cout<<"error: "<<syn.var<<" not found\n";
  // exit(1);
}

int getSizeTS(string type){
  if(type == "void"){
    return 0;
  }else if(type == "int"){
    return 4;
  }else{
    return gst.find(type)->size;
  }
}

string getMem(string name, AddressDescriptor* addDsp){
  if(name.size() == 0) return "";
  return addDsp->findVar(name);
}

string getMem(struct SynAtr syn, AddressDescriptor* addDsp){
  string name = syn.var;
  if(name.size() == 0) return "";
  if(syn.offset > 0){
    return addDsp->findVarOff(name, syn.offset);
  }else{
    return addDsp->findVar(name);
  }
}


// freeReg
bool freeReg(string reg, RegisterDescriptor* regDsp){
  regDsp->live[reg] = false;
  regDsp->reg_to_var[reg] = false;
  return true;
}

// tbdl
string getReg(struct SynAtr syn, RegisterDescriptor* regDsp, AddressDescriptor* addDsp){
  return "";
}

//
bool isRelOp(string op){
  if(op == "EQ_OP_INT") return true;
  if(op == "NE_OP_INT") return true;
  if(op == "LE_OP_INT") return true;
  if(op == "LT_OP_INT") return true;
  if(op == "GE_OP_INT") return true;
  if(op == "GT_OP_INT") return true;
  if(op == "NE_OP_FLOAT") return true;
  if(op == "EQ_OP_FLOAT") return true;
  if(op == "LE_OP_FLOAT") return true;
  if(op == "LT_OP_FLOAT") return true;
  if(op == "GE_OP_FLOAT") return true;
  if(op == "GT_OP_FLOAT") return true;
  return false;
}

// 
string op_to_ins(string op, bool fall){
  if(op == "EQ_OP_INT"){
    if(fall){
      return "jne";
    }else{
      return "je";
    }
  }
  if(op == "NE_OP_INT"){
    if(fall){
      return "je";
    }else{
      return "jne";
    }
  }

  if(op == "LE_OP_INT"){
    if(fall){
      return "jg";
    }else{
      return "jle";
    }
  }
  if(op == "LT_OP_INT"){
    if(fall){
      return "jge";
    }else{
      return "jl";
    }
  }
  if(op == "GE_OP_INT"){
    if(fall){
      return "jl";
    }else{
      return "jge";
    }
  }
  if(op == "GT_OP_INT"){
    if(fall){
      return "jle";
    }else{
      return "jg";
    }
  }
  if(op == "PLUS_INT") return "addl";
  if(op == "MINUS_INT") return "subl";
  if(op == "MULT_INT") return "imull";
  if(op == "DIV_INT") return "div";
  return "addl";
}

//checks num
bool isNum(string s){
  int start;
  if((s[0] == '+' || s[0] == '-') && isdigit(s[1])){
    start = 1;
  }else{
    start = 0;
  }

  for(int i=start; i<(int)s.size(); i++){
    if(!isdigit(s[i])) return false;
  }
  return true;
}