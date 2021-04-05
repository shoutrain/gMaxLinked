/*
 ============================================================================
 Name        : BaseType.h
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : Define types
 ============================================================================
 */

#ifndef _BASE_TYPE_H_
#define _BASE_TYPE_H_

typedef char b1_;
typedef unsigned char ub1_;
typedef short b2_;
typedef unsigned short ub2_;
typedef int b4_;
typedef unsigned int ub4_;
typedef long long b8_;
typedef unsigned long long ub8_;

typedef float fb4_;
typedef double fb8_;

typedef char c1_;

typedef enum { false_v, true_v } bool_;

#define null_v nullptr

typedef void none_;
typedef void* obj_;

#include <assert.h>

#endif  // _BASE_TYPE_H_
