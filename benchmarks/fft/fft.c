
#include <annot.h>

#define PI 3.14159
#define M_PI 3.14159

double ar[8];
double ai[8] = {0.,  };


static double fabs(double n)
{
  double f;

  if (n >= 0) {f = n;}
  else {f = -n;}
  return f;
}

static double log(double n)
{
   double result=1.0;

  /*if (n==2.0) {return(0.3);}
  else if (n==8.0) {return(0.9);}
  else return(1.0); /* just to return something IP */

  if (n==2.0) {result=0.3;}
  else if (n==8.0) {result=0.9;}
  return result;

}


static double sin(double rad)
{
  double app;

  double diff;
  int inc = 1;

  
  /*  while (rad > 2*PI) [0] {
      rad -= 2*PI;
      }
      
      while (rad < -2*PI) [0] {
      rad += 2*PI;
      }
  */

  app = diff = rad;
  diff = (diff * (-(rad*rad))) /
    ((2.0 * inc) * (2.0 * inc + 1.0));
  app = app + diff;
  inc++;

  while(fabs(diff) >= 0.00001) 

    {
      ANNOT_MAXITER(8);
      diff = (diff * (-(rad*rad))) /
	((2.0 * inc) * (2.0 * inc + 1.0));
      app = app + diff;
      inc++;
  }

  return(app);
}


static double cos(double rad)
{
  double sin();

  return (sin (PI / 2.0 - rad));
}






int fft1(int n, int flag)
{

  int i, j, k, it, xp, xp2, j1, j2, iter;
  double sign, w, wr, wi, dr1, dr2, di1, di2, tr, ti, arg;
  
  int result=0;
  
  if (n < 2) {
    
    result = 999;
    
  } else {
    
    iter = log((double)n)/log(2.0);
    j = 1;
#ifdef DEBUG 
    printf("iter=%d\n",iter);
#endif
    for(i = 0; i < iter; i++) 

      {
      ANNOT_MAXITER(4);
      j *= 2;
    }
    
    if(fabs(n-j) > 1.0e-6) {
      result=1;
    } else {
      
      /*  Main FFT Loops  */
      if (flag == 1) {sign=1.0;} else {sign = -1.0;}
      xp2 = n;
      for(it = 0; it < iter; it++) 

	{
	  ANNOT_MAXITER(4);
	  xp = xp2;
	  xp2 /= 2;
	  w = PI / xp2;
#ifdef DEBUG
	  printf("xp2=%d\n",xp2);
#endif
	  for(k = 0; k < xp2; k++) 
	    
	    {
	  ANNOT_MAXITER(4);
	  arg = k * w;
	  wr = cos(arg);
	  wi = sign * sin(arg);
	  i = k - xp;
	  
	  for(j = xp; j <= n; j += xp) 
	    {
	      ANNOT_MAXITER(4);
	      j1 = j + i;
	      j2 = j1 + xp2;
	      dr1 = ar[j1];
	      dr2 = ar[j2];
	      di1 = ai[j1];
	      di2 = ai[j2];
	      tr = dr1 - dr2;
	      ti = di1 - di2;
	      ar[j1] = dr1 + dr2;
	      ai[j1] = di1 + di2;
	      ar[j2] = tr * wr - ti * wi;
	      ai[j2] = ti * wr + tr * wi;
	    }
	    }
	}

      /*  Digit Reverse Counter  */
      
      j1 = n / 2;
      j2 = n - 1;
      j = 1;
#ifdef DEBUG
      printf("j2=%d\n",j2);
#endif
      for(i = 1; i <= j2; i++) 
	{
	  ANNOT_MAXITER(7);
	  if(i < j) {
	    tr = ar[j-1];
	    ti = ai[j-1];
	    ar[j-1] = ar[i-1];
	    ai[j-1] = ai[i-1];
	    ar[i-1] = tr;
	    ai[i-1] = ti;
	  }
	k = j1;

	while(k < j) 
	  {
	  ANNOT_MAXITER(2);
	  j -= k;
	  k /= 2;
	}
	j += k;
      }

      if(flag == 0) {
	result=0;
      } else {
	
	w = n;
	for(i = 0; i < n; i++)
	  {
	  ANNOT_MAXITER(8);
	  ar[i] /= w;
	  ai[i] /= w;
	}
      }
    }
  }

  return(result);
}

int main()
{

	int  i, n = 8, flag, chkerr;


	/* ar  */

	for(i = 0; i < n; i++)
	  {
	  ANNOT_MAXITER(8);
	  ar[i] = cos(2*M_PI*i/n);
	  ai[i] = 0.0;
	}

	/* forward fft */
	flag = 0;
	chkerr = fft1(n, flag);

	/* inverse fft */
	flag = 1;
	chkerr = fft1(n, flag);

}

