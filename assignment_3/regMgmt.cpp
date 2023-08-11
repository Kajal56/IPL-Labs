#pragma once
#include<bits/stdc++.h>
#include "regMgmt.hh"
#include "symtab.hh"
#include "ast.hh"
using namespace std;

RegisterDescriptor::RegisterDescriptor(string context){
  this->context = context;
  this->regSt.push("%edx");
  this->regSt.push("%ecx");
  this->regSt.push("%ebx");
  this->regSt.push("%eax");
  this->live["%eax"] = false;
  this->live["%ebx"] = false;
  this->live["%ecx"] = false;
  this->live["%edx"] = false;
  this->numRegs = 4;
}

bool RegisterDescriptor::swapRegSt(){
  string t1 = this->regSt.top();
  this->regSt.pop();
  string t2 = this->regSt.top();
  this->regSt.pop();
  this->regSt.push(t1);
  this->regSt.push(t2);
  return true;
}

string RegisterDescriptor::popRegSt(){
  string topReg = this->regSt.top();
  this->regSt.pop();
  return topReg; 
}

bool RegisterDescriptor::pushRegSt(string tReg){
  this->regSt.push(tReg);
  return true;
}

string RegisterDescriptor::find(string var){
  for(unordered_map<string,string>::iterator it=reg_to_var.begin();it!=reg_to_var.end();it++){
    if(it->second == var) return it->first;
  }
  return "";
}

bool RegisterDescriptor::setLive(string liveReg){
  this->live[liveReg] = true;
  return true;
}

bool RegisterDescriptor::isLive(string liveReg){
  return this->live[liveReg];
}

bool RegisterDescriptor::freeTempRegs(){
  for(int i=0; i<(int)this->reg_names.size(); i++){
    if(this->reg_to_var[this->reg_names[i]] == ""){
      this->live[this->reg_names[i]] = false;
    }else if(this->reg_to_var[this->reg_names[i]][0] == '@'){
      this->live[this->reg_names[i]] = false;
      this->reg_to_var[this->reg_names[i]] = "";
    }
  }
  return true;
}

bool RegisterDescriptor::freeAllRegs(){
  for(unordered_map<string, string>::iterator i=this->reg_to_var.begin(); i!= this->reg_to_var.end(); i++){
    this->live[i->first] = false;
    this->reg_to_var[i->first] = "";
  }
  return true;
}

bool RegisterDescriptor::setRegToVar(string sReg, string var){
  this->live[sReg] = true;
  this->reg_to_var[sReg] = var;
  // this->var_to_reg[var] = tReg;
  return true;
}

bool RegisterDescriptor::freeReg(string fReg){
  this->live[fReg] = false;
  this->reg_to_var[fReg] = "";
  return true;
}

AddressDescriptor::AddressDescriptor(string context){
  this->context = context;
  this->espOff = 0;
}


bool AddressDescriptor::addTemp(string temp, DataType dt){
  this->espOff -= dt.size;
  this->temp_size[temp] = dt.size;
  this->temp_offset[temp] = this->espOff;
  return true;
}

bool AddressDescriptor::addTemp(string temp, int sz){
  this->espOff -= sz;
  this->temp_size[temp] = sz;
  this->temp_offset[temp] = this->espOff;
  return true;
}


bool AddressDescriptor::popTemp(string temp){
  this->espOff += temp_size[temp];
  this->temp_offset.erase(temp);
  this->temp_size.erase(temp);
  return true;
}


// bool AddressDescriptor::popTemp(string temp){
//   this->espOff -= 4;
//   this->temp_offset.erase(temp);
//   return true;
// }

string AddressDescriptor::findTemp(string temp){
  for(unordered_map<string,int>::iterator it=temp_offset.begin();it!=temp_offset.end();it++){
    if(it->first == temp) return it->second+"(%ebp)";
  }
  return "";
  // cout<<"error for :"<<temp<<"findTemp"<<endl;
  // exit(2);
}

string AddressDescriptor::findVar(string var){
  for(unordered_map<string,int>::iterator it=var_offset.begin();it!=var_offset.end();it++){
    if(it->first == var) {
      return to_string(var_offset[var])+"(%ebp)";
    }
  }
  for(unordered_map<string,int>::iterator it=temp_offset.begin();it!=temp_offset.end();it++){
    if(it->first == var){
      return to_string(temp_offset[var])+"(%ebp)";
    }
  }
  return "";
  // cout<<"error for :"<<var<<"findVar"<<endl;
  // exit(2);
}

int AddressDescriptor::getOffsetMem(string var){
  for(unordered_map<string,int>::iterator it=var_offset.begin();it!=var_offset.end();it++){
    if(it->first == var) {
      return var_offset[var];
    }
  }
  for(unordered_map<string,int>::iterator it=temp_offset.begin();it!=temp_offset.end();it++){
    if(it->first == var){
      return temp_offset[var];
    }
  }
  // cout<<"error for :"<<var<<endl;
  return 0;
  // exit(2);
}

string AddressDescriptor::findVarOff(string var, int off){
  for(unordered_map<string,int>::iterator it=var_offset.begin();it!=var_offset.end();it++){
    if(it->first == var) return to_string(off+var_offset[var])+"(%ebp)";
  }
  for(unordered_map<string,int>::iterator it=temp_offset.begin();it!=temp_offset.end();it++){
    if(it->first == var) return to_string(off+it->second)+"(%ebp)";
  }
  // cout<<"error for :"<<var<<"in addDsp"<<endl;
  return "-8(%ebp)";
  // exit(2);
}

bool AddressDescriptor::clearAllTemp(){
  for(unordered_map<string,int>::iterator it=var_offset.begin();it!=var_offset.end();it++){
    this->espOff += this->temp_size[it->first];
  }
  this->temp_size.clear();
  this->temp_offset.clear();
  return true;
}

bool RegisterDescriptor::reArgStack(){
  stack<string> newSt;
  vector<string> leftOutRegs;
  for(int i=0; i<(int)this->reg_names.size(); i++){
    if(this->reg_to_var[this->reg_names[i]] == "" || this->reg_to_var[this->reg_names[i]][0] == '@'){
      leftOutRegs.push_back(this->reg_names[i]);
    }else{
      newSt.push(this->reg_names[i]);
    }
  }
  for(int i=0; i<(int)leftOutRegs.size(); i++){
    newSt.push(leftOutRegs[i]);
  }
  this->regSt = newSt;
  return true;
}

bool RegisterDescriptor::reTopStack(string topReg){
  stack<string> tempRegs;
  bool inStack = false;
  // while(this->regSt.top() != topReg){
  //   tempRegs.push(this->regSt.top());
  //   this->regSt.pop();
  // }
  // if(this->regSt.top == topReg){
  //   inStack = true;
  //   this->regSt.pop();
  // }
  // while(tempRegs.size()){
  //   this->regSt.push(tempRegs.top());
  //   tempRegs.pop();
  // }
  // if(inStack){
  //   this->regSt.push(topReg);
  // }
  return inStack;
}

// string getMem(string name){
//   if(name.size() == 0) return "";
//   if(isdigit(name[0])) return "$"+stoi(name);
//   if(name[0] == '#') return temp[name];
//   return vars[name];
//   return "";
// }

// string getMem(struct SynAtr syn){
//   string name = syn.var;
//   if(name.size() == 0) return "";
//   if(isdigit(name[0])) return "$"+stoi(name);
//   if(name[0] == '#') return temp[name];
//   return vars[name];
//   return "";
// }