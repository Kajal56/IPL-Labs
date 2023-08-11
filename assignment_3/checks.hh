#pragma once
#include<bits/stdc++.h>
#include "ast.hh"
using namespace std;

bool isPointer(DataType type);
bool isArray(DataType type);
bool isInt(DataType type);
bool isFloat(DataType type);
bool isVoidPtr(DataType d);
bool isNullPtr(DataType d);
bool isCompatible(DataType d1, DataType d2);
bool isCompSub(DataType d1, DataType d2);
bool isCompParam(DataType param, DataType exp);
