#pragma once
#include<bits/stdc++.h>
#include "ast.hh"
#include "regMgmt.hh"
#include "dec.hh"
using namespace std;

bool isNum(string s); 
string genLabel(); // generates temp label for jumps
string newTemp(int sz, AddressDescriptor* addDsp); // new temp for store in mem
string newTempVar(); // temp var for non l-value 
string getTemp(); // tbdl
string getLealTemp();

struct InhAtr{
  bool fall;
  bool isCondit;
  class SymbTabEntry* ste;
};

struct SynAtr{
  bool isConst;
  int constVal;
  string var;  // declared or derived variables/ expression
  vector<string*> truelist;
  vector<string*> falselist;
  vector<string*> breaklist; // useless
  vector<string*> contlist; // useless
  vector<string*> next;
  DataType dt;
  // deque<int> offset;
  bool isFuncCall = false; 
  int offset;
};

bool backpatch(vector<string*> v_str, string label); 
vector<string*> mergelist(vector<string*>& v1, vector<string*>& v2);
void merge(vector<string>& file, deque<string>& code);
bool isRelOp(string op);
string op_to_ins(string op, bool fall); //binary op to inturstion used

// get variable from constant, reg or mem
string getVar(struct SynAtr var, RegisterDescriptor* regDsp, AddressDescriptor* addDsp);

// get variable from mem
string getMem(string name, AddressDescriptor* addDsp);
string getMem(struct SynAtr syn, AddressDescriptor* addDsp);
string getMem(struct SynAtr, int offset, AddressDescriptor* addDsp);

// returns offset of return variable in funcall (return astNode)
int getRetOff(string fname);

// returns datatype of variable id given name E [func, struct]
DataType getIdDt(string name, string id, bool isFunc = true);
int getSizeTS(string type);

int getStructIdOff(string curr_struct, string id); // offset struct 
int getOffTot(struct SynAtr syn, int off_base, AddressDescriptor* addDsp); // will give total offset