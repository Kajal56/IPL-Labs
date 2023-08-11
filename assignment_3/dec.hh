#pragma once
#include<bits/stdc++.h>
using namespace std;

enum Types {
  STRING_LIT_TYPE,
  INT_TYPE, 
  FLOAT_TYPE,
  VOID_TYPE, 
  STRUCT_TYPE
};

class Type_Specifier_Class;

class Declarator_Class;
class Declarator_List_Class;
class Declaration_Class;
class Declaration_List_Class;

class Fun_Declarator_Class;
class Parameter_List_Class;
class Parameter_Declaration_Class;

struct DataType{
    int size;
    int base_type_size;
    string type_name;
    int base_type;
    string base_type_name;
    int star;
    vector<int> arr;
    int ref;
    int val;
    bool isConst;
};


DataType createDataType(Type_Specifier_Class*, Declarator_Class*);
DataType createDataType(int type);
DataType createDataType(string type);
DataType refDataType(DataType dt);
struct DataType derefDataType(DataType dt);

class Type_Specifier_Class{
  public:
    int type;
    int size;
    string type_name;
    Type_Specifier_Class();
    Type_Specifier_Class(int type);
    Type_Specifier_Class(int type, string name);
};

class Fun_Declarator_Class{
  public:
    string identifier;
    Parameter_List_Class* param_list;
    Fun_Declarator_Class(string fname);
    Fun_Declarator_Class(string fname, Parameter_List_Class* plc);
};

class Parameter_Declaration_Class{
  public:
    Type_Specifier_Class* type_spec;
    Declarator_Class* declarator;
    Parameter_Declaration_Class(Type_Specifier_Class* tsc, Declarator_Class* dc);
};

class Parameter_List_Class{
  public:
    vector<Parameter_Declaration_Class*> vec;
    void push(Parameter_Declaration_Class* pdc);
};


class Declarator_Class{
  public:
    string identifier;
    int star;
    vector<int> arr;
    Declarator_Class();
    Declarator_Class(string id);
    void addStar();
    void addArrIndex(int val);
};

class Declarator_List_Class{
  public:
    int offset;
    vector<Declarator_Class*> vec;
    Declarator_List_Class();
    void push(Declarator_Class* dc);
};

class Declaration_Class{
  public:
    Type_Specifier_Class* type_spec;
    Declarator_List_Class* declarator_list;
    Declaration_Class(Type_Specifier_Class* ts, Declarator_List_Class* dlc);
};

class Declaration_List_Class{
  public:
    vector<Declaration_Class*> vec;
    int offset;
    Declaration_List_Class();
    void push(Declaration_Class* dc);
    vector<Declaration_Class*> getDecs();
};

// class Array_Type_Class:public Type_Tree_Class{
//   public:
//     int* array_size;
//     Type_Tree_Class* child_tt;
//     Array_Type_Class(int* arr_sz, Type_Tree_Class* tt){
//       this->array_size=arr_sz;
//       this->child_tt=tt;
//     }
// };

// class Pointer_Type_Class: public Type_Tree_Class{
//   public:
//     Type_Tree_Class* child_tt;
//     Pointer_Type_Class(Type_Tree_Class* tt){
//       this->child_tt=tt;
//     }
// };
