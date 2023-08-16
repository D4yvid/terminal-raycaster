#ifndef TYPES_H
#define TYPES_H

typedef char i8;
typedef short i16;
typedef int i32;
typedef long i64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

typedef float f32;
typedef double f64;

#define MS_TO_SEC(ms) ms / 1000
#define MS_TO_US(ms) ms * 1000

#define __NORETURN__ __attribute__((noreturn))
#define __INTERNAL__ __attribute__((visibility("hidden"))) static

#endif /** TYPES_H */
