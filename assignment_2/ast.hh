#pragma once
#include<bits/stdc++.h>
#include "dec.hh"
using namespace std;

enum TypeExp {
    ABSTRACT_ASTNODE,
    REF_ASTNODE,
    OP_BINARY_ASTNODE,
    OP_UNARY_ASTNODE,
    ASSIGNE_ASTNODE,
    FUNCALL_ASTNODE,
    INTCONSTANT_ASTNODE,
    FLOATCONSTANT_ASTNODE,
    STRINGCONSTANT_ASTNODE,
    POINTER_ASTNODE,
    IDENTIFIER_ASTNODE,
    ARRAYREF_ASTNODE,
    DEREF_ASTNODE,
    MEMBER_ASTNODE,
    ARROW_ASTNODE,
    EMPTY_ASTNODE,
    SEQ_ASTNODE,
    ASSIGNS_ASTNODE,
    RETURN_ASTNODE,
    IF_ASTNODE,
    WHILE_ASTNODE,
    FOR_ASTNODE
};

class Abstract_Astnode{
    public:
        bool lvalue;
        bool rvalue;
        // bool isStatementAstNodeEmpty;
        Abstract_Astnode();
        enum TypeExp astnode_type;
        DataType data_type;
        virtual void print(int blanks)=0;
};

class Statement_Astnode: public Abstract_Astnode {
    public: 
};

class Exp_Astnode: public Abstract_Astnode {
};

class Ref_Astnode: public Exp_Astnode { 
};

class Empty_Astnode: public Statement_Astnode{
    public:
    Empty_Astnode();
    void print(int blanks);
};

class Identifier_Astnode;

class Seq_Astnode: public Statement_Astnode{
    public:
    vector<Statement_Astnode*> seq;
    Seq_Astnode();
    void push(Statement_Astnode* ptr);
    void print(int blanks);
};

class AssignS_Astnode: public Statement_Astnode{
    public:
    Exp_Astnode* left;
    Exp_Astnode* right;
    AssignS_Astnode(Exp_Astnode* left, Exp_Astnode* right);
    void print(int blanks);
};

class For_Astnode: public Statement_Astnode{
    public:
    // int blanks;
    Exp_Astnode* init;
    Exp_Astnode* guard;
    Exp_Astnode* step;
    Statement_Astnode* body;
    void print(int blanks);
    For_Astnode(Exp_Astnode* init,Exp_Astnode* cond,Exp_Astnode* update,Statement_Astnode * stmts);
};

class If_Astnode: public Statement_Astnode{
    public:
    Exp_Astnode* cond;
    Statement_Astnode* then_sts;
    Statement_Astnode* else_sts;
    void print(int blanks);
    If_Astnode(Exp_Astnode* cond, Statement_Astnode* true_stmts, Statement_Astnode* flase_stmts);
};
class While_Astnode: public Statement_Astnode{
    public:
        Exp_Astnode* cond;
        Statement_Astnode* stmt;
        void print(int blanks);
        While_Astnode(Exp_Astnode* exp,Statement_Astnode* stmts);
};

class Return_Astnode: public Statement_Astnode{
    public:
    Exp_Astnode* exp;
    void print(int blanks);
    Return_Astnode(Exp_Astnode* exp);
};

class Proccall_Astnode: public Statement_Astnode{
    public:
    Identifier_Astnode* pname;
    vector<Exp_Astnode*> seq;
    Proccall_Astnode(vector<Exp_Astnode*> seq);
    Proccall_Astnode(Identifier_Astnode* pname, vector<Exp_Astnode*> seq);
    void push(Exp_Astnode* ptr);
    void print(int blanks);
};

class Op_Binary_Astnode: public Exp_Astnode {
    public:
        // int type;
        string op;
        Exp_Astnode* left, *right;
        Op_Binary_Astnode(string op, Exp_Astnode* lnode, Exp_Astnode* rnode);
        virtual void print(int blanks);
};

class Op_Unary_Astnode: public Exp_Astnode {
    public:
        // int type;
        string op;
        Exp_Astnode* child;
        Op_Unary_Astnode(string op);
        Op_Unary_Astnode(string op, Exp_Astnode* node);
        string getOp();
        virtual void print(int blanks);
};

class AssignE_Astnode: public Exp_Astnode {
    public:
        // int type; 
        Exp_Astnode *left, *right;
        AssignE_Astnode(Exp_Astnode* lnode, Exp_Astnode* rnode);
        virtual void print(int blanks);
};

class Identifier_Astnode: public Exp_Astnode {
    public:
        string id_str;
        bool isStruct;
        Identifier_Astnode(string str);
        Identifier_Astnode(bool isStruct, string str);
        virtual void print(int blanks);
};

class Funcall_Astnode: public Exp_Astnode {
    public:
        // int type;
        Identifier_Astnode* fname;
        vector<Exp_Astnode*> nodes;
        Funcall_Astnode(){}
        Funcall_Astnode(Identifier_Astnode* fname);
        void setIdentifier(Identifier_Astnode* id);
        int push(Exp_Astnode* node);
        virtual void print(int blanks);
};

class Intconst_Astnode: public Exp_Astnode {
    public:
        // int type; 
        int val;
        // Intconst_Astnode(int val);
        Intconst_Astnode(string val);
        virtual void print(int blanks);
};


class Floatconst_Astnode: public Exp_Astnode {
    public:
        // int type;
        float val;
        // Floatconst_Astnode(float val);
        Floatconst_Astnode(string val);
        virtual void print(int blanks);
};


class Stringconst_Astnode: public Exp_Astnode {
    public:
        // int type; 
        string str;
        Stringconst_Astnode(string str);
        virtual void print(int blanks);
};


class Pointer_Astnode: public Exp_Astnode {
    public:
        // int type;
        Exp_Astnode* node;
        Pointer_Astnode(Exp_Astnode* node);
        virtual void print(int blanks);
};

class Arrayref_Astnode: public Ref_Astnode {
    public:
        // int type; 
        Exp_Astnode *array;
        Exp_Astnode *index;
        Arrayref_Astnode(Exp_Astnode* lnode, Exp_Astnode* rnode);
        virtual void print(int blanks);
};

class Deref_Astnode: public Ref_Astnode {
    public:
        // int type; 
        Exp_Astnode *node;
        Deref_Astnode(Exp_Astnode* node);
        virtual void print(int blanks);
};

class Member_Astnode: public Ref_Astnode {
    public:
        // int type; 
        Exp_Astnode *lnode;
        Identifier_Astnode *rnode;
        Member_Astnode(Exp_Astnode* lnode, Identifier_Astnode* rnode);
        virtual void print(int blanks);
};

class Arrow_Astnode: public Ref_Astnode {
    public:
        // int type; 
        Exp_Astnode *lnode;
        Identifier_Astnode *rnode;
        Arrow_Astnode(Exp_Astnode* lnode, Identifier_Astnode* rnode);
        virtual void print(int blanks);
};