#pragma once
#include<bits/stdc++.h>
#include "ast.hh"
#include "regMgmt.hh"
#include "regMgmt.cpp"
#include "utils.hh"
#include "checks.hh"
using namespace std;

extern RegisterDescriptor* regDsp;
extern AddressDescriptor* addDsp;

extern SymbTab gst;
extern map<string, SymbTab*> flocal_vars;
extern map<string, SymbTab*> slocal_vars;
extern string curr_func_driver;

Abstract_Astnode::Abstract_Astnode(){
    this->lvalue = false;
    this->rvalue = false;
    this->nodeLabel = 0;
}

// EMPTY
Empty_Astnode::Empty_Astnode(){
    this->astnode_type = EMPTY_ASTNODE;
}
void Empty_Astnode::print(int blanks){
    string tabs(blanks, ' ');
    cout<<"\"empty\"";
}
SynAtr Empty_Astnode::genCode(deque<string>& code, InhAtr inh){
    struct SynAtr a;
    a.var = "";
    return a;
}
// SEQ
Seq_Astnode::Seq_Astnode(){
    this->astnode_type = SEQ_ASTNODE;
}
void Seq_Astnode::push(Statement_Astnode* ptr){
    seq.push_back(ptr);
}
SynAtr Seq_Astnode::genCode(deque<string>& code, InhAtr inh){
    struct SynAtr syn;
    bool is = false;

    for(int i=0; i<(int)this->seq.size(); i++){
        if(syn.next.size() != 0){
            string label = genLabel();
            code.push_back(label+":\n");
            backpatch(syn.next, label);
        }
        struct SynAtr tSyn = this->seq[i]->genCode(code, inh);
        syn.next = tSyn.next;
        syn.breaklist = mergelist(syn.breaklist, tSyn.breaklist);
    }
    if(syn.next.size()!=0 && seq.back()->astnode_type == RETURN_ASTNODE){
        string label = genLabel();
        code.push_back(label+":\n");
        backpatch(syn.next, label);
    }
    syn.isConst = false;
    return syn;
}

// ASSIGNS
AssignS_Astnode::AssignS_Astnode(Exp_Astnode* left, Exp_Astnode* right){
    this->astnode_type = ASSIGNS_ASTNODE;
    this->left = left;
    this->right = right;
}
SynAtr AssignS_Astnode::genCode(deque<string>& code, InhAtr inh){
    struct SynAtr sleft = this->left->genCode(code, inh);
    struct SynAtr sright = this->right->genCode(code, inh);
    if(sright.truelist.size() != 0){
        string tLabel = genLabel();
        code.push_back(tLabel+":\n");
        backpatch(sright.truelist, tLabel);
        code.push_back("\t\tmovl\t\t$1, "+getMem(sleft, addDsp)+"\n");
        string nLabel = genLabel();
        code.push_back("\t\tjmp\t\t\t"+nLabel+"\n");
        string fLabel = genLabel();
        code.push_back(fLabel+":\n");
        backpatch(sright.falselist, fLabel);
        code.push_back("\t\tmovl\t\t$0, "+getMem(sleft, addDsp)+"\n");
        code.push_back(nLabel+":\n");
    }else if(sright.truelist.size() == 0 && sright.falselist.size() != 0){
        code.push_back("\t\tmovl\t\t$1, "+getMem(sleft, addDsp)+"\n");
        string nLabel = genLabel();
        code.push_back("\t\tjmp\t\t\t"+nLabel+"\n");
        string fLabel = genLabel();
        code.push_back(fLabel+":\n");
        backpatch(sright.falselist, fLabel);
        code.push_back("\t\tmovl\t\t$0, "+getMem(sleft, addDsp)+"\n");
        code.push_back(nLabel+":\n");
    }else{
        // in arthmatic op tl and fl are null
        if(regDsp->find(sright.var) == "" && !isNum(sright.var)){
            code.push_back("\t\tmovl\t\t"+getVar(sright, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
            regDsp->setLive(regDsp->regSt.top());
            regDsp->setRegToVar(regDsp->regSt.top(), sright.var);
        }
        code.push_back("\t\tmovl\t\t"+getVar(sright, regDsp, addDsp)+", "+getMem(sleft, addDsp)+"\n");
    }
    if(sright.var[0] == '#'){
        code.push_back("\t\taddl\t\t$"+to_string(sright.dt.size)+", %esp\n");
        addDsp->popTemp(sright.var);
    }
    regDsp->freeTempRegs();
    regDsp->freeAllRegs();
    struct SynAtr a;
    a.var = "";
    return a;
}

// IF
If_Astnode::If_Astnode(Exp_Astnode* cond, Statement_Astnode* true_stmts,Statement_Astnode* flase_stmts){
    this->astnode_type = IF_ASTNODE;
    this->cond=cond;
    this->then_sts=true_stmts;
    this->else_sts=flase_stmts;
}
SynAtr If_Astnode::genCode(deque<string>& code, InhAtr inh){
    struct SynAtr syn;
    struct SynAtr syn_cond = this->cond->genCode(code, inh);
    if(syn_cond.truelist.size() != 0){
        string tLabel = genLabel();
        code.push_back(tLabel+":\n");
        backpatch(syn_cond.truelist, tLabel);
    }
    struct SynAtr syn_then = this->then_sts->genCode(code, inh);
    string fLabel = genLabel();
    code.push_back("\t\tjmp\t\t\t");
    syn.next = {&code.back()};
    code.push_back(fLabel+":\n");
    backpatch(syn_cond.falselist, fLabel);
    struct SynAtr syn_else = this->else_sts->genCode(code, inh);
    syn.next = mergelist(syn.next, syn_then.next);
    syn.next = mergelist(syn.next, syn_else.next);
    syn.isConst = false;
    return syn;
}

// WHILE
While_Astnode::While_Astnode(Exp_Astnode* exp, Statement_Astnode* stmts){
    this->astnode_type = WHILE_ASTNODE;
    this->cond=exp;
    this->stmt=stmts;
}
SynAtr While_Astnode::genCode(deque<string>& code, InhAtr inh){
    regDsp->freeAllRegs();
    string m1 = genLabel();
    code.push_back(m1+":\n");
    struct InhAtr inh1 = inh;
    inh1.fall = true;
    struct SynAtr syn_cond = this->cond->genCode(code, inh1);
    if(syn_cond.truelist.size() != 0){
        string m2 = genLabel();
        code.push_back(m2+":\n");
        backpatch(syn_cond.truelist, m2);
    }
    struct SynAtr syn_stmts =  this->stmt->genCode(code, inh);
    backpatch(syn_stmts.next, m1);
    code.push_back("\t\tjmp\t\t\t"+m1+"\n");
    struct SynAtr syn;
    syn.var = "";
    syn.next = mergelist(syn_cond.falselist, syn_stmts.breaklist);
    syn.isConst = false;
    return syn;
}

// FOR
For_Astnode::For_Astnode(
    Exp_Astnode* init,
    Exp_Astnode* cond,
    Exp_Astnode* update,
    Statement_Astnode *stmts
){
    this->astnode_type = FOR_ASTNODE;
    this->init=init;
    this->guard=cond;
    this->step=update;
    this->body=stmts;
}
SynAtr For_Astnode::genCode(deque<string>& code, InhAtr inh){
    struct SynAtr syn_init = this->init->genCode(code, inh);
    regDsp->freeAllRegs();
    string m1 = genLabel();
    code.push_back(m1+":\n");
    struct InhAtr inh1 = inh;
    inh1.fall = true;
    struct SynAtr syn_guard =  this->guard->genCode(code, inh1);
    if(syn_guard.truelist.size() != 0){
        string m2 = genLabel();
        code.push_back(m2+":\n");
        backpatch(syn_guard.truelist, m2);
    }
    struct SynAtr syn_body =  this->body->genCode(code, inh);
    if(syn_body.next.size() > 0){
        string s = genLabel();
        code.push_back(s+":\n");
        backpatch(syn_body.next, s);
    }
    struct SynAtr syn_step = this->step->genCode(code, inh);
    code.push_back("\t\tjmp\t\t\t"+m1+"\n");
    struct SynAtr syn;
    syn.var = "";
    syn.next = mergelist(syn_guard.falselist, syn_body.breaklist);
    syn.isConst = false;
    return syn;
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
SynAtr Proccall_Astnode::genCode(deque<string>& code, InhAtr inh){
    regDsp->freeAllRegs();
    int ret_sz = 0;
    string returnVar;
    bool isPrintF = this->pname->getId() == "printf";
    bool isStruct = false;
    string type_ret = "void";
    if(isPrintF){
        type_ret = "int";
        addDsp->espOff-=4;
        code.push_back("\t\tsubl\t\t$4, %esp\n");
    }else{
        type_ret = gst.find(this->pname->getId())->type_returnType;
        ret_sz = getSizeTS(type_ret);
    }
    if(type_ret != "int" && type_ret != "flaot" && type_ret != "void"){
        isStruct = true;
    }
    if(ret_sz>0){
        code.push_back("\t\tsubl\t\t$"+to_string(ret_sz)+", %esp\n");
        returnVar = newTemp(ret_sz, addDsp);
    }
    int callStackSize = 0;
    for(int i=this->seq.size()-1; i>=0; i--){
        struct SynAtr temp = this->seq[i]->genCode(code, inh);
        if(temp.truelist.size() != 0){
            string tLabel = genLabel();
            code.push_back(tLabel+":\n");
            backpatch(temp.truelist, tLabel);
            code.push_back("\t\tpushl\t\t$1\n");
            string nLabel = genLabel();
            code.push_back("\t\tjmp\t\t\t"+nLabel+"\n");
            string fLabel = genLabel();
            code.push_back(fLabel+":\n");
            backpatch(temp.falselist, fLabel);
            code.push_back("\t\tpushl\t\t$0\n");
            code.push_back(nLabel+":\n");
            temp.dt = createDataType(INT_TYPE);
        }else if(temp.truelist.size() == 0 && temp.falselist.size() != 0){
            code.push_back("\t\tpushl\t\t$1\n");
            string nLabel = genLabel();
            code.push_back("\t\tjmp\t\t\t"+nLabel+"\n");
            string fLabel = genLabel();
            code.push_back(fLabel+":\n");
            backpatch(temp.falselist, fLabel);
            code.push_back("\t\tpushl\t\t$0\n");
            code.push_back(nLabel+":\n");
            temp.dt = createDataType(INT_TYPE);
        }else{
            if(temp.var[0] == '#'){
                if(regDsp->find(temp.var) == ""){
                    code.push_back("\t\tmovl\t\t"+getVar(temp, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
                }else{
                    regDsp->reTopStack(regDsp->find(temp.var));
                }
                int sz = addDsp->temp_size[temp.var];
                addDsp->popTemp(temp.var);
                code.push_back("\t\taddl\t\t$"+to_string(sz)+", %esp\n");
                code.push_back("\t\tpushl\t\t"+regDsp->regSt.top()+"\n");
            }else{
                if(regDsp->find(temp.var) == ""){
                    code.push_back("\t\tmovl\t\t"+getVar(temp, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
                    code.push_back("\t\tpushl\t\t"+regDsp->regSt.top()+"\n");
                }else{
                    code.push_back("\t\tpushl\t\t"+getVar(temp, regDsp, addDsp)+"\n");
                }
            }
        }
        addDsp->espOff -= temp.dt.size;
        callStackSize += temp.dt.size;
    }
    addDsp->espOff += callStackSize;
    regDsp->freeAllRegs();
    code.push_back("\t\tcall\t\t"+this->pname->getId()+"\n");
    if(seq.size()){
        code.push_back("\t\taddl\t\t$"+to_string(4*seq.size())+", %esp\n");
    }
    if(isPrintF){
        code.push_back("\t\taddl\t\t$4, %esp\n");
        addDsp->espOff += 4;
    }
    struct SynAtr syn;
    syn.var = returnVar;
    syn.isFuncCall = true;
    if(isStruct){

    }else if(type_ret == "int"){
        syn.dt = createDataType(INT_TYPE);
    }
    return syn;
}

// RETURN
Return_Astnode::Return_Astnode(Exp_Astnode* exp){
    this->astnode_type = RETURN_ASTNODE;
    this->exp= exp;
}
SynAtr Return_Astnode::genCode(deque<string>& code, InhAtr inh){
    struct SynAtr syn_exp = this->exp->genCode(code, inh);
    if(curr_func_driver != "main" && gst.find(curr_func_driver)->type_returnType != "void"){
        int ret_offset = getRetOff(curr_func_driver);
        if(regDsp->find(syn_exp.var) != ""){
            code.push_back("\t\tmovl\t\t"+getVar(syn_exp, regDsp, addDsp)+", "+to_string(ret_offset)+"(%ebp)\n");
        }else{
            code.push_back("\t\tmovl\t\t"+getVar(syn_exp, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
            code.push_back("\t\tmovl\t\t"+regDsp->regSt.top()+", "+to_string(ret_offset)+"(%ebp)\n");
        }
    }
    if(syn_exp.var[0] == '#'){
        addDsp->popTemp(syn_exp.var);
        code.push_back("\t\taddl\t\t$"+to_string(syn_exp.dt.size)+", %esp\n");
    }
    if(curr_func_driver == "main"){
        code.push_back("\t\tmovl\t\t$0, %eax\n");
    }
    code.push_back("\t\tleave\n");
    code.push_back("\t\tret\n");
    struct SynAtr a;
    a.var = "#ret";
    regDsp->freeAllRegs();
    return a;
}

// Binay op
Op_Binary_Astnode::Op_Binary_Astnode(string op, Exp_Astnode* lnode, Exp_Astnode* rnode){
    this->astnode_type = OP_BINARY_ASTNODE;
    this->op = op;
    this->left = lnode;
    this->right = rnode;
}
SynAtr Op_Binary_Astnode::genCode(deque<string>& code, InhAtr inh){
    if(isRelOp(this->op)){
        struct SynAtr lSyn = this->left->genCode(code, inh);
        string tReg = "";
        if(lSyn.var[0] == '!' ){
            tReg = regDsp->popRegSt();
        }
        struct SynAtr rSyn = this->right->genCode(code, inh);
        if(tReg != "") {
            regDsp->pushRegSt(tReg);
        }
        if(regDsp->find(lSyn.var) == ""){
            code.push_back("\t\tmovl\t\t"+getVar(lSyn, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
        }
        if(lSyn.var[0] == '#'){
            addDsp->popTemp(lSyn.var);
            code.push_back("\t\taddl\t\t$"+to_string(lSyn.dt.size)+", %esp\n");
        }
        code.push_back("\t\tcmpl\t\t"+getVar(rSyn, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
        string ins = op_to_ins(this->op, inh.fall);
        struct SynAtr syn;
        // temp mem free
        if(rSyn.var[0] == '#'){
            addDsp->popTemp(rSyn.var);
            code.push_back("\t\taddl\t\t$"+to_string(lSyn.dt.size)+", %esp\n");
        }
        code.push_back("\t\t"+ins+"\t\t\t");
        if(inh.fall){
            syn.falselist = {&code.back()};
        }else{
            syn.truelist = {&code.back()};
        }

        string name = newTempVar();
        regDsp->setLive(regDsp->regSt.top());
        regDsp->setRegToVar(regDsp->regSt.top(), name);
        syn.var = name;
        return syn;
    }
    else if(this->op == "OR_OP" || this->op == "AND_OP"){
        struct InhAtr inh1 = inh;
        bool isOr = this->op == "OR_OP";
        
        if(isOr){
            inh1.fall = false;
        }else{
            inh1.fall = true;
        }
        
        struct SynAtr lSyn = this->left->genCode(code, inh1);
        
        if(lSyn.var[0] != '@'){
            string ins = op_to_ins("GT_OP_INT", inh1.fall);
            
            if(regDsp->find(lSyn.var) == ""){
                code.push_back("\t\tmovl\t\t"+getVar(lSyn, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
            }else{
                regDsp->reTopStack(regDsp->find(lSyn.var));
            }
            
            code.push_back("\t\tcmpl\t\t$0, "+regDsp->regSt.top()+"\n");
            code.push_back("\t\t"+ins+"\t\t\t");
            
            if(inh1.fall){
                lSyn.falselist = {&code.back()};
            }else{
                lSyn.truelist = {&code.back()};
            }
            
            string name = newTempVar();
            regDsp->setLive(regDsp->regSt.top());
            regDsp->setRegToVar(regDsp->regSt.top(), name);

            // temp mem free
            if(lSyn.var[0] == '#'){
                addDsp->popTemp(lSyn.var);
                code.push_back("\t\taddl\t\t$"+to_string(lSyn.dt.size)+", %esp\n");
            }

            lSyn.var = name;
        }

        if(isOr){
            if(lSyn.falselist.size()){
                string label = genLabel();
                code.push_back(label+":\n");
                backpatch(lSyn.falselist, label);
            }
        }else{
            if(lSyn.truelist.size()){
                string label = genLabel();
                code.push_back(label+":\n");
                backpatch(lSyn.truelist, label);
            }
        }

        struct SynAtr rSyn = this->right->genCode(code, inh);
        
        if(rSyn.var[0] != '@'){
            string ins = op_to_ins("LT_OP_INT", inh.fall);

            if(regDsp->find(rSyn.var) == ""){
                code.push_back("\t\tmovl\t\t"+getVar(rSyn, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
            }else{
                regDsp->reTopStack(regDsp->find(rSyn.var));
            }
            
            code.push_back("\t\tcmpl\t\t$0, "+regDsp->regSt.top()+"\n");
            code.push_back("\t\t"+ins+"\t\t\t");
            
            if(inh.fall){
                rSyn.falselist = {&code.back()};
            }else{
                rSyn.truelist = {&code.back()};
            }
            
            string name = newTempVar();
            regDsp->setLive(regDsp->regSt.top());
            regDsp->setRegToVar(regDsp->regSt.top(), name);

            // mem manage
            if(rSyn.var[0] == '#'){
                addDsp->popTemp(rSyn.var);
                code.push_back("\t\taddl\t\t$"+to_string(lSyn.dt.size)+", %esp\n");
            }

            rSyn.var = name;
        }
        struct SynAtr syn;
        if(isOr){
            syn.truelist = mergelist(lSyn.truelist, rSyn.truelist);
            syn.falselist = rSyn.falselist;
        }else{
            syn.truelist = rSyn.truelist;
            syn.falselist = mergelist(lSyn.falselist, rSyn.falselist);
        }
        string name = newTempVar();
        regDsp->setLive(regDsp->regSt.top());
        regDsp->setRegToVar(regDsp->regSt.top(), name);
        syn.var = name;
        return syn;
    }

    ////////////////////////////
    struct SynAtr lSyn, rSyn;
    string tReg, temp;
    int sethi_case = 0;
    // sethi ulmam
    // 2.
    if(this->right->nodeLabel == 0){
        sethi_case = 2;
        if(this->left->nodeLabel == 0 || this->left->nodeLabel == 1){
            lSyn = this->left->genCode(code, inh);
            if(regDsp->find(lSyn.var) == ""){
                code.push_back("\t\tmovl\t\t"+getVar(lSyn, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
                regDsp->setLive(regDsp->regSt.top());
                regDsp->setRegToVar(regDsp->regSt.top(), lSyn.var);
            }
        }else{
            lSyn = this->left->genCode(code, inh);
        }
        rSyn = this->right->genCode(code, inh);
    }
    // 3. 
    else if(this->left->nodeLabel < this->right->nodeLabel && this->left->nodeLabel < regDsp->numRegs){
        // cout<<"3 "<<regDsp->regSt.size()<<endl;
        sethi_case = 3;
        // code.push_back("3 "+to_string(this->left->nodeLabel)+":\n");
        regDsp->swapRegSt();
        rSyn = this->right->genCode(code, inh);
        if(regDsp->find(rSyn.var) == ""){
            code.push_back("\t\tmovl\t\t"+getVar(rSyn, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
            regDsp->setLive(regDsp->regSt.top());
            regDsp->setRegToVar(regDsp->regSt.top(), rSyn.var);
        }
        tReg = regDsp->popRegSt();
        regDsp->freeReg(tReg);
        if(this->left->nodeLabel == 0 || this->left->nodeLabel == 1){
            lSyn = this->left->genCode(code, inh);
            if(regDsp->find(lSyn.var) == ""){
                code.push_back("\t\tmovl\t\t"+getVar(lSyn, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
                regDsp->setLive(regDsp->regSt.top());
                regDsp->setRegToVar(regDsp->regSt.top(), lSyn.var);
            }
        }else{
            lSyn = this->left->genCode(code, inh);
        }
    }
    // 4.
    else if(this->right->nodeLabel <= this->left->nodeLabel && this->right->nodeLabel < regDsp->numRegs){
        sethi_case = 4;
        // code.push_back("4 "+to_string(this->left->nodeLabel)+":\n");
        if(this->left->nodeLabel == 0 || this->left->nodeLabel == 1){
            lSyn = this->left->genCode(code, inh);
            if(regDsp->find(lSyn.var) == ""){
                code.push_back("\t\tmovl\t\t"+getVar(lSyn, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
                regDsp->setLive(regDsp->regSt.top());
                regDsp->setRegToVar(regDsp->regSt.top(), lSyn.var);
            }
        }else{
            lSyn = this->left->genCode(code, inh);
        }
        tReg = regDsp->popRegSt();
        regDsp->freeReg(tReg);
        rSyn = this->right->genCode(code, inh);
    }
    // 5
    else{
        // cout<<"5 "<<regDsp->regSt.size()<<endl;
        sethi_case = 5;
        rSyn = this->right->genCode(code, inh);
        temp = newTemp(4, addDsp);
        code.push_back("\t\tsubl\t\t$4, %esp\n");
        code.push_back("\t\tmovl\t\t"+regDsp->regSt.top()+", "+getMem(temp, addDsp)+"\n"); //store
        rSyn.var = temp;
        regDsp->freeReg(regDsp->regSt.top());
        if(this->left->nodeLabel == 0 || this->left->nodeLabel == 1){
            lSyn = this->left->genCode(code, inh);
            if(regDsp->find(lSyn.var) == ""){
                code.push_back("\t\tmovl\t\t"+getVar(lSyn, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
                regDsp->setLive(regDsp->regSt.top());
                regDsp->setRegToVar(regDsp->regSt.top(), lSyn.var);
            }
        }else{
            lSyn = this->left->genCode(code, inh);
        }
    }
    string ins = op_to_ins(this->op, inh.fall);

    // for recursive function calls
    if(lSyn.isFuncCall || (rSyn.isFuncCall && regDsp->find(lSyn.var) == "")){
        code.push_back("\t\tmovl\t\t"+getVar(lSyn, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
        regDsp->setLive(regDsp->regSt.top());
        regDsp->setRegToVar(regDsp->regSt.top(), lSyn.var);
    }
    
    // div operation
    if(ins == "div"){
        bool isLiveEAX = regDsp->isLive("%eax");
        bool isLiveEBX = regDsp->isLive("%ebx");
        bool isLiveEDX = regDsp->isLive("%edx");
        bool use_ebp = false;
        string eax_temp = "", ebx_temp = "", edx_temp = "";
        if(getVar(rSyn, regDsp, addDsp)[0] == '$')
        {
            if(isLiveEBX){
                ebx_temp = newTemp(4, addDsp);
                code.push_back("\t\tsubl\t\t$4, %esp\n");
                code.push_back("\t\tmovl\t\t"+getMem(ebx_temp, addDsp)+", %ebp\n");     
            }
            // regDsp->freeReg("%ebx");
            code.push_back("\t\tmovl\t\t"+getVar(rSyn, regDsp, addDsp)+", %ebx\n");
            use_ebp = true;
        }
        if(isLiveEDX && regDsp->regSt.top() != "%edx")
        {
            edx_temp = newTemp(4, addDsp);
            code.push_back("\t\tsubl\t\t$4, %esp\n");
            code.push_back("\t\tmovl\t\t%edx, "+getMem(edx_temp, addDsp)+"\n"); //store
            // regDsp->freeReg("%edx");
        }
        if(getVar(lSyn, regDsp, addDsp) == "%eax"){
            code.push_back("\t\tcltd\n");
            if(use_ebp){
                code.push_back("\t\tidivl\t\t%ebx\n");
            }else{
            // code.push_back("here1:\n");
                code.push_back("\t\tidivl\t\t"+getVar(rSyn,regDsp, addDsp)+"\n");
            }
            code.push_back("\t\txorl\t\t%edx, %edx\n");
            regDsp->reTopStack("%eax");
        }else{
            bool inStack = regDsp->reTopStack("%eax");
            if(!inStack){
                eax_temp = newTemp(4, addDsp);
                code.push_back("\t\tsubl\t\t$4, %esp\n");
                code.push_back("\t\tmovl\t\t%eax, "+getMem(eax_temp, addDsp)+"\n"); //store
                // regDsp->freeReg("%eax", regDsp);
            }else{
                code.push_back("\t\tmovl\t\t"+getVar(lSyn, regDsp, addDsp)+", %eax\n");
            }
            // code.push_back("here2:");
            if(use_ebp){
                code.push_back("\t\tidivl\t\t%ebx\n");
            }else{
                code.push_back("\t\tidivl\t\t"+getVar(rSyn,regDsp, addDsp)+"\n");
            }
            code.push_back("\t\txorl\t\t%edx, %edx\n");
            if(!inStack)
            {
                code.push_back("\t\tmovl\t\t%eax, "+regDsp->regSt.top()+"\n");
                code.push_back("\t\tmovl\t\t"+getMem(eax_temp, addDsp)+", %eax\n");
                code.push_back("\t\taddl\t\t$4, %esp\n");
                addDsp->popTemp(eax_temp);
            }
        }
        if(isLiveEDX && regDsp->regSt.top() != "%edx"){
            code.push_back("\t\txorl\t\t%edx, %edx\n");
            code.push_back("\t\tmovl\t\t"+getMem(edx_temp, addDsp)+", %edx\n");
            code.push_back("\t\taddl\t\t$4, %esp\n");
            addDsp->popTemp(edx_temp);
        }
        if(getVar(rSyn, regDsp, addDsp)[0] == '$' && isLiveEBX){
            code.push_back("\t\tmovl\t\t"+getMem(ebx_temp, addDsp)+", %ebx\n");
            code.push_back("\t\taddl\t\t$4, %esp\n");
            addDsp->popTemp(ebx_temp);
        }  
    }else{
        if(sethi_case == 3){
            code.push_back("\t\t"+ins+"\t\t"+tReg+", "+regDsp->regSt.top()+"\n");
        }else if(sethi_case == 4){
            code.push_back("\t\t"+ins+"\t\t"+getVar(rSyn, regDsp, addDsp)+", "+tReg+"\n");
        }else {
            code.push_back("\t\t"+ins+"\t\t"+getVar(rSyn, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
        }
    }
    // 2
    if(this->right->nodeLabel == 0){
        // nothing special
    } // 3
    else if(this->left->nodeLabel < this->right->nodeLabel && this->left->nodeLabel < regDsp->numRegs){
        regDsp->pushRegSt(tReg);
        regDsp->swapRegSt();
    } // 4
    else if(this->right->nodeLabel <= this->left->nodeLabel && this->right->nodeLabel < regDsp->numRegs){
        regDsp->pushRegSt(tReg);
    } // 5
    else{
        code.push_back("\t\taddl\t\t$4, %esp\n");
        addDsp->popTemp(temp);
    }

    // temp mem free
    if(lSyn.var[0] == '#'){
        addDsp->popTemp(lSyn.var);
        code.push_back("\t\taddl\t\t$"+to_string(lSyn.dt.size)+", %esp\n");
    }
    if(rSyn.var[0] == '#'){
        addDsp->popTemp(rSyn.var);
        code.push_back("\t\taddl\t\t$"+to_string(lSyn.dt.size)+", %esp\n");
    }
    string name = getTemp();
    regDsp->setLive(regDsp->regSt.top());
    regDsp->setRegToVar(regDsp->regSt.top(), name);
    struct SynAtr syn;
    syn.var = name;
    return syn;

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
SynAtr Op_Unary_Astnode::genCode(deque<string>& code, InhAtr inh){
    struct SynAtr syn = this->child->genCode(code, inh);
    if(this->op == "NOT"){
        if(syn.var[0] != '@'){
            if(regDsp->find(syn.var) == ""){
                code.push_back("\t\tmovl\t\t"+getVar(syn, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
            }else{
                regDsp->reTopStack(regDsp->find(syn.var));
            }
            code.push_back("\t\tcmpl\t\t$0, "+regDsp->regSt.top()+"\n");
            string ins = op_to_ins("GT_OP_INT", inh.fall);
            code.push_back("\t\t"+ins+"\t\t\t");
            if(inh.fall){
                syn.falselist = {&code.back()};
            }else{
                syn.truelist = {&code.back()};
            }
            string name = newTempVar();
            regDsp->setLive(regDsp->regSt.top());
            regDsp->setRegToVar(regDsp->regSt.top(), name);
            syn.var = name;
            return syn;
        }else{
            vector<string*> temp = syn.truelist;
            syn.truelist = syn.falselist;
            syn.falselist = temp;
        }
        
    }else if(this->op == "DEREF"){
        if(this->child->nodeLabel == 0){
        code.push_back("\t\tmovl\t\t"+getVar(syn, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
        if(!syn.isConst){
            regDsp->setLive(regDsp->regSt.top());
            regDsp->setRegToVar(regDsp->regSt.top(), syn.var);
        }
        this->child->nodeLabel = 1;
        code.push_back("\t\tmovl\t\t("+regDsp->regSt.top()+"), "+regDsp->regSt.top()+"\n");
        syn.dt = derefDataType(syn.dt);
        return syn;
    }
    }else if(this->op == "PP"){
        code.push_back("\t\tmovl\t\t"+getMem(syn.var, addDsp)+", "+regDsp->regSt.top()+"\n");
        if(isInt(syn.dt)){
            code.push_back("\t\taddl\t\t$1, "+getMem(syn, addDsp)+"\n");
        }
        // else{
        //     cout<<"PP should not get this error for now\n";
        //     exit(1);
        // }
        string name = getTemp();
        regDsp->setLive(regDsp->regSt.top());
        regDsp->setRegToVar(regDsp->regSt.top(), name);
        syn.var = name;
        return syn;
    }else if(this->op == "UMINUS"){
        if(regDsp->find(syn.var) == ""){
            code.push_back("\t\tmovl\t\t"+getVar(syn, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
            code.push_back("\t\tneg\t\t"+regDsp->regSt.top()+"\n");
            string name = getTemp();
            regDsp->setLive(regDsp->regSt.top());
            regDsp->setRegToVar(regDsp->regSt.top(), name);
            syn.var = name;
            return syn;
        }else{
            code.push_back("\t\tneg\t\t"+regDsp->find(syn.var)+"\n");
            return syn;
        }
    }else if(this->op == "ADDRESS"){
        if(regDsp->find(syn.var) != ""){

        }else{

        }
        code.push_back("\t\tleal\t\t"+getVar(syn, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
        string name = getTemp();
        regDsp->setLive(regDsp->regSt.top());
        regDsp->setRegToVar(regDsp->regSt.top(), name);
        struct SynAtr syn_ret;
        syn_ret.var = name;
        syn_ret.dt = refDataType(syn.dt);
        return syn;
    }
    return syn;
}

// assign
AssignE_Astnode::AssignE_Astnode(Exp_Astnode* lnode, Exp_Astnode* rnode){
    this->astnode_type = ASSIGNE_ASTNODE; 
    this->left = lnode;
    this->right = rnode;
}
SynAtr AssignE_Astnode::genCode(deque<string>& code, InhAtr inh){
    struct SynAtr sleft = this->left->genCode(code, inh);
    struct SynAtr sright = this->right->genCode(code, inh);
    if(sright.truelist.size() != 0){
        string tLabel = genLabel();
        code.push_back(tLabel+":\n");
        backpatch(sright.truelist, tLabel);
        code.push_back("\t\tmovl\t\t$1, "+getMem(sleft, addDsp)+"\n");
        string nLabel = genLabel();
        code.push_back("\t\tjmp\t\t\t"+nLabel+"\n");
        string fLabel = genLabel();
        code.push_back(fLabel+":\n");
        backpatch(sright.falselist, fLabel);
        code.push_back("\t\tmovl\t\t$0, "+getMem(sleft, addDsp)+"\n");
        code.push_back(nLabel+":\n");
    }else if(sright.truelist.size() == 0 && sright.falselist.size() != 0){
        code.push_back("\t\tmovl\t\t$1, "+getMem(sleft, addDsp)+"\n");
        string nLabel = genLabel();
        code.push_back("\t\tjmp\t\t\t"+nLabel+"\n");
        string fLabel = genLabel();
        code.push_back(fLabel+":\n");
        backpatch(sright.falselist, fLabel);
        code.push_back("\t\tmovl\t\t$0, "+getMem(sleft, addDsp)+"\n");
        code.push_back(nLabel+":\n");
    }else{
        // in arthmatic op tl and fl are null
        if(regDsp->find(sright.var) == "" && !isNum(sright.var)){
            code.push_back("\t\tmovl\t\t"+getVar(sright, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
            regDsp->setLive(regDsp->regSt.top());
            regDsp->setRegToVar(regDsp->regSt.top(), sright.var);
        }
        code.push_back("\t\tmovl\t\t"+getVar(sright, regDsp, addDsp)+", "+getMem(sleft, addDsp)+"\n");
    }
    if(sright.var[0] == '#'){
        code.push_back("\t\taddl\t\t$"+to_string(sright.dt.size)+", %esp\n");
        addDsp->popTemp(sright.var);
    }
    regDsp->freeTempRegs();
    regDsp->freeAllRegs();
    struct SynAtr a;
    a.var = "";
    return a;
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
SynAtr Identifier_Astnode::genCode(deque<string>& code, InhAtr inh){
    struct SynAtr a;
    a.dt = getIdDt(curr_func_driver, this->id_str, true);
    a.var = this->id_str;
    a.isConst = false;
    a.offset = 0;
    return a;
}
string Identifier_Astnode::getId(){
    return this->id_str;
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
SynAtr Funcall_Astnode::genCode(deque<string>& code, InhAtr inh){
    regDsp->freeAllRegs();
    int ret_sz = 0;
    string returnVar;
    bool isPrintF = this->fname->getId() == "printf";
    bool isStruct = false;
    string type_ret = "void";
    int callStackSize = 0;
    if(isPrintF){
        type_ret = "int";
        addDsp->espOff -= 4;
        code.push_back("\t\tsubl\t\t$4, %esp\n");
    }else{
        type_ret = gst.find(this->fname->getId())->type_returnType;
        ret_sz = getSizeTS(type_ret);
        if(ret_sz>0){
            code.push_back("\t\tsubl\t\t$"+to_string(ret_sz)+", %esp\n");
            returnVar = newTemp(ret_sz, addDsp);
        }
    }
    if(type_ret != "int" && type_ret != "flaot" && type_ret != "void"){
        isStruct = true;
    }
    for(int i=0; i<(int)this->nodes.size(); i++){
    // for(int i=this->nodes.size()-1; i>=0; i--){
        struct SynAtr temp = this->nodes[i]->genCode(code, inh);
        if(temp.truelist.size() != 0){
            string tLabel = genLabel();
            code.push_back(tLabel+":\n");
            backpatch(temp.truelist, tLabel);
            code.push_back("\t\tpushl\t\t$1\n");
            string nLabel = genLabel();
            code.push_back("\t\tjmp\t\t\t"+nLabel+"\n");
            string fLabel = genLabel();
            code.push_back(fLabel+":\n");
            backpatch(temp.falselist, fLabel);
            code.push_back("\t\tpushl\t\t$0\n");
            code.push_back(nLabel+":\n");
            temp.dt = createDataType(INT_TYPE);
        }else if(temp.truelist.size() == 0 && temp.falselist.size() != 0){
            code.push_back("\t\tpushl\t\t$1\n");
            string nLabel = genLabel();
            code.push_back("\t\tjmp\t\t\t"+nLabel+"\n");
            string fLabel = genLabel();
            code.push_back(fLabel+":\n");
            backpatch(temp.falselist, fLabel);
            code.push_back("\t\tpushl\t\t$0\n");
            code.push_back(nLabel+":\n");
            temp.dt = createDataType(INT_TYPE);
        }else{
            if(temp.var[0] == '#'){
                if(regDsp->find(temp.var) == ""){
                    code.push_back("\t\tmovl\t\t"+getVar(temp, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
                }else{
                    regDsp->reTopStack(regDsp->find(temp.var));
                }
                int sz = addDsp->temp_size[temp.var];
                addDsp->popTemp(temp.var);
                code.push_back("\t\taddl\t\t"+to_string(sz)+", %esp\n");
                code.push_back("\t\tpushl\t\t"+regDsp->regSt.top()+"\n");
            }else{
                if(regDsp->find(temp.var) == ""){
                    code.push_back("\t\tmovl\t\t"+getVar(temp, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
                    code.push_back("\t\tpushl\t\t"+regDsp->regSt.top()+"\n");
                }else{
                    code.push_back("\t\tpushl\t\t"+getVar(temp, regDsp, addDsp)+"\n");
                }
            }
        }
        addDsp->espOff -= temp.dt.size;
        callStackSize += temp.dt.size;
    }
    addDsp->espOff += callStackSize;
    code.push_back("\t\tcall\t\t"+this->fname->getId()+"\n");
    if(nodes.size()){
        code.push_back("\t\taddl\t\t$"+to_string(4*nodes.size())+", %esp\n");
    }
    if(isPrintF){
        addDsp->espOff += 4;
        code.push_back("\t\taddl\t\t$4, %esp\n");
    }
    regDsp->freeAllRegs();
    struct SynAtr syn;
    syn.var = returnVar;
    if(isStruct){

    }else if(type_ret == "int"){
        syn.dt = createDataType(INT_TYPE);
    }

    syn.dt.size = 4;
    syn.offset = 0;
    syn.isFuncCall = true;
    return syn;
}

// intconstant
Intconst_Astnode::Intconst_Astnode(string val){
  this->astnode_type = INTCONSTANT_ASTNODE;
  this->val = stoi(val);
}
SynAtr Intconst_Astnode::genCode(deque<string>& code, InhAtr inh){
    struct SynAtr a;
    a.isConst = true;
    a.var = to_string(this->val);
    a.constVal = this->val;
    a.offset = 0;
    return a;
}

// floatconstant // not used anymore
Floatconst_Astnode::Floatconst_Astnode(string val){
  this->astnode_type = FLOATCONSTANT_ASTNODE; 
  this->val = stof(val);
}
SynAtr Floatconst_Astnode::genCode(deque<string>& code, InhAtr inh){
    struct SynAtr a;
    a.var = "";
    return a;
}

// string constant
Stringconst_Astnode::Stringconst_Astnode(string str){
    this->astnode_type = STRINGCONSTANT_ASTNODE; 
    this->str = str;
}
string Stringconst_Astnode::getId(){
    return this->str;
}
SynAtr Stringconst_Astnode::genCode(deque<string>& code, InhAtr inh){
    string label = genLabel();
    string temp = label+":\n\t\t.string\t\t" + this->str +"\n";
    // label + "\n\t\t.string\t\t" + this->str +"\n\n"
    code.push_front(temp);
    struct SynAtr syn;
    syn.var = label;
    syn.offset = 0;
    return syn;
}

// pointer not used in parser
Pointer_Astnode::Pointer_Astnode(Exp_Astnode* node){
    this->astnode_type = POINTER_ASTNODE;
    this->node = node;
}
SynAtr Pointer_Astnode::genCode(deque<string>& code, InhAtr inh){
    struct SynAtr a;
    a.var = "";
    return a;
}

// array ref
Arrayref_Astnode::Arrayref_Astnode(Exp_Astnode* lnode, Exp_Astnode* rnode){
    this->astnode_type = ARRAYREF_ASTNODE; 
    this->array = lnode;
    this->index = rnode;
}
SynAtr Arrayref_Astnode::genCode(deque<string>& code, InhAtr inh){
    struct SynAtr lArr = this->array->genCode(code, inh);
    struct SynAtr rIdx = this->index->genCode(code, inh);
    struct SynAtr a;
    a.var = "";
    return a;
}

// DEREF
Deref_Astnode::Deref_Astnode(Exp_Astnode* node){
  this->astnode_type = DEREF_ASTNODE;
  this->node = node;
}
SynAtr Deref_Astnode::genCode(deque<string>& code, InhAtr inh){
    struct SynAtr syn = this->node->genCode(code, inh);
    syn.dt = derefDataType(syn.dt);
    // return syn
    struct SynAtr a;
    a.var = "";
    return a;
}

// MEMBER
Member_Astnode::Member_Astnode(Exp_Astnode* lnode, Identifier_Astnode* rnode){
    this->astnode_type = MEMBER_ASTNODE; 
    this->lnode = lnode;
    this->rnode = rnode;
}
SynAtr Member_Astnode::genCode(deque<string>& code, InhAtr inh){
    struct SynAtr syn = this->lnode->genCode(code, inh);
    struct SynAtr syn_id = this->rnode->genCode(code, inh);
    int off_base = getStructIdOff(syn.dt.base_type_name, syn_id.var);
    syn.offset+=off_base;
    syn.dt = getIdDt(syn.dt.base_type_name, syn_id.var, false);
    return syn;
    
}

// ARROW
Arrow_Astnode::Arrow_Astnode(Exp_Astnode* lnode, Identifier_Astnode* rnode){
    this->astnode_type = ARROW_ASTNODE; 
    this->lnode = lnode;
    this->rnode = rnode;
}
SynAtr Arrow_Astnode::genCode(deque<string>& code, InhAtr inh){
    struct SynAtr lSyn = this->lnode->genCode(code, inh);
    // if(regDsp->find(lSyn.var) == ""){
    //     code.push_back("\t\tmovl\t\t"+getVar(lSyn, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
    //     regDsp->setLive(regDsp->regSt.top());
    //     regDsp->setRegToVar(regDsp->regSt.top(), lSyn.var);
    // }
    // struct SynAtr syn_id = this->rnode->genCode(code, inh);
    // int off_base = getStructIdOff(lSyn.dt.base_type_name, syn_id.var);
    // code.push_back("\t\tleal\t\t"+to_string(off_base)+"("+regDsp->find(lSyn.var)+"), "+regDsp->find(lSyn.var)+"\n");
    struct SynAtr syn;
    // syn.dt = getIdDt(lSyn.dt.base_type_name, syn_id.var, false);
    // syn.offset = 0;
    // syn.var = getLealTemp();
    // regDsp->setLive(regDsp->regSt.top());
    // regDsp->setRegToVar(regDsp->regSt.top(), syn.var);
    return syn;
    // struct SynAtr syn_lnode = this->lnode->genCode(code, inh);
    // struct SynAtr syn_id = this->rnode->genCode(code, inh);
    // if(this->lnode->nodeLabel == 0){
    //     code.push_back("\t\tmovl\t\t"+getVar(syn_lnode, regDsp, addDsp)+", "+regDsp->regSt.top()+"\n");
    // }
    // int off_base = getStructIdOff(syn_lnode.dt.base_type_name, syn_id.var);
    // int off_tot = getOffTot(syn_lnode, off_base, addDsp);
    // string temp = getTemp();
    // addDsp->var_offset[temp] = off_tot;
    // struct SynAtr syn;
    // syn.var = temp;
    // syn.dt = getIdDt(syn_lnode.dt.base_type_name, syn_id.var, false);
    // return syn;
    // struct SynAtr a;
    // a.var = "";
    // return a;
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

void Return_Astnode:: print(int blanks){
    string tabs(blanks, ' ');
    cout<<tabs<<"\"return\" : {\n";
    exp->print(blanks+2);
    cout<<tabs<<"}";
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

void Intconst_Astnode::print(int blanks){
  string tabs(blanks, ' ');
  cout<<tabs<<"\"intconst\": "<<this->val;
}

void Floatconst_Astnode::print(int blanks){
  string tabs(blanks, ' ');
//   cout<<"--------------------\nThe values is : "<<this->val<<"\n------------------\n";
  cout<<tabs<<"\"floatconst\": "<<this->val;
}

void Stringconst_Astnode::print(int blanks){
    string tabs(blanks, ' ');
    cout<<tabs<<"\"stringconst\": "<<this->str;
}

void Pointer_Astnode::print(int blanks){
    string tabs(blanks, ' ');
    cout<<tabs<<"\"pointer\": {\n";
    this->node->print(blanks+2);
    cout<<"\n";
    cout<<"}";
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

void Deref_Astnode::print(int blanks){
    string tabs(blanks, ' ');
    cout<<tabs<<"\"deref\": {\n";
    this->node->print(blanks+2);
    cout<<"\n";
    cout<<"}";
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