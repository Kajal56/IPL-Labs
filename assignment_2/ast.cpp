#pragma once
// #include<bits/stdc++.h>
// #include "ast.hh"
// using namespace std;

bool isPointer(DataType type){
    if(type.star){
        return true;
    }
    return false;
}

bool isArray(DataType type){
    if(type.arr.size()){
        return true;
    }
    return false;
}
bool isInt(DataType type){
    if(type.star == 0 && type.arr.size() == 0 && type.base_type == INT_TYPE){
        if(type.ref != 0) return false;
        return true;
    }else{
        return false;
    }
}

bool isFloat(DataType type){
    if(type.star == 0 && type.arr.size() == 0 && type.base_type == FLOAT_TYPE){
        if(type.ref) return false;
        return true;
    }else{
        return false;
    }
}
bool isVoidPtr(DataType d){
    if(d.base_type_name == "void" && d.star == 1 && !isArray(d)) return true;
    return false;
}
bool isNullPtr(DataType d){
    if(d.star >=1 && d.val ==0) return true;
    return false;
}

bool isCompatible(DataType d1, DataType d2){
    if(d1.ref != d2.ref) return false;
    if(d1.ref){
        return false;
        //but nvm, it'll be handled by parser
    }
    // if(isArray(d1)){
    //     return false;  // not sure if it's needed
    // }
    if(isArray(d1)) return false;
    
    if(d1.type_name == d2.type_name){
        return true;
    }
    if((isInt(d1) || isFloat(d1)) && (isInt(d2) || isFloat(d2))){
        return true;
    }
    // if(d1.star == (int)(d2.arr.size()) + d2.star && d1.base_type == d2.base_type && d1.arr.size() == 0){
    //     return true;
    // }
    if(d1.star == (int)(d2.arr.size()) + d2.star && d1.base_type == d2.base_type && d1.arr.size() == 0 && d1.star - d2.star == 1){
        return true;
    }
    if(isPointer(d1) && d1.star==1 && d1.base_type_name=="void" && (isPointer(d2)||isArray(d2))){
        return true;
    }
    if(isPointer(d1) && (!isArray(d1)) && isPointer(d2) && d2.star==1 && d2.base_type_name=="void"){
        return true;
    }
    return false;
}
bool isCompSub(DataType d1, DataType d2){
    // if(d1.star == (int)(d2.arr.size()) + d2.star && d1.base_type == d2.base_type && d1.arr.size() == 0){
    //     return true;
    // }
    // if(d1.ref!=d2.ref) return false;
    if(d1.base_type != d2.base_type) return false;

    if(d1.ref && d1.star == d2.star && d2.arr.size() == d1.arr.size()+1) {
        for(int i=1; i< int(d2.arr.size()) ; i++ ){
            if(d1.arr[i-1] !=d2.arr[i]) return false;
        }
        return true;
    }
    if(d2.ref && d2.star == d1.star && d1.arr.size() == d2.arr.size()+1) {
        for(int i=1; i< int(d1.arr.size()) ; i++ ){
            if(d2.arr[i-1] !=d1.arr[i]) return false;
        }
        return true;
    }
    if(d1.ref){
        if(d1.star != d2.star) return false;
        if(d1.arr.size() != d2.arr.size()) return false;
        for(int i=0; i< int(d1.arr.size()); i++){
            if(d1.arr[i] != d2.arr[i]) return false;
        }
        return true;
    }
    if(d1.star==d2.star){
        if(int(d1.arr.size())==int(d2.arr.size()) ){
                int n=int(d1.arr.size());
                if(n>0){
                    for(int i=1;i<n;i++){
                        if(d1.arr[i]!=d2.arr[i]){
                            return false;
                        }
                    }
                    return true;
                }else{
                    return true;
                }      
            }
        else{
            return false;
        }
    }
    else if(d1.star-d2.star==1 && int(d2.arr.size())==1 && int(d1.arr.size())==0){
        return true;
    }
    else if(d2.star-d1.star==1 && int(d1.arr.size())==1 && int(d2.arr.size())==0){
        return true;
    }
    return false;
}



bool isCompParam(DataType param, DataType exp){
    // if(d1.star == (int)(d2.arr.size()) + d2.star && d1.base_type == d2.base_type && d1.arr.size() == 0){
    //     return true;
    // }
    if(param.type_name=="void*"){
        if(isPointer(exp) || isArray(exp) || exp.ref) return true; // as long as parameter is of void* type, any pointer given as argument would work
    }
    if(exp.type_name=="void*"){
        if((isPointer(param) || isArray(param)) && param.ref == 0 ) return true; // as long as expression is of void* type, any parameter type  would work
    }
    if(isInt(exp) && isFloat(param)) return true;
    // if(exp.ref) return false;
    
    if(exp.base_type!=param.base_type) return false;
    if(exp.type_name==param.type_name) return true;
    if(!isArray(param)){
        if((param.star == exp.star + int(exp.arr.size())) && (int(exp.arr.size()) == 1)) return true;
    }
    if(exp.ref && exp.star == param.star && int(param.arr.size())-int(exp.arr.size()) == 1)
    {
        for(int i=1; i< (int)param.arr.size() ; i++){
            if(param.arr[i]!=exp.arr[i-1]) return false;
        }
        return true;
    }
    
    if(exp.ref) return false;

    if(exp.star==param.star){      //if number of stars are same, first size can vary..others need to be exact same
        if(int(exp.arr.size())==int(param.arr.size()) ){
                int n=int(exp.arr.size());
                if(n>0){
                    for(int i=1;i<n;i++){
                        if(exp.arr[i]!=param.arr[i]){
                            return false;
                        }
                    }
                    return true;
                }
            }
        else{
            return false;
        }

    }
    //first index is interchangable to star or any other integer
    else if(exp.star-param.star==1 && int(param.arr.size())-int(exp.arr.size())==1){
        // return true;
        for(int i=1;i<int(param.arr.size());i++){
            if(param.arr[i]!=exp.arr[i-1]) return false;
        }
        return true;
    }
    // else if(param.star-exp.star==1 && int(exp.arr.size())-int(param.arr.size())==1){
    //     // return true;
    //     for(int i=1;i<int(exp.arr.size());i++){
    //         if(exp.arr[i]!=param.arr[i-1]) return false;
    //     }
    //     return true;        
    // }
    return false;
}


Abstract_Astnode::Abstract_Astnode(){
    this->lvalue = false;
    this->rvalue = false;
}

// EMPTY
Empty_Astnode::Empty_Astnode(){
    this->astnode_type = EMPTY_ASTNODE;
}
void Empty_Astnode::print(int blanks){
    string tabs(blanks, ' ');
    // cout<<tabs<<"\"\" : \"empty\" \n}" ;
    cout<<"\"empty\"";
}

// SEQ
Seq_Astnode::Seq_Astnode(){
    this->astnode_type = SEQ_ASTNODE;
}
void Seq_Astnode::push(Statement_Astnode* ptr){
    seq.push_back(ptr);
}
void Seq_Astnode:: print(int blanks){
    string tabs(blanks, ' ');
    cout<<tabs<<"\"seq\" : [\n";
    string tab1 = tabs+"  ";
    int n=seq.size();
    for(int i=0;i<n;i++){

        if(seq[i]->astnode_type==EMPTY_ASTNODE){
            seq[i]->print(blanks+4);
            if(i!=n-1){
                cout<<",\n";
            }else{
                cout<<"\n";
            }
        }
        else{
            cout<<tab1<<"{\n";
            seq[i]->print(blanks+4);
            if(i!=n-1){
                cout<<"\n},\n";
            }else{
                cout<<"\n}\n";
            }
        }
    }
    cout<<tabs<<"]";
}

// ASSIGNS
AssignS_Astnode::AssignS_Astnode(Exp_Astnode* left, Exp_Astnode* right){
    this->astnode_type = ASSIGNS_ASTNODE;
    this->left = left;
    this->right = right;
}
void AssignS_Astnode:: print(int blanks){
    string tabs(blanks, ' ');
    cout<<tabs<<"\"assignS\" : {\n";
    string tab1 = tabs+"  ";
    cout<<tab1<<"\"left\": {\n";
    this->left->print(blanks+4);
    cout<<tab1<<"\n},\n";
    cout<<tab1<<"\"right\": {\n";
    this->right->print(blanks+4);
    cout<<tab1<<"\n}\n";
    cout<<tabs<<"}";
}

// FOR
For_Astnode::For_Astnode(
    Exp_Astnode* init,
    Exp_Astnode* cond,
    Exp_Astnode* update,
    Statement_Astnode * stmts
){
    this->astnode_type = FOR_ASTNODE;
    this->init=init;
    this->guard=cond;
    this->step=update;
    this->body=stmts;
}
void For_Astnode:: print(int blanks){
    string tabs(blanks, ' ');
    string tab1 = tabs+" ";
    cout<<tabs<<"\"for\": {\n";
    cout<<tab1<<"\"init\": {\n";
    this->init->print(blanks+4);
    cout<<tab1<<"\n},\n";
    cout<<tab1<<"\"guard\": {\n";
    this->guard->print(blanks+4);
    cout<<tab1<<"\n},\n";
    cout<<tab1<<"\"step\": {\n";
    this->step->print(blanks+4);
    cout<<tab1<<"\n},\n";
    if(this->body->astnode_type == EMPTY_ASTNODE){
        cout<<tab1<<"\"body\": \"empty\" \n";
    }else{
        cout<<tab1<<"\"body\": {\n";
        this->body->print(blanks+4);
        cout<<tab1<<"\n}\n";
    }
    cout<<tabs<<"}";
}

// IF
If_Astnode::If_Astnode(Exp_Astnode* cond, Statement_Astnode* true_stmts,Statement_Astnode* flase_stmts){
    this->astnode_type = IF_ASTNODE;
    this->cond=cond;
    this->then_sts=true_stmts;
    this->else_sts=flase_stmts;
}
void If_Astnode:: print(int blanks){
    string tabs(blanks, ' ');
    string tab1 = tabs+" ";
    cout<<tabs<<"\"if\": {\n";
    cout<<tab1<<"\"cond\": {\n";
    this->cond->print(blanks+4);
    cout<<tab1<<"\n},\n";
    cout<<tab1<<"\"then\": {\n";
    this->then_sts->print(blanks+4);
    cout<<tab1<<"\n},\n";
    // cout<<tab1<<"\"else\": {\n";
    // this->else_sts->print(blanks+4);
    // cout<<tab1<<"\n}\n";
    if(this->else_sts->astnode_type == EMPTY_ASTNODE){
        cout<<tab1<<"\"else\": \"empty\" \n";
    }else{
        cout<<tab1<<"\"else\": {\n";
        this->else_sts->print(blanks+4);
        cout<<tab1<<"\n}\n";
    }
    cout<<tabs<<"}";
}

// WHILE
While_Astnode::While_Astnode(Exp_Astnode* exp,Statement_Astnode* stmts){
    this->astnode_type = WHILE_ASTNODE;
    this->cond=exp;
    this->stmt=stmts;
}
void While_Astnode:: print(int blanks){
    string tabs(blanks, ' ');
    string tab1 = tabs+" ";
    cout<<tabs<<"\"while\": {\n";
    cout<<tab1<<"\"cond\": {\n";
    this->cond->print(blanks+4);
    cout<<tab1<<"\n},\n";
    cout<<tab1<<"\"stmt\": {\n";
    this->stmt->print(blanks+4);
    cout<<tab1<<"\n}\n";
    cout<<tabs<<"}";
}

// RETURN
Return_Astnode::Return_Astnode(Exp_Astnode* exp){
    this->astnode_type = RETURN_ASTNODE;
    this->exp= exp;
}
void Return_Astnode:: print(int blanks){
    string tabs(blanks, ' ');
    cout<<tabs<<"\"return\" : {\n";
    exp->print(blanks+2);
    cout<<tabs<<"}";
}

// SEQ
Proccall_Astnode::Proccall_Astnode(vector<Exp_Astnode*> seq){
    this->astnode_type = FUNCALL_ASTNODE;
    this->seq=seq;
}
Proccall_Astnode::Proccall_Astnode(Identifier_Astnode* pname, vector<Exp_Astnode*> seq){
    this->astnode_type = FUNCALL_ASTNODE;
    this->pname=pname;
    this->seq=seq;
}
void Proccall_Astnode::push(Exp_Astnode* ptr){
    this->pname = new Identifier_Astnode("");
    seq.push_back(ptr);
}

void Proccall_Astnode:: print(int blanks){
    string tabs(blanks, ' ');
    cout<<tabs<<"\"proccall\" : {\n";
    string tab1 = tabs+"  ";
    int n=seq.size();
    cout<<tab1<<"\"fname\":{\n";
    this->pname->print(blanks+4);
    cout<<tab1<<"\n},\n";
    cout<<tab1<<"\"params\":[\n";
    string tab2 = tab1+"  ";
    for(int i=0;i<n;i++){
        cout<<tab2<<"{\n";
        seq[i]->print(blanks+4);
        if(i!=n-1){
            cout<<"\n},\n";
        }else{
            cout<<"\n}\n";
        }
    }
    cout<<tab1<<"]\n";
    cout<<tabs<<"}";
}

// Binay op
Op_Binary_Astnode::Op_Binary_Astnode(string op, Exp_Astnode* lnode, Exp_Astnode* rnode){;
    this->astnode_type = OP_BINARY_ASTNODE;
    this->op = op;
    this->left = lnode;
    this->right = rnode;
}
void Op_Binary_Astnode::print(int blanks){
    string tabs(blanks, ' ');
    cout<<tabs<<"\"op_binary\": {\n";
    string tab1 = tabs+"  ";
    cout<<tab1<<"\"op\": "<<"\""<<this->op<<"\",\n";
    cout<<tab1<<"\"left\": {\n";
    this->left->print(blanks+4);
    cout<<tab1<<"\n},\n";
    cout<<tab1<<"\"right\": {\n";
    this->right->print(blanks+4);
    cout<<tab1<<"\n}\n";
    cout<<tabs<<"}";
}

// unary op
Op_Unary_Astnode::Op_Unary_Astnode(string op, Exp_Astnode* node){
    this->astnode_type = OP_UNARY_ASTNODE;
    this->op = op;
    this->child = node;
}
Op_Unary_Astnode::Op_Unary_Astnode(string op){
    this->astnode_type = OP_UNARY_ASTNODE;
    this->op = op;
    this->child = NULL;
}
string Op_Unary_Astnode::getOp(){
    return this->op;
}
void Op_Unary_Astnode::print(int blanks){
    string tabs(blanks, ' ');
    cout<<tabs<<"\"op_unary\": {\n";
    string tab1 = tabs+"  ";
    cout<<tab1<<"\"op\": "<<"\""<<this->op<<"\",\n";
    cout<<tab1<<"\"child\": {\n";
    if(this->child != NULL){
        this->child->print(blanks+4);
    }
    cout<<tab1<<"\n}\n";
    cout<<tabs<<"}";
}

// assign
AssignE_Astnode::AssignE_Astnode(Exp_Astnode* lnode, Exp_Astnode* rnode){
    this->astnode_type = ASSIGNE_ASTNODE; 
    this->left = lnode;
    this->right = rnode;
}
void AssignE_Astnode::print(int blanks){
    string tabs(blanks, ' ');
    string tab1 = tabs+"  "; 
    cout<<"\"assignE\": {\n";
    cout<<tab1<<"\"left\": {\n";
    this->left->print(blanks+4);
    cout<<tab1<<"\n},\n";
    cout<<tab1<<"\"right\": {\n";
    this->right->print(blanks+4);
    cout<<tab1<<"\n}\n";
    cout<<tabs<<"}";
}

// indentifier
Identifier_Astnode::Identifier_Astnode(string str){
    this->astnode_type = IDENTIFIER_ASTNODE;
    this->id_str = str;
    this->isStruct = false;
}
Identifier_Astnode::Identifier_Astnode(bool isStruct, string str){
    this->astnode_type = IDENTIFIER_ASTNODE;
    this->id_str = str;
    this->isStruct = isStruct;
}
void Identifier_Astnode::print(int blanks){
    string tabs(blanks, ' ');
    // if(this->data_type.base_type_name.substr(0, 6) == "struct"){
    //     string tab1 = tabs+"  ";
    //     cout<<tabs<<"\"struct\": {\n";
    //     cout<<tab1<<"\"identifier\": "<<"\""<<this->id_str<<"\"\n";
    //     cout<<tabs<<"}";
    // }else{
        cout<<tabs<<"\"identifier\": "<<"\""<<this->id_str<<"\"";
    // }
}

// funcall 
Funcall_Astnode::Funcall_Astnode(Identifier_Astnode* fname){
    this->astnode_type = FUNCALL_ASTNODE;
    this->fname = fname;
}
int Funcall_Astnode::push(Exp_Astnode* node){
    this->nodes.push_back(node);
    return 1;
}
void Funcall_Astnode::setIdentifier(Identifier_Astnode* id){
    this->fname = id;
}
void Funcall_Astnode::print(int blanks){\
    string tabs(blanks, ' ');
    string tab1 = tabs+"  ";
    cout<<"\"funcall\": {\n";
    cout<<tabs<<"\"fname\": {\n";
    this->fname->print(blanks+4);
    cout<<tab1<<"},\n";
    cout<<tab1<<"\"params\": [\n";
    string tab2 = tab1+"  ";
    int n = nodes.size();
    for(int i=0; i<n; i++){
        cout<<tab2<<"{\n";
        this->nodes[i]->print(blanks+4);
        if(i == n-1) cout<<tabs<<"\n}\n";
        else cout<<tab2<<"\n},\n";
    } 
    cout<<tab1<<"]\n";
    cout<<tabs<<"}";
}

// intconstant 
Intconst_Astnode::Intconst_Astnode(string val){
  this->astnode_type = INTCONSTANT_ASTNODE;
  this->val = stoi(val);
}
void Intconst_Astnode::print(int blanks){
  string tabs(blanks, ' ');
  cout<<tabs<<"\"intconst\": "<<this->val;
}

// floatconstant
Floatconst_Astnode::Floatconst_Astnode(string val){
  this->astnode_type = FLOATCONSTANT_ASTNODE; 
  this->val = stof(val);
}
void Floatconst_Astnode::print(int blanks){
  string tabs(blanks, ' ');
//   cout<<"--------------------\nThe values is : "<<this->val<<"\n------------------\n";
  cout<<tabs<<"\"floatconst\": "<<this->val;
}

// string constant
Stringconst_Astnode::Stringconst_Astnode(string str){
    this->astnode_type = STRINGCONSTANT_ASTNODE; 
    this->str = str;
}
void Stringconst_Astnode::print(int blanks){
    string tabs(blanks, ' ');
    cout<<tabs<<"\"stringconst\": "<<this->str;
}

// pointer 
Pointer_Astnode::Pointer_Astnode(Exp_Astnode* node){
    this->astnode_type = POINTER_ASTNODE;
    this->node = node;
}
void Pointer_Astnode::print(int blanks){
    string tabs(blanks, ' ');
    cout<<tabs<<"\"pointer\": {\n";
    this->node->print(blanks+2);
    cout<<"\n";
    cout<<"}";
}

// array ref
Arrayref_Astnode::Arrayref_Astnode(Exp_Astnode* lnode, Exp_Astnode* rnode){
    this->astnode_type = ARRAYREF_ASTNODE; 
    this->array = lnode;
    this->index = rnode;
}
void Arrayref_Astnode::print(int blanks){
    string tabs(blanks, ' ');
    cout<<"\"arrayref\": {\n";
    string tab1 = tabs+"  ";
    cout<<tab1<<"\"array\": {\n";
    this->array->print(blanks+4);
    cout<<tab1<<"\n},\n";
    cout<<tab1<<"\"index\": {\n";
    this->index->print(blanks+4);
    cout<<tab1<<"\n}\n";
    cout<<tabs<<"}";
}


// below are left for concerned user
// DEREF
Deref_Astnode::Deref_Astnode(Exp_Astnode* node){
  this->astnode_type = DEREF_ASTNODE;
  this->node = node;
}
void Deref_Astnode::print(int blanks){
    string tabs(blanks, ' ');
    cout<<tabs<<"\"deref\": {\n";
    this->node->print(blanks+2);
    cout<<"\n";
    cout<<"}";
}

// MEMBER
Member_Astnode::Member_Astnode(Exp_Astnode* lnode, Identifier_Astnode* rnode){
    this->astnode_type = MEMBER_ASTNODE; 
    this->lnode = lnode;
    this->rnode = rnode;
}
void Member_Astnode::print(int blanks){
    string tabs(blanks, ' ');
    string tab1 = tabs+"  ";
    cout<<tabs<<"\"member\": {\n";
    cout<<tab1<<"\"struct\": {\n";
    this->lnode->print(blanks+4);
    cout<<"\n";
    cout<<tab1<<"}";
    cout<<",\n";
    cout<<tab1<<"\"field\": {\n";
    this->rnode->print(blanks+4);
    cout<<"\n";
    cout<<tab1<<"}\n";
    cout<<tabs<<"}";
}

// ARROW
Arrow_Astnode::Arrow_Astnode(Exp_Astnode* lnode, Identifier_Astnode* rnode){
    this->astnode_type = ARROW_ASTNODE; 
    this->lnode = lnode;
    this->rnode = rnode;
}
void Arrow_Astnode::print(int blanks){
    string tabs(blanks, ' ');
    string tab1 = tabs+"  ";
    cout<<tabs<<"\"arrow\": {\n";
    cout<<tab1<<"\"pointer\": {\n";
    this->lnode->print(blanks+4);
    cout<<"\n";
    cout<<tab1<<"}";
    cout<<",\n";
    cout<<tab1<<"\"field\": {\n";
    this->rnode->print(blanks+4);
    cout<<tab1<<"\n}\n}";
}

// int main(){
//     Exp_Astnode* a = new Identifier_Astnode("a");
//     Exp_Astnode* b = new Identifier_Astnode("b");
//     Abstract_Astnode* comp = new Op_Binary_Astnode("some", a, b);
//     comp->print(0);
// }