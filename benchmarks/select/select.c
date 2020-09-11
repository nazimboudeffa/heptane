/*************************************************************************/  
/*                                                                       */ 
/*   SNU-RT Benchmark Suite for Worst Case Timing Analysis               */ 
/*   =====================================================               */ 
/*                              Collected and Modified by S.-S. Lim      */ 
/*                                           sslim@archi.snu.ac.kr       */ 
/*                                         Real-Time Research Group      */ 
/*                                        Seoul National University      */ 
/*                                                                       */ 
/*                                                                       */ 
/*        < Features > - restrictions for our experimental environment   */ 
/*                                                                       */ 
/*          1. Completely structured.                                    */ 
/*               - There are no unconditional jumps.                     */ 
/*               - There are no exit from loop bodies.                   */ 
/*                 (There are no 'break' or 'return' in loop bodies)     */ 
/*          2. No 'switch' statements.                                   */ 
/*          3. No 'do..while' statements.                                */ 
/*          4. Expressions are restricted.                               */ 
/*               - There are no multiple expressions joined by 'or',     */ 
/*                'and' operations.                                      */ 
/*          5. No library calls.                                         */ 
/*               - All the functions needed are implemented in the       */ 
/*                 source file.                                          */ 
/*                                                                       */ 
/*                                                                       */ 
/*************************************************************************/ 
/*                                                                       */ 
/*  FILE: select.c                                                       */ 
/*  SOURCE : Numerical Recipes in C - The Second Edition                 */ 
/*                                                                       */ 
/*  DESCRIPTION :                                                        */ 
/*                                                                       */ 
/*     A function to select the Nth largest number in the floating poi-  */ 
/*     nt array arr[].                                                   */ 
/*     The parameters to function select are k and n. Then the function  */ 
/*     selects k-th largest number out of n original numbers.            */ 
/*                                                                       */ 
/*  REMARK :                                                             */ 
/*                                                                       */ 
/*  EXECUTION TIME :                                                     */ 
/*                                                                       */ 
/*                                                                       */ 
/*************************************************************************/ 

#include <annot.h>
#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;

float arr[20] = {
  5, 4, 10.3, 1.1, 5.7, 100, 231, 111, 49.5, 99,
  10, 150, 222.22, 101, 77, 44, 35, 20.54, 99.99, 888.88
};

float select(unsigned long k, unsigned long n)
{
  unsigned long i,ir,j,l,mid;
  float a,temp;
  int flag, flag2;

  l=0;
  ir=n-1;
  flag = flag2 = 0; 
  
  while (!flag) {
    ANNOT_MAXITER(7);
    if (ir <= l+1) {
      if (ir == l+1) 
	if (arr[ir] < arr[l]) {
	  SWAP(arr[l],arr[ir])
	    }
      flag = 1;
    } else 
      {
	i=l+1;
	j=ir;
	a=arr[l];
	flag2 = 0;

	while (!flag2) {
	  ANNOT_MAXITER(5); 
	  while (arr[i] < a) 
	    {
	      ANNOT_MAXITER(12); 
	      i++;
	    }

	  while (arr[j] > a)
	    {
	      ANNOT_MAXITER(16);
	      j--;
	    }
	  if (j < i)
	    flag2 = 1;
	  else
	    {
	      SWAP(arr[i],arr[j]);
	    }
	}
	
	arr[l]=arr[j];
	arr[j]=a;
	if (j >= k) { ir=j-1;} 
	if (i <= k) l=i;
      }

  }
  return arr[k-1];
}

main()
{
  select(10, 20);
}

/* #include <stdio.h>

void main()
{
  int kieme= 15;
  float val = select(kieme, 20);
  printf("For array[20], the %d th largest value is: %g\n", kieme, val);
}
 */

