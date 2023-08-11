#include <cstring>
#include <cstddef>
#include <istream>
#include <iostream>
#include <fstream>

#include "scanner.hh"
#include "parser.tab.hh"
#include "ast.hh"
#include "symtab.hh"
#include "dec.hh"
#include "checks.hh"
#include "regMgmt.hh"
// #include "utils.hh"

using namespace std;

SymbTab gst;
map<string, Abstract_Astnode*> ast;
SymbTab predefined; 
map<string, SymbTab*> flocal_vars;
map<string, SymbTab*> slocal_vars;
RegisterDescriptor* regDsp = new RegisterDescriptor("");
AddressDescriptor* addDsp = new AddressDescriptor("");
SymbTab *curr_symbatb;
string curr_func_driver;


// populating pre-defined
void populate_predef();
//populated pre-defined
void print_st();
vector<string> asmFile;

void init_xlocal_var(){
  for (map<string,SymbTabEntry*>::iterator it=gst.stes.begin();it!=gst.stes.end();it++){
    pair<string,SymbTabEntry*> i= *it;
    if(i.second->var_fun_type=="fun" && i.second->symtab!= NULL) flocal_vars[i.second->name]=i.second->symtab;
    else if(i.second->var_fun_type=="struct" && i.second->symtab!= NULL) slocal_vars[i.second->name]=i.second->symtab;
  }
}

void fillFile(string filePath){
  init_xlocal_var();
  asmFile.push_back("\t\t.file\t\t\""+filePath+"\"\n");
  asmFile.push_back("\t\t.text\n");
  for (map<string, Abstract_Astnode*>::iterator it=ast.begin();it!=ast.end();it++){
    SymbTabEntry* ste = gst.find(it->first);
    if(ste->var_fun_type != "fun") continue;
    curr_func_driver = it->first;
    delete(regDsp);
    delete(addDsp);

    regDsp = new RegisterDescriptor(it->first);
    addDsp = new AddressDescriptor(it->first);

    deque<string> code;

    string temp = "";
    temp+="\t\t.text\n";
    temp+="\t\t.global " + addDsp->context+"\n";
    temp+="\t\t.type " + addDsp->context + ", @function\n";
    code.push_back(temp);

    struct InhAtr inh;
    inh.fall = true;
    code.push_back(it->first+":\n");
    code.push_back("\t\tpushl\t\t%ebp \n");
    code.push_back("\t\tmovl\t\t%esp, %ebp\n");
    int sz = 0;
    curr_symbatb = ste->symtab;
    // init variables in mem
    for(map<string,SymbTabEntry*>::iterator itr=ste->symtab->stes.begin();itr!=ste->symtab->stes.end();itr++){
      if(itr->second->offset < 0) sz+=itr->second->size;
      addDsp->var_offset[itr->first] = itr->second->offset-4;
      addDsp->espOff = min(addDsp->espOff, itr->second->offset);
    }
    if(sz > 0){
      code.push_back("\t\tsubl\t\t$"+to_string(sz)+", %esp\n");
    }
    it->second->genCode(code, inh);
    if(code.back() != "ret"){
      code.push_back("\t\tleave\n");
      code.push_back("\t\tret\n");
    }
    code.push_front("\t\t.section\t\t.rodata\n");
    code.push_back("\t\t.size\t\t"+it->first+", .-"+it->first+"\n");
    merge(asmFile, code);
  }
  
  asmFile.push_back("\t\t.ident\t\t\"GCC: (Ubuntu 8.1.0-9ubuntu1~16.04.york1) 8.1.0\"\n");
  asmFile.push_back("\t\t.section\t\t.note.GNU-stack,\"\",@progbits\n");

  for(int i=0; i<(int)asmFile.size(); i++){
    cout<<asmFile[i];
  }
}

int main(const int argc, const char **argv)
{
  // using namespace std;
  fstream in_file;
  populate_predef();
  in_file.open(argv[1], ios::in);
  // Generate a scanner
  IPL::Scanner scanner(in_file);
  // Generate a Parser, passing the scanner as an argument.
  // Remember %parse-param { Scanner  &scanner  }
  IPL::Parser parser(scanner);
  
  #ifdef YYDEBUG
   parser.set_debug_level(1);
  #endif 
  // std::cout << "digraph D { node [ordering=out]" << std::endl;
  parser.parse();
  fillFile(argv[1]);
  // print_st();
}

void populate_predef(){
  SymbTabEntry* ste = new SymbTabEntry("printf","proc","global");
  ste->size = 0;
  ste->symtab=NULL;
  ste->type_returnType = "void";
  predefined.insert(ste->name, ste);
  ste = new SymbTabEntry("scanf","proc","global");
  ste->size = 0;
  ste->symtab=NULL;
  ste->type_returnType = "void";

  predefined.insert(ste->name, ste);
}

void print_st(){
  bool isFirst=1;
  std::cout<<"{\n\t\"globalST\" : [";
  for (map<string,SymbTabEntry*>::iterator it=gst.stes.begin();it!=gst.stes.end();it++){
    pair<string,SymbTabEntry*> i= *it;
    if(!isFirst){
      cout<<",";
    }
    cout<<"\n[\n"<<"\""<<i.second->name<<"\"";
    cout<<",\n"<<"\""<<i.second->var_fun_type<<"\"";
    cout<<",\n"<<"\""<<i.second->scope<<"\"";
    cout<<",\n"<<i.second->size;
    if(i.second->var_fun_type=="struct"){
      cout<<",\n"<<"\"-\"";  
    }
    else{
      cout<<",\n"<<i.second->offset;
    }
    cout<<",\n"<<"\""<<i.second->type_returnType<<"\"";
    cout<<"\n]";
    isFirst=0;
    if(i.second->var_fun_type=="fun" && i.second->symtab!= NULL) flocal_vars[i.second->name]=i.second->symtab;
    else if(i.second->var_fun_type=="struct" && i.second->symtab!= NULL) slocal_vars[i.second->name]=i.second->symtab;
  }
  bool inIsFirst;
  cout<<"\n],\n\"structs\": [\n";
  isFirst=1;
  for (map<string,SymbTab*>::iterator it=slocal_vars.begin();it!=slocal_vars.end();it++){
      pair<string,SymbTab*> j=*it;
      if(!isFirst){
        cout<<",";
      }
      cout<<"{\n\"name\": \""<<j.first<<"\",";
      cout<<"\"localST\":[\n";
      inIsFirst=1;
      // for(auto j: i.second->st){
        for(map<string,SymbTabEntry*>::iterator it2=j.second->stes.begin();it2!=j.second->stes.end();it2++){
        pair<string,SymbTabEntry*> i=*it2;
        if(!inIsFirst){
          cout<<",";
        }
        // cout<<"\n[\n";
        cout<<"\n[\n"<<"\""<<i.second->name<<"\"";
        cout<<",\n"<<"\""<<i.second->var_fun_type<<"\"";
        cout<<",\n"<<"\""<<i.second->scope<<"\"";
        cout<<",\n"<<i.second->size;
        cout<<",\n"<<(i.second->offset+i.second->size)*(-1);
        cout<<",\n"<<"\""<<i.second->type_returnType<<"\"";
        cout<<"\n]";
        inIsFirst=0;
      }
      cout<<"\n]\n}";
      isFirst=0;

  }
  cout<<"\n],\n";
  cout<<"\"functions\":[\n";



  //-------------------copying structs part --------
  isFirst=1;
  // for(auto i:flocal_vars){
  for (map<string,SymbTab*>::iterator it=flocal_vars.begin();it!=flocal_vars.end();it++){
      pair<string,SymbTab*> j= *it;
      if(!isFirst){
        cout<<",";
      }
      cout<<"{\n\"name\": \""<<j.first<<"\",\n";
      cout<<"\"localST\":[\n";
      inIsFirst=1;
      // for(auto j: i.second->st){
      for(map<string,SymbTabEntry*>::iterator it2=j.second->stes.begin();it2!=j.second->stes.end();it2++){
        pair<string,SymbTabEntry*> i=*it2;
        if(!inIsFirst){
          cout<<",";
        }
        // cout<<"\n[\n";
        cout<<"\n[\n"<<"\""<<i.second->name<<"\"";
        cout<<",\n"<<"\""<<i.second->var_fun_type<<"\"";
        cout<<",\n"<<"\""<<i.second->scope<<"\"";
        cout<<",\n"<<i.second->size;
        cout<<",\n"<<i.second->offset;
        cout<<",\n"<<"\""<<i.second->type_returnType<<"\"";
        cout<<"\n]";
        inIsFirst=0;
      }
      cout<<"\n],\n";
      cout<<"\"ast\":{\n";
      ast[j.first]->print(4); //or whatever argument :)
      cout<<"}\n}";
      isFirst=0;

  }
  cout<<"\n]\n}";

}

  // for (map<string,SymbTabEntry*>::iterator it=gst.stes.begin();it!=gst.stes.end();it++){
  //   pair<string,SymbTabEntry*> i= *it;
  //   if(i.second->var_fun_type == "struct") continue;
  //   vector<string> code;
  //   InhAtr inh;
  //   inh.fall = true;
  //   code.push_back(fname+":\n");
  //   code.push_back("\t\t pushl\t\t %ebp \n");
  //   code.push_back("\t\t movl\t\t %esp, %ebp\n");
  // }

// void fillAsmFile(string fname, Abstract_Astnode* astPtr, vector<string>& asmFile){
//   vector<string> code, printf_strs;
//   code.push_back(fname+":\n");
//   code.push_back("\t\t pushl\t\t %ebp \n");
//   code.push_back("\t\t movl\t\t %esp, %ebp\n");
//   SymbTabEntry* ste = gst.find(fname);
//   // set esp wali instruction
//   // assgin entry for every stack variable in varDescriptor class

//   struct InhAtr inh;
//   inh.fall = true;
//   // astPtr->genCode(code, inh);
// }

