/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
    return ~(~x | ~y);
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
    return (x >> (n << 3)) & (0xff);
}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
    // int mask = ((0x01 << (32 + ~n)) + ~0) | (0x01 <<(32 + ~n));
    // return mask & (x >> n);
    int mask = (0x1 << (32 + ~n) << 1) + ~0;
    return mask & (x >> n);
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
    //five masks: 0x55555555 0x33333333 0x0f0f0f0f 0x00ff00ff 0x0000ffff
    int _mask_1 = (0x55 << 8) + 0x55;
    int _mask_2 = (0x33 << 8) + 0x33;
    int _mask_3 = (0x0f << 8) + 0x0f;

    int mask_1 = (_mask_1 << 16) + _mask_1;
    int mask_2 = (_mask_2 << 16) + _mask_2;
    int mask_3 = (_mask_3 << 16) + _mask_3;
    int mask_4 = (0xff << 16) + 0xff;
    int mask_5 = (0xff << 8) + 0xff;

    int ans = (x & mask_1) + ((x >> 1) & mask_1);
    ans = (ans & mask_2) + ((ans >> 2) & mask_2);
    ans = (ans & mask_3) + ((ans >> 4) & mask_3);
    ans = (ans & mask_4) + ((ans >> 8) & mask_4);
    ans = (ans & mask_5) + ((ans >> 16) & mask_5);
    return ans;
} 
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
    // another solution is binary reducing the bit via OR operation
    int tmin = 1 << 31;
    int sign_bit_mask = x & tmin;
    int plus_tmax = x + tmin + ~1 + 1; // overflow if x > 0
    int non_zero_flag = sign_bit_mask | (plus_tmax^sign_bit_mask);
    return ((~non_zero_flag) >> 31) & 1;  
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
    return 0x01 << 31;
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
    // signed n-bit range: -2^(n-1) -> 2^(n-1) -1
    int offset = 32 + ~n + 1;
    int delta = ((x << offset) >> offset) + ~x + 1;
    return !(delta^0);
}
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) {
    // add 2^n-1 if x < 0
    // -1 =  ~0
    int offset = (x >> 31) & ((1 << n) + ~0);
    return (x + offset) >> n; 
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x + 1;
}
/* 
 * isPositive - return 1 if x > 0, return 0 otherwise 
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 3
 */
int isPositive(int x) {
    return !(x & (1 << 31)) & !!x;
} 
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
    //overflow pattern:
    //positive overflow: a_msb=0  b_msb=0  -> c_msb=1
    //negative overflow: a_msb=1  b_msb=1  -> c_msb=0     
    //when x + (-y) <= 0:
    //no overflow: sub <= 0
    //pos overflow: sub > 0
    int not_y = ~y;
    int sub = x + not_y + 1;

    //sub < 0 and !((x >> 0, y <0) or (x > 0, y << 0))
    int less_no_overflow_flag = sub & (x | not_y);
    //sub > 0 and ((x << 0, y >0) or (x < 0, y >>0))
    int less_neg_overflow_flag = ~sub & (x & not_y);
    
    int less_flag = ((less_no_overflow_flag | less_neg_overflow_flag) >> 31) & 1;
    int equal_flag = !(sub^0);
    return less_flag | equal_flag;
    
    }
/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int ilog2(int x) {
    //the floor can be represented as 0xaaaaa where a is 0 or 1
    //thus x should be 2^0xaaaaa ie 1 << 0xaaaaa
    int ans = (!!(x >> 16)) << 4;
    ans = ans + ((!!(x >> (8 + ans))) << 3);
    ans = ans + ((!!(x >> (4 + ans))) << 2);
    ans = ans + ((!!(x >> (2 + ans))) << 1);
    ans = ans + ((!!(x >> (1 + ans))) << 0);
    return ans;
} 
/* 
 * float_neg - Return bit-level equivalent of expression -f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_neg(unsigned uf) {
    unsigned exp = (uf << 1) >> 24;
    unsigned frac = uf & 0x7fffff;
    if((frac == 0 || exp != 0xff)){
        uf += 0x80000000;
    }
    return uf;
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
    //valid bits fewer than 23
    //valid bits more than 23
    //zero -> zero
    int len = -1;
    unsigned temp_x = 0;
    unsigned sign = 0;
    unsigned frac = 0;
    unsigned exp = 0;

    if(x == 0){
        return 0;
    }
    if(x == 0x80000000){
        return 0xcf000000; 
    }
    if(x < 0){
        sign = 1;
        x = -x;
    }
    temp_x = x;
    while(temp_x){
        ++len;
        temp_x >>= 1;
    }
    x -=  (0x1 << len);
    if(len < 24){
        frac = x << (23 -len);
    }
    else{

        int tmp2=x>>(len-23);
        int cp2=0x1<<(len-24);
        int cp=x&((cp2<<1)-1);

        if(cp<cp2){
            frac=tmp2;
        }
        else{
            if(tmp2==0x7fffff){
                x=0;
                len++;
            }
            else{
                if(cp==cp2){
                    frac=((tmp2)&0x1)+tmp2;
                }
                else{
                    frac=tmp2+1;
                }
            }
        }

    }
    exp = len + 0x7f;
    return (sign << 31) | (exp << 23) | frac;



    
}
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
    //non-normalized -> normalized
    //normalized -> infinity
    //normalized -> normalized
    //NaN -> NaN
    //zero -> zero
    unsigned sign = uf >> 31;
    unsigned exp = (uf << 1) >> 24;
    unsigned frac = uf & 0x007fffff;
    //zero or NaN or infinity
    if((exp == 0xff) || (exp == 0 && frac == 0)){
        return uf;
    }
    // non-normalized
    else if(exp == 0){
        if(frac >> 22){ //highest fraction bit not zero
            exp += 1;
        }
        frac <<= 1;
    }
    //normalized
    else if(exp != 0){
        exp += 1;
        if(exp == 0xff){//becomes infinity
            frac = 0;
        }
    }
    return (sign << 31) | (exp << 23) | frac;
}
