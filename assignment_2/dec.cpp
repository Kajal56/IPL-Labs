#pragma once 
#include "dec.hh"

// Datatype
struct DataType createDataType(Type_Specifier_Class* ts, Declarator_Class* dc){
  DataType datatype;  
  datatype.star = dc->star;
  datatype.arr = dc->arr;
  datatype.base_type_name = ts->type_name;
  datatype.base_type = ts->type;
  datatype.base_type_size = ts->size;
  datatype.ref = 0;
  datatype.isConst = false;
  
  datatype.size = 1;
  datatype.val = -1;
  datatype.type_name = ts->type_name;
  string stars(dc->star, '*');
  datatype.type_name = datatype.type_name+stars;
  if(datatype.ref) datatype.type_name = datatype.type_name + "(*)";
  if(dc->arr.size() != 0){
    for(int j=0; j<(int)(dc->arr.size()); j++){
            datatype.size*=dc->arr[j];
            // check below later
            datatype.type_name += "[" + to_string(dc->arr[j]) +"]";
    }
  }
  
  if(dc->star){
          datatype.size = 4*datatype.size;
  }else{
          datatype.size = ts->size*datatype.size;
  }
  return datatype;
}

DataType createDataType(int type){
  Type_Specifier_Class* ts = new Type_Specifier_Class(type);
  Declarator_Class* dc = new Declarator_Class();
  return createDataType(ts, dc);
}

struct DataType refDataType(DataType dt){
  DataType datatype;  
  datatype = dt;
  if(dt.arr.size()){
    datatype.star = dt.star;
    datatype.ref++;
    datatype.size = 4;
  }
  else{
    datatype.star = dt.star +1;
    datatype.arr = dt.arr;
    datatype.size =  4; //not sure 
  }
  datatype.type_name = dt.base_type_name;
  string stars(datatype.star, '*');
  datatype.type_name = datatype.type_name+stars;
  if(datatype.ref) datatype.type_name = datatype.type_name + "(*)";
  if(datatype.arr.size() != 0){
    for(int j=0; j<(int)(datatype.arr.size()); j++){
            datatype.size*=datatype.arr[j];
            // check below later
            datatype.type_name += "[" + to_string(datatype.arr[j]) +"]";
    }
  }
  return datatype;

}

struct DataType derefDataType(DataType dt){
  DataType datatype;  
  datatype = dt;
  if(dt.ref){
    datatype.ref--;
  }
  else if(datatype.arr.size()){
    int div_fac = datatype.arr[0];
    datatype.arr.erase(datatype.arr.begin());
    datatype.size = datatype.size / div_fac;
  }
  else if(datatype.star){
    datatype.star = datatype.star -1;
    if(!datatype.star){
      datatype.size = datatype.base_type_size;
    }
    //size thing to be handled
  }

  datatype.type_name = dt.base_type_name;
  string stars(datatype.star, '*');
  datatype.type_name = datatype.type_name+stars;
  if(datatype.ref) datatype.type_name = datatype.type_name + "(*)";
  if(datatype.arr.size() != 0){
    for(int j=0; j<(int)(datatype.arr.size()); j++){
            datatype.size*=datatype.arr[j];
            // check below later
            datatype.type_name += "[" + to_string(datatype.arr[j]) +"]";
    }
  }
  return datatype;

}

string t_name(DataType datatype){
  string s = datatype.base_type_name;
  string stars(datatype.star, '*');
  s = s+stars;
  if(datatype.ref) s = s + "(*)";
  if(datatype.arr.size() != 0){
    for(int j=0; j<(int)(datatype.arr.size()); j++){
            s += "[" + to_string(datatype.arr[j]) +"]";
    }
  }
  return s;

}

int dtype_size(DataType datatype){
  if(datatype.ref) return 8;
  else return datatype.size;
}

// Type_Specifier_Class
Type_Specifier_Class::Type_Specifier_Class(){
  // do nothing
}
Type_Specifier_Class::Type_Specifier_Class(int type){
  this->type=type;
  switch(this->type)
  {
    case INT_TYPE: 
      this->size = 4; 
      this->type_name = "int";
      break;
    case FLOAT_TYPE : 
      this->size = 4; 
      this->type_name = "float";
      break;
    case VOID_TYPE : 
      this->size = 0; 
      this->type_name = "void";
      break;
  }
}
Type_Specifier_Class::Type_Specifier_Class(int type, string name){
  this->type = type;
  this->type_name = name;
}

// Fun_Declarator_Class
Fun_Declarator_Class::Fun_Declarator_Class(string fname){
  this->identifier = fname;
  this->param_list = NULL;
}
Fun_Declarator_Class::Fun_Declarator_Class(string fname, Parameter_List_Class* plc){
  this->identifier = fname;
  this->param_list = plc;
}

// Parameter_Declaration_Class
Parameter_Declaration_Class::Parameter_Declaration_Class(Type_Specifier_Class* tsc, Declarator_Class* dc){
  this->type_spec=tsc;
  this->declarator=dc;
}

// Parameter_List_Class
void Parameter_List_Class::push(Parameter_Declaration_Class* pdc){
  this->vec.push_back(pdc);
}

// Declarator_Class 
Declarator_Class::Declarator_Class(){
  // do nothing
  this->identifier = "";
  this->star = 0;
}
Declarator_Class::Declarator_Class(string id){
  this->identifier = id;
  this->star = 0;
}
void Declarator_Class::addStar(){
  this->star++;
}
void Declarator_Class::addArrIndex(int val){
  arr.push_back(val);
}

// Declaration_List_Class
Declarator_List_Class::Declarator_List_Class(){
  this->offset = 0;
}
void Declarator_List_Class::push(Declarator_Class* dc){
  this->vec.push_back(dc);
}

// Declaration_Class
Declaration_Class::Declaration_Class(Type_Specifier_Class* ts, Declarator_List_Class* dlc){
  this->type_spec = ts;
  this->declarator_list = dlc;
}

// Declaration_List_Class
Declaration_List_Class::Declaration_List_Class(){
  this->offset = 0;
}
void Declaration_List_Class::push(Declaration_Class* dc){
  this->vec.push_back(dc);
}
vector<Declaration_Class*> Declaration_List_Class::getDecs(){
  return vec; 
}