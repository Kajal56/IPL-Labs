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
using namespace std;
SymbTab gst;
map<string, Abstract_Astnode*> ast;
SymbTab predefined; 
map<string, SymbTab*> flocal_vars;
map<string, SymbTab*> slocal_vars;
// populating pre-defined
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
//populated pre-defined
void print_st();


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
  print_st();

}

void print_st(){
    bool isFirst=1;
  std::cout<<"{\n\t\"globalST\" : [";
  // for (auto i:gst.st){
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
        // else{
        //   cout<<"-------------------------\n";
        //   cout<<i.second->var_fun_type<<endl;
        //   cout<<"-------------------------\n";
        // }
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


