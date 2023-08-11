#pragma once
#include<bits/stdc++.h>
#include "ast.hh"
using namespace std;

class RegisterDescriptor{
  public:
  int numRegs;
  string context;
  vector<string> reg_names = {"%eax", "%ebx", "%ecx", "%edx"};
  unordered_map<string, string> reg_to_var;
  unordered_map<string, bool> live;
  stack<string> regSt;
  RegisterDescriptor(string context);
  bool setLive(string liveReg);
  bool isLive(string liveReg);
  bool freeTempRegs();
  bool freeAllRegs();
  bool swapRegSt();
  string popRegSt();
  bool pushRegSt(string tReg);
  string find(string name);
  bool setRegToVar(string sReg, string var);
  bool freeReg(string fReg);
  // not used for now
  bool reArgStack();
  bool reTopStack(string topReg);
};

class AddressDescriptor{
  public:
  int espOff;
  string context;
  unordered_map<string, int> var_offset;
  unordered_map<string, int> var_size;
  unordered_map<string, int> temp_offset;
  unordered_map<string, int> temp_size;
  AddressDescriptor(string context);
  string findTemp(string temp);
  string findVar(string var);
  string findVarOff(string a, int off);
  int getOffsetMem(string var);
  // int getVarOff(string var);
  bool addTemp(string temp, DataType dt);
  bool addTemp(string temp, int sz);
  bool popTemp(string temp); 
  bool clearAllTemp();
    // how to keep track of the register during its live range?? 
};
