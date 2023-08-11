%skeleton "lalr1.cc"
%require  "3.0.1"

%defines 
%define api.namespace {IPL}
%define api.parser.class {Parser}

%define parse.trace

%code requires{

  #pragma once
  #include<bits/stdc++.h>
  using namespace std;
  #include "ast.hh"
  #include "symtab.hh"
  #include "dec.hh"
  #include "checks.hh"
  #include "utils.hh"
  
  namespace IPL {
      class Scanner;
  }

  //# ifndef YY_NULLPTR
  //#  if defined __cplusplus && 201103L <= __cplusplus
  //#   define YY_NULLPTR nullptr
  //#  else
  //#   define YY_NULLPTR 0
  //#  endif
  //# endif

}

%printer { std::cerr << $$; } STRUCT
%printer { std::cerr << $$; } IDENTIFIER
%printer { std::cerr << $$; } VOID
%printer { std::cerr << $$; } INT
%printer { std::cerr << $$; } FLOAT
%printer { std::cerr << $$; } INT_CONSTANT
%printer { std::cerr << $$; } FLOAT_CONSTANT
%printer { std::cerr << $$; } RETURN
%printer { std::cerr << $$; } AND_OP
%printer { std::cerr << $$; } OR_OP
%printer { std::cerr << $$; } EQ_OP
%printer { std::cerr << $$; } NE_OP
%printer { std::cerr << $$; } LE_OP
%printer { std::cerr << $$; } GE_OP
%printer { std::cerr << $$; } IF
%printer { std::cerr << $$; } ELSE
%printer { std::cerr << $$; } INC_OP
%printer { std::cerr << $$; } PTR_OP
%printer { std::cerr << $$; } STRING_LITERAL
%printer { std::cerr << $$; } WHILE
%printer { std::cerr << $$; } FOR
%printer { std::cerr << $$; } OTHERS


%parse-param { Scanner  &scanner  }
%locations
%code{
   #include <iostream>
   #include <cstdlib>
   #include <fstream>
   #include <string>
   #include <unordered_map>
   #include "ast.cpp"
   #include "dec.cpp"
   #include "symtab.cpp"
   #include "checks.cpp"
   #include "utils.cpp"

   #include "scanner.hh"
#undef yylex
#define yylex IPL::Parser::scanner.yylex

    string fname;
    extern map<string, Abstract_Astnode*> ast;
    extern SymbTab gst;
    SymbTab *st;
    SymbTabEntry* currFunc;
    string currStruct;
    extern SymbTab predefined;

    int line = 1;
}


%define api.value.type variant
%define parse.assert

%start translation_unit

%token '\n'
%token <std::string> STRUCT
%token <std::string> VOID
%token <std::string> INT 
%token <std::string> FLOAT 
%token <std::string> RETURN
%token <std::string> AND_OP
%token <std::string> OR_OP
%token <std::string> EQ_OP
%token <std::string> NE_OP
%token <std::string> LE_OP
%token <std::string> GE_OP
%token <std::string> IF
%token <std::string> ELSE
%token <std::string> INC_OP
%token <std::string> PTR_OP
%token <std::string> WHILE
%token <std::string> FOR
%token <std::string> OTHERS
%token <std::string> IDENTIFIER 
%token <std::string> STRING_LITERAL
%token <std::string> INT_CONSTANT
%token <std::string> FLOAT_CONSTANT
%token ';' '{' '}' '(' ')' '[' ']' ':' '-' '+' '*' '/' '.' '<' '>' '=' '&' '!'

%nterm <Abstract_Astnode*> translation_unit;
%nterm <Abstract_Astnode*> struct_specifier;
%nterm <Abstract_Astnode*> function_definition;

%nterm <Type_Specifier_Class*> type_specifier;
%nterm <Declarator_Class*> declarator;
%nterm <Declarator_Class*> declarator_arr;
%nterm <Declarator_List_Class*> declarator_list;
%nterm <Declaration_Class*> declaration;
%nterm <Declaration_List_Class*> declaration_list;

%nterm <Exp_Astnode*> expression;

%nterm <Fun_Declarator_Class*> fun_declarator;
%nterm <Parameter_Declaration_Class*> parameter_declaration;
%nterm <Parameter_List_Class*> parameter_list;
%nterm <Statement_Astnode*> statement;
%nterm <AssignS_Astnode*> assignment_statement;

%nterm <Abstract_Astnode*> compound_statement;
%nterm <Seq_Astnode*> statement_list;
%nterm <AssignE_Astnode*> assignment_expression;
%nterm <Exp_Astnode*> logical_and_expression;
%nterm <Exp_Astnode*> equality_expression;
%nterm <Exp_Astnode*> relational_expression;
%nterm <Exp_Astnode*> additive_expression;
%nterm <Exp_Astnode*> multiplicative_expression;
%nterm <Exp_Astnode*> unary_expression;
%nterm <Exp_Astnode*> postfix_expression;
%nterm <Exp_Astnode*> primary_expression;
%nterm <Op_Unary_Astnode*> unary_operator;
%nterm <Funcall_Astnode*> expression_list;
%nterm <Funcall_Astnode*> procedure_call;
%nterm <Statement_Astnode*> selection_statement;
%nterm <Statement_Astnode*> iteration_statement;

%%

translation_unit: 
    struct_specifier
{
}
    | function_definition
{   
        ast[fname] = $1;
}
    | translation_unit struct_specifier
{
}
    | translation_unit function_definition
{
        ast[fname] = $2;
}
;

struct_specifier: 
    STRUCT IDENTIFIER 
{
        st = new SymbTab();
        currStruct = "struct "+$2;
        if(gst.find(currStruct)){
                cout<<"Error at line "<<line<<": "<<currStruct<<" already defined in struct_specifier\n";
                exit(1);
        }
        SymbTabEntry* ste = new SymbTabEntry();
        ste->name = currStruct;
        ste->scope = "global";
        gst.insert(currStruct, ste);
} 
        '{' declaration_list '}' ';' 
{
        SymbTabEntry* ste = gst.find(currStruct);
        ste->size = -1*($5->offset);
        ste->var_fun_type = "struct";
        ste->symtab=st;
        ste->type_returnType="-";
        currStruct = "";
        $$= new Seq_Astnode();
}
;

function_definition: 
        type_specifier fun_declarator 
{
        fname = $2->identifier;
        if(gst.find(fname) != NULL){
                cout<<"Error at line "<<line<<": in func_def redefinition of "<<fname<<endl;
                exit(1);
        }   
        SymbTabEntry* ste = new SymbTabEntry();
        ste->name = $2->identifier;
        ste->type_returnType = $1->type_name;
        ste->var_fun_type = "fun";
        ste->scope = "global";
        ste->symtab = st;
        ste->datatype = createDataType($1, new Declarator_Class());
        gst.insert(fname, ste);
}       
        compound_statement
{
        $$ = $4;
}
;
    
fun_declarator: IDENTIFIER '(' parameter_list ')'
{
        fname = $1;
        currFunc = new SymbTabEntry();
        st = new SymbTab();
        $$ = new Fun_Declarator_Class($1, $3);

        Parameter_Declaration_Class* param;
        Type_Specifier_Class* type_spec;
        Declarator_Class* declarator;
        int size;
        
        DataType datatype;

        int curr_offset = 12;
        
        for(int i=(int)($3->vec.size())-1; i>=0; i--){
                param = $3->vec[i];
                type_spec = param->type_spec;
                declarator = param->declarator;

                datatype = createDataType(type_spec, declarator);
                size = datatype.size;
                if(datatype.base_type_name=="void"){
                        if(!isPointer(datatype)){
                                cout << "Error at line "<<line<<": Cannot declare parameter of type \"void\"";
                                exit(1);
                        }
                }
                
                if(st->find(declarator->identifier) != NULL)
                {
                        cout<<"Error at line "<<line<<": identifier \""<<declarator->identifier<<"\" already declared\n";
                        exit(1);
                }

                SymbTabEntry* ste = new SymbTabEntry();     
                ste->name = declarator->identifier;
                ste->var_fun_type = "var";
                ste->type_returnType = datatype.type_name;
                ste->size = size;
                ste->offset = curr_offset;
                ste->symtab = NULL;
                ste->scope = "param";
                ste->datatype = datatype;
                st->insertParam(declarator->identifier, ste);
                curr_offset += size;
        }
}
        | IDENTIFIER '(' ')'
{       
        fname = $1;
        currFunc = new SymbTabEntry();
        st = new SymbTab();
        $$ = new Fun_Declarator_Class($1);     
}
;

type_specifier: 
        VOID
{
        $$ = new Type_Specifier_Class(VOID_TYPE);
}
        | INT
{
        $$ = new Type_Specifier_Class(INT_TYPE);
}
        | FLOAT
{
        $$ = new Type_Specifier_Class(FLOAT_TYPE);
}
        | STRUCT IDENTIFIER 
{       
        string name = "struct "+$2;
        $$ = new Type_Specifier_Class(STRUCT_TYPE, name);
        if(gst.find(name) != NULL){
                $$->size = gst.find(name)->size;
        }else{
                cout<<"Struct identifier in line "<<line<<": "<<name<<" not defined\n";
        }
}

parameter_list:
        parameter_declaration
{
        $$ = new Parameter_List_Class();
        $$->push($1);
}
        | parameter_list ',' parameter_declaration
{       
        $$ = $1;
        $$->push($3);
};

parameter_declaration:
       type_specifier declarator
{
        $$ = new Parameter_Declaration_Class($1, $2);
}
;
declarator: 
        declarator_arr 
{
        $$ = $1;
}
        | '*' declarator 
{
        $$ = $2;
        $$->addStar();
};

declarator_arr: 
        IDENTIFIER
{
        $$ = new Declarator_Class($1);
}
        | declarator_arr '[' INT_CONSTANT ']'
{
        int index = stoi($3);
        if(index<0){
                cout<<"Error at line "<<line<<": index cannot be less than zero\n";
                exit(1);
        }
        $$ = $1;
        $$->addArrIndex(index);
}
;

declarator_list: 
        declarator
{
        $$ = new Declarator_List_Class();
        $$->push($1);
}
        | declarator_list ',' declarator
{
        $$ = $1;
        $$->push($3);
}
;

declaration: 
        type_specifier declarator_list ';'
{
        DataType datatype;
        for(int i=0; i< int($2->vec.size());i++){
                datatype= createDataType($1 , $2->vec[i]);
                if(datatype.base_type_name=="void"){
                        if(!isPointer(datatype)){
                                cout << "Error at line "<<line<<": Cannot declare variable of type \"void\"";
                                exit(1);
                        }
                }

        }
        $$ = new Declaration_Class($1, $2);
}
;

declaration_list: 
        declaration
{
        Declarator_Class* declarator;
        Type_Specifier_Class* type_spec = $1->type_spec;
        int curr_offset = 0;
        int size;
        Declarator_List_Class* declarator_list= $1->declarator_list;
        DataType datatype;

        for(int i=0; i<(int)(declarator_list->vec.size()); i++){
                declarator = declarator_list->vec[i];
                datatype = createDataType(type_spec, declarator); 
                size = datatype.size;
                curr_offset -= size; 

                 if(st->find(declarator->identifier) != NULL)
                {
                        cout<<"Error at line "<<line<<": identifier \""<<declarator->identifier<<"\" already declared\n";
                        exit(1);
                }

                SymbTabEntry* ste = new SymbTabEntry();     
                ste->name = declarator->identifier;
                ste->var_fun_type = "var";
                ste->type_returnType = datatype.type_name;
                ste->size = size;
                ste->offset = curr_offset;
                ste->symtab = NULL;
                ste->scope = "local";
                ste->datatype = datatype;

                st->insert(declarator->identifier, ste);        
        }

        $$ = new Declaration_List_Class();
        $$->push($1);
        $$->offset = curr_offset;
}
        | declaration_list declaration
{
        Declarator_Class* declarator;
        Type_Specifier_Class* type_spec = $2->type_spec;
        int curr_offset = $1->offset;
        Declarator_List_Class* declarator_list = $2->declarator_list;  
        int size = 1; 
        DataType datatype;

        for(int i=0; i<(int)(declarator_list->vec.size()); i++){
                declarator = declarator_list->vec[i];
                datatype = createDataType(type_spec, declarator); 
                size = datatype.size;
                curr_offset -= size;  
                if(st->find(declarator->identifier) != NULL)
                {
                        cout<<"Error at line "<<line<<": identifier \""<<declarator->identifier<<"\" already declared\n";
                        exit(1);
                }

                SymbTabEntry* ste = new SymbTabEntry();     
                ste->name = declarator->identifier;
                ste->var_fun_type = "var";
                ste->type_returnType = datatype.type_name;
                ste->size = size;
                ste->offset = curr_offset;
                ste->symtab = NULL;
                ste->scope = "local";
                ste->datatype = datatype;
                
                st->insert(declarator->identifier, ste);        
        }

        $$ = $1; 
        $$->push($2);
        $$->offset = curr_offset;
}
;

compound_statement: 
        '{' '}' 
{
        $$ = new Seq_Astnode();
}
        | '{' statement_list '}' 
{
        $$= $2;
}
        | '{' declaration_list '}' 
{
        $$ = new Seq_Astnode();
}
        | '{' declaration_list statement_list '}' 
{
        $$ = $3;
};

statement_list: 
        statement 
{
        $$ = new Seq_Astnode();
        $$->push($1);    
}
        | statement_list statement 
{
        $$ = $1;
        $$->push($2);
};

statement: 
        ';' 
{
        $$ = new Empty_Astnode();
}
        | '{' statement_list '}' 
{
        $$ = $2;
}
        | selection_statement 
{
        $$ = $1;
}
        | iteration_statement 
{
        $$ = $1;
}
        | assignment_statement 
{
        $$= $1;
}
        | procedure_call 
{
        $$ = new Proccall_Astnode($1->fname, $1->nodes);
        // $$->nodeLabel = 1; // phase 3
}
        | RETURN expression ';' 
{
        DataType retn_type = gst.find(fname)->datatype;
        if(!isCompatible(retn_type, $2->data_type)){
                cout<<"Error at line "<<line<<": "<<$2->data_type.type_name<<" is not compatible with "<<retn_type.type_name<<endl;
                exit(1);
        }
        else if(isInt(retn_type) && isFloat($2->data_type)){
                $$ = new Return_Astnode(new Op_Unary_Astnode("TO_INT", $2));
        }
        else if(isFloat(retn_type) && isInt($2->data_type)){
                $$ = new Return_Astnode(new Op_Unary_Astnode("TO_FLOAT", $2));
        }
        else{
                $$ = new Return_Astnode($2);  
                $$->data_type = retn_type;
        }       
};

assignment_expression: unary_expression '=' expression 
{
        if(!$1->lvalue)
        {
                cout<<"Error at line "<<line<<": Syntax error, lhs does not have a lvalue \n";
                exit(1);
        }
        if($1->data_type.arr.size() != 0)
        {
                cout<<"Error at line "<<line<<": cant assign array \n";
                exit(1); 
        }
        else if($1->data_type.type_name == $3->data_type.type_name)
        {
                $$= new AssignE_Astnode($1, $3);
                $$->data_type = $1->data_type;
                $$->data_type.val = $3->data_type.val;
                $1->data_type.val = $3->data_type.val;
        }
        else if(isPointer($1->data_type) && isInt($3->data_type))
        {
                if($3->data_type.val == 0){
                        $$ = new AssignE_Astnode($1, $3);
                        $$->data_type = $1->data_type;
                        $$->data_type.val = $3->data_type.val;
                        $1->data_type.val = $3->data_type.val;
                }else{
                        cout<<"Error at line "<<line<<": cant assign non-zero value to a pointer\n";
                        exit(1); 
                } 
        }
        else if(isInt($1->data_type) && isFloat($3->data_type))
        {
                $$ = new AssignE_Astnode($1, new Op_Unary_Astnode("TO_INT", $3));
                $$->data_type = $1->data_type;
                $$->data_type.val = $3->data_type.val;
                $1->data_type.val = $3->data_type.val;
        }
        else if(isFloat($1->data_type) && isInt($3->data_type))
        {
                $$ = new AssignE_Astnode($1, new Op_Unary_Astnode("TO_FLOAT", $3));
                $$->data_type = $1->data_type;
                $$->data_type.val = $3->data_type.val;
                $1->data_type.val = $3->data_type.val;
        }
        else if(isPointer($1->data_type) && $3->data_type.val == 0){
                $$ = new AssignE_Astnode($1, $3);  
                $$->data_type = $1->data_type;
                $$->data_type.val = $3->data_type.val;
                $1->data_type.val = $3->data_type.val;
        }
        else if(isCompatible($1->data_type, $3->data_type))
        {
                $$ = new AssignE_Astnode($1, $3);  
                $$->data_type = $1->data_type;
                $$->data_type.val = $3->data_type.val;
                $1->data_type.val = $3->data_type.val;
        }
        else
        {
                cout<<"Error at line "<<line<<": Incompatible assignment when assigning to type \""<<$1->data_type.type_name<<"\" from type \""<<$3->data_type.type_name<<"\"\n";
                exit(1);
        }
};

assignment_statement: assignment_expression ';'
{
        $$ = new AssignS_Astnode($1->left,$1->right);          
}
;
procedure_call: 
        IDENTIFIER '(' ')' ';'
{
        if(predefined.find($1) == NULL){
                SymbTabEntry *func = gst.find($1);
                if(func == NULL)
                {
                        cout<<"Error at line "<<line<<": Func "<<$1<<" not defined  procedure_call\n";
                        exit(1);
                }
                if(func->var_fun_type != "fun")
                {
                        cout<<"Error at line "<<line<<": Object called "<<$1<<" not a function procedure_call\n";
                        exit(1);  
                }
                if(func->symtab->params.size() != 0)
                {
                        cout<<"Error at line "<<line<<": too many argument procedure_call\n"; 
                        exit(1);
                }
                $$ = new Funcall_Astnode(new Identifier_Astnode($1));
                $$->data_type = func->datatype;
        }else{
                $$ = new Funcall_Astnode(new Identifier_Astnode($1));
                $$->data_type = createDataType(VOID_TYPE);
        } 
        // $$->nodeLabel = 1; // phase 3       
}
        | IDENTIFIER '(' expression_list ')' ';'
{
        if(predefined.find($1) == NULL){
                SymbTabEntry *func = gst.find($1);
                if(func == NULL)
                {
                        cout<<"Error at line "<<line<<": Func "<<$1<<" not defined procedure_call\n";
                        exit(1);
                }
                if(func->var_fun_type != "fun"){
                        cout<<"Error at line "<<line<<": Object called "<<$1<<" not a function  procedure_call\n";
                        exit(1);  
                }
                if(func->symtab->params.size() < $3->nodes.size()){
                        cout<<"Error at line "<<line<<": too few argument  procedure_call\n"; 
                        exit(1);
                }
                if(func->symtab->params.size() > $3->nodes.size()){
                        cout<<"Error at line "<<line<<": too many argument  procedure_call\n"; 
                        exit(1);
                }
                SymbTab *symtab = func->symtab;
                string param;
                int n = (int) $3->nodes.size();
                for(int i=0; i<(int)$3->nodes.size(); i++){
                        param = symtab->params[n-i-1];
                        
                        if(!isCompParam(symtab->stes[param]->datatype, $3->nodes[i]->data_type))
                        {
                                //cout<<"Error at line "<<line<<": parameter "<<i<<" is in compatible \n";
                                cout<<"Error at line "<<line<<":  procedure_call expected "<<symtab->stes[param]->datatype.type_name<<" but argument is of type "<<$3->nodes[i]->data_type.type_name<<"\n";
                                exit(1);
                        }
                        if(isFloat(symtab->stes[param]->datatype) && isInt($3->nodes[i]->data_type)){
                                $3->nodes[i] = new Op_Unary_Astnode("TO_FLOAT", $3->nodes[i]);
                        }
                        // check if the parameters passes are type correct of not 
                        // else cout<<"Error at line "<<line<<": type mismatch_ of parameter "<<i+1<<endl;
                }
                
                $$ = new Funcall_Astnode(new Identifier_Astnode($1));
                $$->data_type = func->datatype;
                $$->nodes=$3->nodes;               
        }
        else
        {
                SymbTabEntry* func= predefined.find($1);
                if(func->var_fun_type != "proc"){
                        cout<<"Error at line "<<line<<": Object called "<<$1<<" not a procedure\n";
                        exit(1);  
                }
               // cout<<"------check1------\n";
                $$ = new Funcall_Astnode(new Identifier_Astnode($1));
                $$->nodes=$3->nodes;
               // cout<<"------check2------\n";

                $$->data_type = createDataType(VOID_TYPE);
                
        }
        // $$->nodeLabel = 1; // phase 3       
}
;
expression:   
        logical_and_expression
{
        $$ = $1;
}
        | expression OR_OP logical_and_expression
{
        $$ = new Op_Binary_Astnode("OR_OP", $1, $3);
        $$->data_type = createDataType(INT_TYPE);
        $$->lvalue = false;
        $$->rvalue = true;

        // nodeLabel
        if($3->nodeLabel == 0 || $3->nodeLabel == 0){
                $$->nodeLabel = $1->nodeLabel;
        } else if($1->nodeLabel == $3->nodeLabel){
                $$->nodeLabel = $1->nodeLabel+1;
        }else{
                $$->nodeLabel = max($1->nodeLabel,$3->nodeLabel);
        }
}
;

logical_and_expression: 
        equality_expression
{
        $$ = $1;
}
        | logical_and_expression AND_OP equality_expression
{
        $$ = new Op_Binary_Astnode("AND_OP", $1, $3);
        $$->data_type = createDataType(INT_TYPE);
        $$->lvalue = false;

        // nodeLabel
        if($3->nodeLabel == 0 || $3->nodeLabel == 0){
                $$->nodeLabel = $1->nodeLabel;
        } else if($1->nodeLabel == $3->nodeLabel){
                $$->nodeLabel = $1->nodeLabel+1;
        }else{
                $$->nodeLabel = max($1->nodeLabel,$3->nodeLabel);
        }
}
;

equality_expression: 
        relational_expression
{
        $$ = $1;
}
        | equality_expression EQ_OP relational_expression
{
        if(isInt($1->data_type))
        {
                if(isInt($3->data_type)){
                        $$ = new Op_Binary_Astnode("EQ_OP_INT", $1, $3);
                        $$->data_type = $1->data_type;
                }else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("EQ_OP_FLOAT", new Op_Unary_Astnode("TO_FLOAT", $1), $3);
                        $$->data_type = $1->data_type;
                }else{
                        cout<<"Error at line "<<line<<": Invalid operands types for binary ==, \""<<$1->data_type.type_name<<"\" and \""<<$3->data_type.type_name<<"\"\n";
                        //cout<<"Error at line "<<line<<": Invalid operands types for binary ==, \""<<$1->data_type.type_name<<"\" and \";

                        exit(1);   
                }       
        }
        else if(isFloat($1->data_type))
        {
                if(isInt($3->data_type)){
                        $$ = new Op_Binary_Astnode("EQ_OP_FLOAT", $1, new Op_Unary_Astnode("TO_FLOAT", $3));    
                        $$->data_type = createDataType(INT_TYPE);
                }
                else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("EQ_OP_FLOAT", $1, $3);    
                        $$->data_type = createDataType(INT_TYPE);
                }
                else{
                        cout<<"Error at line "<<line<<": type mismatch in rel\n";
                        exit(1);   
                }
        }else if(isCompSub($1->data_type , $3->data_type)){
                $$ = new Op_Binary_Astnode("EQ_OP_INT" , $1, $3);
                $$->data_type = createDataType(INT_TYPE);
        }
        else if(isVoidPtr($1->data_type) || isVoidPtr($3->data_type)){
                $$ = new Op_Binary_Astnode("EQ_OP_INT" , $1, $3);
                $$->data_type = createDataType(INT_TYPE);
        }
        else if( 
                isNullPtr($1->data_type) 
                && 
                (isArray($3->data_type) || isPointer($3->data_type))
        ){
                $$ = new Op_Binary_Astnode("EQ_OP_INT" , $1, $3);
                $$->data_type = createDataType(INT_TYPE);
        }
        else if( 
                isNullPtr($3->data_type) 
                && 
                (isArray($1->data_type) || isPointer($1->data_type))
        ){
                $$ = new Op_Binary_Astnode("EQ_OP_INT" , $1, $3);
                $$->data_type = createDataType(INT_TYPE);
        }
        else
        {
                cout<<"Error at line "<<line<<": type mismatch in ne_expression \n";
                exit(1);   
        }    
        $$->lvalue= false;
        // nodeLabel
        if($3->nodeLabel == 0 || $3->nodeLabel == 0){
                $$->nodeLabel = $1->nodeLabel;
        } else if($1->nodeLabel == $3->nodeLabel){
                $$->nodeLabel = $1->nodeLabel+1;
        }else{
                $$->nodeLabel = max($1->nodeLabel,$3->nodeLabel);
        }
}
        | equality_expression NE_OP relational_expression
{
        if(isInt($1->data_type))
        {
                if(isInt($3->data_type)){
                        $$ = new Op_Binary_Astnode("NE_OP_INT", $1, $3);
                        $$->data_type = $1->data_type;
                }else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("NE_OP_FLOAT", new Op_Unary_Astnode("TO_FLOAT", $1), $3);
                        $$->data_type = $1->data_type;
                }else{
                        cout<<"Error at line "<<line<<": type mismatch rel_exp\n";
                        exit(1);   
                }       
        }
        else if(isFloat($1->data_type))
        {
                if(isInt($3->data_type)){
                        $$ = new Op_Binary_Astnode("NE_OP_FLOAT", $1, new Op_Unary_Astnode("TO_FLOAT", $3));    
                        $$->data_type = createDataType(INT_TYPE);
                }
                else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("NE_OP_FLOAT", $1, $3);    
                        $$->data_type = createDataType(INT_TYPE);
                }
                else{
                        cout<<"Error at line "<<line<<": type mismatch in rel\n";
                        exit(1);   
                }
        }else if(isCompSub($1->data_type , $3->data_type)){
                $$ = new Op_Binary_Astnode("NE_OP_INT" , $1, $3);
                $$->data_type = createDataType(INT_TYPE);
        }
        else if(isVoidPtr($1->data_type) || isVoidPtr($3->data_type)){
                $$ = new Op_Binary_Astnode("NE_OP_INT" , $1, $3);
                $$->data_type = createDataType(INT_TYPE);
        }
        else if( 
                isNullPtr($1->data_type) 
                && 
                (isArray($3->data_type) || isPointer($3->data_type))
        ){
                $$ = new Op_Binary_Astnode("NE_OP_INT" , $1, $3);
                $$->data_type = createDataType(INT_TYPE);
        }
        else if( 
                isNullPtr($3->data_type) 
                && 
                (isArray($1->data_type) || isPointer($1->data_type))
        ){
                $$ = new Op_Binary_Astnode("NE_OP_INT" , $1, $3);
                $$->data_type = createDataType(INT_TYPE);
        }
        else
        {
                cout<<"Error at line "<<line<<": type mismatch in ne_expression \n";
                exit(1);   
        }    
        $$->lvalue= false;
        
        // nodeLabel
        if($3->nodeLabel == 0 || $3->nodeLabel == 0){
                $$->nodeLabel = $1->nodeLabel;
        } else if($1->nodeLabel == $3->nodeLabel){
                $$->nodeLabel = $1->nodeLabel+1;
        }else{
                $$->nodeLabel = max($1->nodeLabel,$3->nodeLabel);
        }
}
;

relational_expression: 
        additive_expression
{
        $$ = $1;
}
        | relational_expression '<' additive_expression
{
        if(isInt($1->data_type))
        {
                if(isInt($3->data_type)){
                        $$ = new Op_Binary_Astnode("LT_OP_INT", $1, $3);
                        $$->data_type = $1->data_type;
                }else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("LT_OP_FLOAT", new Op_Unary_Astnode("TO_FLOAT", $1), $3);
                        $$->data_type = $1->data_type;
                }else{
                        cout<<"Error at line "<<line<<": type mismatch rel_exp\n";
                        exit(1);   
                }       
        }
        else if(isFloat($1->data_type))
        {
                if(isInt($3->data_type)){
                        $$ = new Op_Binary_Astnode("LT_OP_FLOAT", $1, new Op_Unary_Astnode("TO_FLOAT", $3));    
                        $$->data_type = createDataType(INT_TYPE);
                }
                else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("LT_OP_FLOAT", $1, $3);    
                        $$->data_type = createDataType(INT_TYPE);
                }
                else{
                        cout<<"Error at line "<<line<<": type mismatch in rel\n";
                        exit(1);   
                }
        }else if(isCompSub($1->data_type , $3->data_type)){
                $$ = new Op_Binary_Astnode("LT_OP_INT" , $1, $3);
                $$->data_type = createDataType(INT_TYPE);
        }
        else
        {
                cout<<"Error at line "<<line<<": type mismatch int relational_expression \n";
                exit(1);   
        }    

        $$->lvalue= false;
        // nodeLabel
        if($3->nodeLabel == 0 || $3->nodeLabel == 0){
                $$->nodeLabel = $1->nodeLabel;
        } else if($1->nodeLabel == $3->nodeLabel){
                $$->nodeLabel = $1->nodeLabel+1;
        }else{
                $$->nodeLabel = max($1->nodeLabel,$3->nodeLabel);
        }
}
        | relational_expression '>' additive_expression
{
        if(isInt($1->data_type))
        {
                if(isInt($3->data_type)){
                        $$ = new Op_Binary_Astnode("GT_OP_INT", $1, $3);
                        $$->data_type = $1->data_type;
                }else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("GT_OP_FLOAT", new Op_Unary_Astnode("TO_FLOAT", $1), $3);
                        $$->data_type = $1->data_type;
                }else{
                        cout<<"Error at line "<<line<<": type mismatch rel_exp\n";
                        exit(1);   
                }       
        }
        else if(isFloat($1->data_type))
        {
                if(isInt($3->data_type)){
                        $$ = new Op_Binary_Astnode("GT_OP_FLOAT", $1, new Op_Unary_Astnode("TO_FLOAT", $3));    
                        $$->data_type = createDataType(INT_TYPE);
                }
                else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("GT_OP_FLOAT", $1, $3);    
                        $$->data_type = createDataType(INT_TYPE);
                }
                else{
                        cout<<"Error at line "<<line<<": type mismatch in rel\n";
                        exit(1);   
                }
        }else if(isCompSub($1->data_type , $3->data_type)){
                $$ = new Op_Binary_Astnode("GT_OP_INT" , $1, $3);
                $$->data_type = createDataType(INT_TYPE);
        }
        else
        {
                cout<<"Error at line "<<line<<": type mismatch int relational_expression \n";
                exit(1);   
        }    
        $$->lvalue= false;
        // nodeLabel
        if($3->nodeLabel == 0 || $3->nodeLabel == 0){
                $$->nodeLabel = $1->nodeLabel;
        } else if($1->nodeLabel == $3->nodeLabel){
                $$->nodeLabel = $1->nodeLabel+1;
        }else{
                $$->nodeLabel = max($1->nodeLabel,$3->nodeLabel);
        }
}
        | relational_expression LE_OP additive_expression
{
        if(isInt($1->data_type))
        {
                if(isInt($3->data_type)){
                        $$ = new Op_Binary_Astnode("LE_OP_INT", $1, $3);
                        $$->data_type = $1->data_type;
                }else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("LE_OP_FLOAT", new Op_Unary_Astnode("TO_FLOAT", $1), $3);
                        $$->data_type = $1->data_type;
                }else{
                        cout<<"Error at line "<<line<<": type mismatch rel_exp\n";
                        exit(1);   
                }       
        }
        else if(isFloat($1->data_type))
        {
                if(isInt($3->data_type)){
                        $$ = new Op_Binary_Astnode("LE_OP_FLOAT", $1, new Op_Unary_Astnode("TO_FLOAT", $3));    
                        $$->data_type = createDataType(INT_TYPE);
                }
                else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("LE_OP_FLOAT", $1, $3);    
                        $$->data_type = createDataType(INT_TYPE);
                }
                else{
                        cout<<"Error at line "<<line<<": type mismatch in rel\n";
                        exit(1);   
                }
        }else if(isCompSub($1->data_type , $3->data_type)){
                $$ = new Op_Binary_Astnode("LE_OP_INT" , $1, $3);
                $$->data_type = createDataType(INT_TYPE);
        }
        else
        {
                cout<<"Error at line "<<line<<": type mismatch int relational_expression \n";
                exit(1);   
        }    
        $$->lvalue= false;
        // nodeLabel
        if($3->nodeLabel == 0 || $3->nodeLabel == 0){
                $$->nodeLabel = $1->nodeLabel;
        } else if($1->nodeLabel == $3->nodeLabel){
                $$->nodeLabel = $1->nodeLabel+1;
        }else{
                $$->nodeLabel = max($1->nodeLabel,$3->nodeLabel);
        }
}
        | relational_expression GE_OP additive_expression
{
        if(isInt($1->data_type))
        {
                if(isInt($3->data_type)){
                        $$ = new Op_Binary_Astnode("GE_OP_INT", $1, $3);
                        $$->data_type = $1->data_type;
                }else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("GE_OP_FLOAT", new Op_Unary_Astnode("TO_FLOAT", $1), $3);
                        $$->data_type = $1->data_type;
                }else{
                        cout<<"Error at line "<<line<<": type mismatch rel_exp\n";
                        exit(1);   
                }       
        }
        else if(isFloat($1->data_type))
        {
                if(isInt($3->data_type)){
                        $$ = new Op_Binary_Astnode("GE_OP_FLOAT", $1, new Op_Unary_Astnode("TO_FLOAT", $3));    
                        $$->data_type = createDataType(INT_TYPE);
                }
                else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("GE_OP_FLOAT", $1, $3);    
                        $$->data_type = createDataType(INT_TYPE);
                }
                else{
                        cout<<"Error at line "<<line<<": type mismatch in rel\n";
                        exit(1);   
                }
        }else if(isCompSub($1->data_type , $3->data_type)){
                $$ = new Op_Binary_Astnode("GE_OP_INT" , $1, $3);
                $$->data_type = createDataType(INT_TYPE);
        }
        else
        {
                cout<<"Error at line "<<line<<": type mismatch int relational_expression \n";
                exit(1);   
        }    
        $$->lvalue= false;
        // nodeLabel
        if($3->nodeLabel == 0 || $3->nodeLabel == 0){
                $$->nodeLabel = $1->nodeLabel;
        }
        else if($1->nodeLabel == $3->nodeLabel){
                $$->nodeLabel = $1->nodeLabel+1;
        }else{
                $$->nodeLabel = max($1->nodeLabel,$3->nodeLabel);
        }
}
;

additive_expression: 
        multiplicative_expression
{
        $$ = $1;
}
        | additive_expression '+' multiplicative_expression
{
        if(isPointer($1->data_type) && isPointer($3->data_type))
        {
                cout<<"Error at line "<<line<<": cant add two pointer in add_exp\n";
                exit(1);
        }
        if( ( isPointer($1->data_type) && isInt($3->data_type) ) 
                || 
            ( isInt($1->data_type) && isPointer($3->data_type) )
        )
        {
                $$ = new Op_Binary_Astnode("PLUS_INT", $1, $3);
                $$->data_type = isPointer($1->data_type) ? $1->data_type : $3->data_type; 
        }
        else if( 
                ( isArray($1->data_type) && isInt($3->data_type) ) 
                || 
                ( isInt($1->data_type) && isArray($3->data_type) )
        )
        {
                $$ = new Op_Binary_Astnode("PLUS_INT", $1, $3);
                $$->data_type = isArray($1->data_type) ? $1->data_type : $3->data_type; 
        }
        else if(isInt($1->data_type))
        {
                if(isInt($3->data_type)){
                        $$ = new Op_Binary_Astnode("PLUS_INT", $1, $3);
                        $$->data_type = $1->data_type;
                }else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("PLUS_FLOAT", new Op_Unary_Astnode("TO_FLOAT", $1), $3);
                        $$->data_type = $3->data_type;
                }else{
                        cout<<"Error at line "<<line<<": type mismatch in add_exp\n";
                        exit(1);   
                }       
        }
        else if(isFloat($1->data_type))
        {
                if(isInt($3->data_type)){
                        $$ = new Op_Binary_Astnode("PLUS_FLOAT", $1, new Op_Unary_Astnode("TO_FLOAT", $3));
                        $$->data_type = $1->data_type;    
                }else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("PLUS_FLOAT", $1, $3);
                        $$->data_type = $1->data_type;    
                }
                else{
                        cout<<"Error at line "<<line<<": type mismatch in add_exp\n";
                        exit(1);   
                }
        }
        else
        {
                cout<<"Error at line "<<line<<": type mismatch in add_exp \n";
                exit(1);   
        }      
        $$->lvalue=false;  
        //nodelabel
        if($3->nodeLabel == 0 || $3->nodeLabel == 0){
                $$->nodeLabel = $1->nodeLabel;
        }else if($1->nodeLabel == $3->nodeLabel){
                $$->nodeLabel = $1->nodeLabel+1;
        }else{
                $$->nodeLabel = max($1->nodeLabel,$3->nodeLabel);
        }   
}
        | additive_expression '-' multiplicative_expression
{
        if( isPointer($1->data_type) && isInt($3->data_type) )
        {
                $$ = new Op_Binary_Astnode("MINUS_INT", $1, $3);
                $$->data_type = $1->data_type; 
        }
        else if((isInt($1->data_type) || isFloat($1->data_type)) && isPointer($3->data_type) )
        {
                cout<<"Error at line "<<line<<": cant subtract pointer from int/float add_exp\n";
                exit(1); 
        }
        else if( isArray($1->data_type) && isInt($3->data_type)  )
        {
                $$ = new Op_Binary_Astnode("MINUS_INT", $1, $3);
                $$->data_type = $1->data_type; 
        }
        else if(isInt($1->data_type))
        {
                if(isInt($3->data_type)){
                        $$ = new Op_Binary_Astnode("MINUS_INT", $1, $3);
                        $$->data_type = $1->data_type;
                }else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("MINUS_FLOAT", new Op_Unary_Astnode("TO_FLOAT",$1), $3);
                        $$->data_type = $3->data_type;
                }else{
                        cout<<"Error at line "<<line<<": type mismatch in add_exp\n";
                        exit(1);   
                }       
        }
        else if(isFloat($1->data_type))
        {
                if(isInt($3->data_type)){
                        $$ = new Op_Binary_Astnode("MINUS_FLOAT", $1, new Op_Unary_Astnode("TO_FLOAT", $3));
                        $$->data_type = $1->data_type;    
                }
                else if( isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("MINUS_FLOAT", $1, $3);
                        $$->data_type = $1->data_type;
                }
                else{
                        cout<<"Error at line "<<line<<": type mismatch in add_exp\n";
                        exit(1);   
                }
        }
        else if(isCompSub($1->data_type, $3->data_type))
        {
                //$$ = $1; //to be changed
                $$= new Op_Binary_Astnode("MINUS_INT", $1, $3);
                $$->data_type = createDataType(INT_TYPE);
        }
        else
        {
                cout<<"Error at line "<<line<<": type mismatch on in minus add_exp \n";
                exit(1);   
        } 
        $$->lvalue=false;
        // nodeLabel
        if($3->nodeLabel == 0 || $3->nodeLabel == 0){
                $$->nodeLabel = $1->nodeLabel;
        }
        else if($1->nodeLabel == $3->nodeLabel){
                $$->nodeLabel = $1->nodeLabel+1;
        }else{
                $$->nodeLabel = max($1->nodeLabel,$3->nodeLabel);
        }     
}
;

multiplicative_expression: 
        unary_expression
{
        $$ =$1;
}
        | multiplicative_expression '*' unary_expression
{
        if(isInt($1->data_type))
        {
                if(isInt($3->data_type)){
                        $$ = new Op_Binary_Astnode("MULT_INT", $1, $3);
                        $$->data_type = $3->data_type;
                }else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("MULT_FLOAT", new Op_Unary_Astnode("TO_FLOAT", $1), $3);
                        $$->data_type = $3->data_type;
                }else{
                        cout<<"Error at line "<<line<<": type mismatch in mult_exp\n";
                        exit(1);   
                }       
        }
        else if(isFloat($1->data_type))
        {
                if(isInt($3->data_type) ){
                        $$ = new Op_Binary_Astnode("MULT_FLOAT", $1, new Op_Unary_Astnode("TO_FLOAT", $3));
                        $$->data_type = $1->data_type;    
                }
                else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("MULT_FLOAT", $1, $3);
                        $$->data_type = $1->data_type;    
                }
                else{
                        cout<<"Error at line "<<line<<": type mismatch in mul_exp\n";
                        exit(1);   
                }
        }else
        {
                cout<<"Error at line "<<line<<": type mismatch in mult_exp\n";
                exit(1);   
        }           
        $$->lvalue=false;
        // nodelabel
        if($3->nodeLabel == 0 || $3->nodeLabel == 0){
                $$->nodeLabel = $1->nodeLabel;
        }
        else if($1->nodeLabel == $3->nodeLabel){
                $$->nodeLabel = $1->nodeLabel+1;
        }else{
                $$->nodeLabel = max($1->nodeLabel,$3->nodeLabel);
        }     
}
        | multiplicative_expression '/' unary_expression
{
        //check div by zero (but I don't think voh apna kaam hai)
        if(isInt($1->data_type))
        {
                if(isInt($3->data_type)){
                        $$ = new Op_Binary_Astnode("DIV_INT", $1, $3);
                        $$->data_type = $3->data_type;
                }else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("DIV_FLOAT", new Op_Unary_Astnode("TO_FLOAT" ,$1), $3);
                        $$->data_type = $3->data_type;
                }else{
                        cout<<"Error at line "<<line<<": type mismatch in div op\n";
                        exit(1);   
                }       
        }
        else if(isFloat($1->data_type))
        {
                if(isInt($3->data_type) ){
                        $$ = new Op_Binary_Astnode("DIV_FLOAT", $1, new Op_Unary_Astnode("TO_FLOAT", $3));
                        $$->data_type = $1->data_type;    
                }
                else if(isFloat($3->data_type)){
                        $$ = new Op_Binary_Astnode("DIV_FLOAT", $1, $3);
                        $$->data_type = $1->data_type;    
                }
                else{
                        cout<<"Error at line "<<line<<": type mismatch in mul_exp\n";
                        exit(1);   
                }
        }else
        {
                cout<<"Error at line "<<line<<": type mismatch in mult_exp\n";
                exit(1);   
        }           
        $$->lvalue=false; 
        // nodelabel
        if($3->nodeLabel == 0 || $3->nodeLabel == 0){
                $$->nodeLabel = $1->nodeLabel;
        }
        else if($1->nodeLabel == $3->nodeLabel){
                $$->nodeLabel = $1->nodeLabel+1;
        }else{
                $$->nodeLabel = max($1->nodeLabel,$3->nodeLabel);
        }    
}
;

unary_expression: 
        postfix_expression
{
        $$ = $1;
}
        | unary_operator unary_expression
{
        if($1->getOp() == "DEREF")
        {
                if($2->data_type.ref){
                        $$ = new Op_Unary_Astnode($1->getOp() , $2);
                        $$->data_type = derefDataType($2->data_type);
                }
                else if($2->data_type.star == 0 && $2->data_type.arr.size() == 0)
                {       
                        cout<<"Error at line "<<line<<": invalid operand type "<<$2->data_type.type_name<<" for * in unary_expression\n";
                        exit(1);
                }
                else if($2->data_type.arr.size()){
                /*        
                        DataType datatype = $2->data_type;
                        std::vector<int> req = datatype.arr;
                        req.erase(req.begin());
                        $$ = new Op_Unary_Astnode($1->getOp(), $2);
                        $$->data_type=$2->data_type;
                        $$->data_type.arr = req; 
                */
                        $$ = new Op_Unary_Astnode($1->getOp(), $2);
                        $$->data_type = derefDataType($2->data_type);
                        
                }
                else if($2->data_type.star){
                        $$ = new Op_Unary_Astnode($1->getOp(), $2);
                        $$->data_type = derefDataType($2->data_type);
                }
                if(!isArray($$->data_type)){
                        $$->lvalue = true;
                }
        }
        else if($1->getOp() == "UMINUS")
        {
                if($2->data_type.ref){
                        cout<<"Error at line "<<line<<": invalid operand type "<<$2->data_type.type_name<<" for \"unary -\" in unary_expression\n";
                        exit(1);
                }
                if(!isInt($2->data_type) && !isFloat($2->data_type))
                {       
                        cout<<"Error at line "<<line<<": invalid operand type "<<$2->data_type.type_name<<" for for \"unary -\" in unary_expression\n";
                        exit(1);
                }
                $$ = new Op_Unary_Astnode($1->getOp(), $2);
                $$->data_type =  $2->data_type;
                $$->lvalue = false;
        }
        else if($1->getOp() == "NOT")
        {
                $$ = new Op_Unary_Astnode($1->getOp(), $2);
                $$->data_type = createDataType(INT_TYPE);
                $$->lvalue = false;
        }
        else if($1->getOp() == "ADDRESS")
        {
                if($2->data_type.isConst){
                        cout<<"Error at line "<<line<<": operand of \"&\" doesn't have lvalue in unary expression\n";
                        exit(1);
                }
                $$ = new Op_Unary_Astnode($1->getOp(), $2);
                $$->data_type = refDataType($2->data_type);
                $$->lvalue = false;

        }
        // nodeLabel
        $$->nodeLabel = $2->nodeLabel+1;
        
}
;

// is fine
unary_operator: 
        '-'
{
        $$ = new Op_Unary_Astnode("UMINUS");
}
        | '!'
{
        $$ = new Op_Unary_Astnode("NOT");
}
        | '&'
{
        $$ = new Op_Unary_Astnode("ADDRESS");
}        
        | '*'
{
        $$ = new Op_Unary_Astnode("DEREF");
}
;


// tbdl
postfix_expression: 
        primary_expression
{
        $$ = $1;
}
        | postfix_expression '[' expression ']'
{
        if(!isInt($3->data_type))
        {
                cout<<"Error at line "<<line<<": in postfix array_type Array subscript is not an integer\n";
                exit(1);    
        }
        if($1->data_type.ref){
                $$ = new Arrayref_Astnode($1 , $3);
                $$->data_type = derefDataType($1->data_type); //I think
        }
        else if($1->data_type.arr.size() != 0 )
        {
                $$ = new Arrayref_Astnode($1, $3);
                //$$->data_type = $1->data_type;
                //$$->data_type.arr.erase($$->data_type.arr.begin());   //i don't understand , till now why was it working ? shouldn't we also update size, type_name and all ?  
                $$->data_type = derefDataType($1->data_type) ;  //twenty_march
                if($$->data_type.arr.size() == 0){
                        $$->lvalue = true;
                }
        }
        else if($1->data_type.star != 0)
        {
                $$ = new Arrayref_Astnode($1, $3);
                $$->data_type = derefDataType($1->data_type);
        }
        else
        {
                cout<<"Error at line "<<line<<": postfix array_type Subscripted value is neither array nor pointer\n";
                exit(1);
        }
}
        | IDENTIFIER '(' ')'
{
        SymbTabEntry *func = gst.find($1);
        if(func == NULL){
                cout<<"Error at line "<<line<<": Func "<<$1<<" not defined function call\n";
                exit(1);
        }
        if(func->var_fun_type != "fun"){
                cout<<"Error at line "<<line<<": Object called "<<$1<<" not a function  function call\n";
                exit(1);  
        }
        if(func->symtab->params.size() != 0){
                cout<<"Error at line "<<line<<": too many argument  function call\n"; 
                exit(1);
        }
        $$ = new Funcall_Astnode(new Identifier_Astnode($1));
        $$->data_type = func->datatype;
        $$->lvalue=false;  
        // $$->nodeLabel = 1; // phase 3                  
}
        | IDENTIFIER '(' expression_list ')'
{
        SymbTabEntry *func = gst.find($1);
        if(func == NULL){
                cout<<"Error at line "<<line<<": Func "<<$1<<" not defined\n";
                exit(1);
        }
        if(func->var_fun_type != "fun"){
                cout<<"Error at line "<<line<<": procedure_call Object called "<<$1<<" not a function\n";
                exit(1);  
        }
        if(func->symtab->params.size() < $3->nodes.size()){
                cout<<"Error at line "<<line<<": too few argument \n"; 
                exit(1);
        }
        if(func->symtab->params.size() > $3->nodes.size()){
                cout<<"Error at line "<<line<<": function call too many argument \n"; 
                exit(1);
        }
        SymbTab *symtab = func->symtab;
        string param;
        int n = (int)$3->nodes.size();
        for(int i=0; i<(int)$3->nodes.size(); i++){
                param = symtab->params[n-i-1];
                //cout<<i<<"\t"<<param<<"\n";
                //cout<<symtab->stes[param]->datatype.type_name<<endl;
                //cout<< $3->nodes[i]->data_type.type_name<<endl;
                
                if(!isCompParam(symtab->stes[param]->datatype, $3->nodes[i]->data_type))
                {
                        //cout<<"Error at line "<<line<<": parameter "<<i<<" is in compatible \n";
                        cout<<"Error at line "<<line<<":  fun_call expected "<<symtab->stes[param]->datatype.type_name<<" but argument is of type "<<$3->nodes[i]->data_type.type_name<<"\n";
                        exit(1);
                }
                if(isFloat(symtab->stes[param]->datatype) && isInt($3->nodes[i]->data_type)){
                        $3->nodes[i] = new Op_Unary_Astnode("TO_FLOAT", $3->nodes[i]);
                }
                // check if the parameters passes are type correct of not 
                // else cout<<"Error at line "<<line<<": type mismatch of parameter "<<i+1<<endl;
        }
        
        $3->setIdentifier(new Identifier_Astnode($1));
        $$ = $3;
        $$->data_type = func->datatype;
        $$->lvalue=false;   
        // $$->nodeLabel = 1; // phase 3         
}
        | postfix_expression '.' IDENTIFIER
{
        if($1->data_type.ref){
                cout<<"Error at line "<<line<<": cant determine membership of pointer in postfix mem in ";
                exit(1);
        }
        if(isPointer($1->data_type) || isArray($1->data_type))
        {
                cout<<"Error at line "<<line<<": cant determine membership of pointer in postfix mem in ";
                exit(1);
        }
        string base = $1->data_type.base_type_name;
        if(gst.find(base) == NULL  || gst.find(base)->var_fun_type != "struct")
        {
                cout<<"Error at line "<<line<<": no sturct found in postfix mem\n";
                exit(1);
        }

        SymbTab* local = gst.find(base)->symtab;
        SymbTabEntry* ste = local->find($3);

        if(local->find($3) == NULL)
        {
                cout<<"Error at line "<<line<<": \""<<base<<"\" has no member \""<<$3<<"\" in postfix mem \n";
                exit(1);
        }
        $$ = new Member_Astnode($1, new Identifier_Astnode($3));
        $$->data_type = ste->datatype;
        //$$->lvalue = true;
        $$->lvalue = $1->lvalue ; 
}
        | postfix_expression PTR_OP IDENTIFIER
{
        if($1->data_type.ref){
                cout<<"Error at line "<<line<<":Invalid operand for arrow operation postfix mem \n";
                exit(1);
        }        
        else if($1->data_type.star + (int)$1->data_type.arr.size() != 1)
        {
                cout<<"Error at line "<<line<<": base operand of '->' is a non pointer type in postfix ptr\n";
                exit(1);
        }
        string base = $1->data_type.base_type_name;
        if(gst.find(base) == NULL)
        {
                cout<<"Error at line "<<line<<": no sturct found in postfix ptr\n";
                exit(1);
        }

        SymbTab* local = gst.find(base)->symtab;
        SymbTabEntry* ste = local->find($3);

        if(local->find($3) == NULL)
        {
                cout<<"Error at line "<<line<<": \""<<base<<"\" has no member \""<<$3<<"\" at postfix ptr\n";
                exit(1);
        }
        $$ = new Arrow_Astnode($1, new Identifier_Astnode($3));
        $$->data_type = ste->datatype;
        $$->lvalue = $1->lvalue ; 

}
        | postfix_expression INC_OP
{
        if(!$1->lvalue)
        {
                cout<<"Error at line "<<line<<": inc_op operand does not have lvalue\n";
                exit(1);
        }
        if(!isInt($1->data_type) && !isFloat($1->data_type) && !isPointer($1->data_type)) {
                cout<<"Error at line "<<line<<": invalid increment operation in postfix inc\n";
                exit(1);
        }
        $$ = new Op_Unary_Astnode("PP", $1);
        $$->data_type = $1->data_type;
        $$->lvalue=false;     
}
;

primary_expression: 
        IDENTIFIER
{
        if(st->find($1) != NULL )
        {
                if(st->stes[$1]->var_fun_type == "var")
                {
                        $$ = new Identifier_Astnode($1);
                        $$->data_type = st->find($1)->datatype;
                        if($$->data_type.arr.size()){
                                $$->lvalue = false;
                        }else{
                                $$->lvalue = true;
                        }
                }else
                {
                        cout<<"Error at line "<<line<<": identifier "<<$1<<" not a variable\n";
                        exit(1);
                }
        }
        else
        {
                cout<<"Error at line "<<line<<": identifier "<<$1<<" not defined\n";
                exit(1);
        }
        //nodelabel
        $$->nodeLabel = 1;
}
        | INT_CONSTANT
{
        $$ = new Intconst_Astnode($1);
        $$->data_type = createDataType(INT_TYPE);
        if(stoi($1) == 0){
                $$->data_type.val = 0;
        }
        $$->rvalue = true;
        $$->data_type.isConst = true;
        // nodelabel
        $$->nodeLabel = 0;
}
        | FLOAT_CONSTANT
{
        $$ = new Floatconst_Astnode($1);
        $$->data_type = createDataType(FLOAT_TYPE);
        $$->rvalue = true;
        $$->data_type.isConst = true;

}
        | STRING_LITERAL
{
        $$ = new Stringconst_Astnode($1);
        $$->data_type = createDataType(STRING_LIT_TYPE);
        $$->rvalue = true;
        $$->data_type.isConst = true;
}
        | '(' expression ')'
{
        $$ = $2;
}
;


expression_list: 
        expression
{
        $$= new Funcall_Astnode();
        $$->push($1);
}
        | expression_list ',' expression
{
        $$= $1;
        $$->push($3);
}
;

selection_statement: 
        IF '(' expression ')' statement ELSE statement
{
        $$ = new If_Astnode($3,$5,$7);
}
;

iteration_statement: 
        WHILE '(' expression ')' statement
{
        $$= new While_Astnode($3,$5);
        }
        | FOR '(' assignment_expression ';' expression ';' assignment_expression ')' statement
{
        $$= new For_Astnode($3,$5,$7,$9);
}
;


%%
void IPL::Parser::error( const location_type &l, const std::string &err_message )
{
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}


