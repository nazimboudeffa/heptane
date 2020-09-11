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
/*  FILE: minver.c                                                       */
/*  SOURCE : Turbo C Programming for Engineering by Hyun Soo Ahn         */
/*                                                                       */
/*  DESCRIPTION :                                                        */
/*                                                                       */
/*     Matrix inversion for 3x3 floating point matrix.                   */
/*                                                                       */
/*  REMARK :                                                             */
/*                                                                       */
/*  EXECUTION TIME :                                                     */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

#include <annot.h>

int minver(int row, int col, double eps);
int  mmul(int  row_a, int col_a, int row_b, int col_b);
double eps;

static double  a[3][3] = {
  3.0, -6.0,  7.0,
  9.0,  0.0, -5.0,
  5.0, -8.0,  6.0,
};
double  b[3][3], c[3][3], aa[3][3], a_i[3][3], e[3][3], det;
double m1[3][3];
double mt[3][3];



int main()
{
	int i, j;

	eps = 1.0e-6;

	/* Init input matrix */
	/* Done here so that the nb of instr of the appli is constant
           when executed periodically */
	a[0][0] = 3.0;
	a[0][1] = -6.0;
	a[0][2] = 7.0;
	a[1][0] = 9.0;
	a[1][1] = 0.0;
	a[1][2] = -5.0;
	a[2][0] = 5.0;
	a[2][1] = -8.0;
	a[2][2] = 6.0;

	for(i = 0; i < 3; i++)
	  {
	    ANNOT_MAXITER(3);
	  for(j = 0; j < 3; j++)
{
	    ANNOT_MAXITER(3);
	    aa[i][j] = a[i][j];
	  }
	}

	minver(3, 3, eps);

	for(i = 0; i < 3; i++) 
	  {
	    ANNOT_MAXITER(3);
	  for(j = 0; j < 3; j++) 
	    {
	    ANNOT_MAXITER(3);
	    a_i[i][j] = a[i][j];
	  }
	}

	mmul(3, 3, 3, 3);
	return 0;
}


int  mmul(int row_a, int col_a, int row_b, int col_b)
{
	 int i, j, k, row_c, col_c;
	 double w;

	int result=0; /*DAMIEN*/

	 row_c = row_a;
	 col_c = col_b;

	/* if(row_c < 1 || row_b < 1 || col_c < 1 || col_a != row_b) {result=999;} */
	/*DAMIEN*/
	 /*if(row_c < 1 ) {result=999;}
if( row_b < 1 ) {result=999;}
if( col_c < 1 ) {result=999;}
if( col_a != row_b) {result=999;}*/

	


	 for(i = 0; i < row_c; i++) 
	 {
	   ANNOT_MAXITER(3);
	   for(j = 0; j < col_c; j++) 
	     {
	       ANNOT_MAXITER(3);
	       w = 0.0;
	       for(k = 0; k < row_b; k++) 
		 {
		   ANNOT_MAXITER(3);
		   w += a[i][k] * b[k][j];
	       }
	       c[i][j] = w;
	     }
	 }
	 /*return(0);*/
	return result; /*DAMIEN*/

}


int minver(int row, int col, double eps)
{
  /*   int fin=0;
	int work[3], i, j, k, r, iw, s, t, u, v;
	double w, wmax, pivot, api, w1;
  */
  int i,j,v1,v2,first,second;
  int ac,al,bc,bl,cc,cl,dc,dl,ec,el,fc,fl,gc,gl,hc,hl,ic,il;

  /*determinant*/
  det = 0;
  for (i=0;i<row;i++)
    {
      ANNOT_MAXITER(3);
      v1= (i+1) %3 ;
      v2= (i+2) %3 ;

      /*DAMIEN pour analyse de cache*/
      /*   if(v1>v2){first=v2;second=v1;}
	   else{first=v1;second=v2;}*/first=v1;second=v2;
      det=a[0][i] * (a[1][first]*a[2][second] - a[1][second]*a[2][first]);   
    }

  det = 1/det;

  /*matrice*/
  ac=0;al=0;
  bc=1;bl=0;
  cc=2;cl=0;
  dc=0;dl=1;
  ec=1;el=1;
  fc=2;fl=1;
  gc=0;gl=2;
  hc=1;hl=2;
  ic=2;il=2;

  m1[0][0]=a[el][ec]*a[il][ic]-a[fl][fc]*a[hl][hc];
  m1[0][1]=a[fl][fc]*a[gl][gc]-a[dl][dc]*a[il][ic];
  m1[0][2]=a[dl][dc]*a[hl][hc]-a[el][ec]*a[gl][gc];
  m1[1][0]=a[cl][cc]*a[hl][hc]-a[bl][bc]*a[il][ic];
  m1[1][1]=a[al][ac]*a[il][ic]-a[cl][cc]*a[gl][gc];
  m1[1][2]=a[bl][bc]*a[gl][gc]-a[al][ac]*a[hl][hc];
  m1[2][0]=a[bl][bc]*a[fl][fc]-a[cl][cc]*a[el][ec];
  m1[2][1]=a[cl][cc]*a[dl][dc]-a[al][ac]*a[fl][fc];
  m1[2][2]=a[al][ac]*a[el][ec]-a[bl][bc]*a[dl][dc];

  /*transposition*/

  for(i=0;i<row;i++)
{
      ANNOT_MAXITER(3);
      for(j=0;j<row;j++)
{
      ANNOT_MAXITER(3);
      mt[j][i]=m1[i][j];
    }
  }

  /*multiplication*/
  for(i=0;i<row;i++)
{
  ANNOT_MAXITER(3);
  for(j=0;j<row;j++)
{
 ANNOT_MAXITER(3);
 a[i][j]=det*mt[i][j];
    }
  }

  return 0;
	
}





