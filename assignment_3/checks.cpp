#pragma once
#include "checks.hh"

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
    }
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
    // first index is interchangable to star or any other integer
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
