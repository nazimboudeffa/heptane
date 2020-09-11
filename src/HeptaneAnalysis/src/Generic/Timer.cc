/* ---------------------------------------------------------------------

Copyright IRISA, 2003-2017

This file is part of Heptane, a tool for Worst-Case Execution Time (WCET)
estimation.
APP deposit IDDN.FR.001.510039.000.S.P.2003.000.10600

Heptane is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Heptane is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details (COPYING.txt).

See CREDITS.txt for credits of authorship

------------------------------------------------------------------------ */

/*****************************************************************
                             Timer
 ****************************************************************/

#include "Timer.h"
#include <sys/time.h>

using namespace std;




//=================Timer::initTimer()=========
/*
* initialisation of the timer
*											*/
//==============================================
void
Timer::initTimer ()
{
  gettimeofday (&timerAbsolute, NULL);
}


//=================Timer::addTimer()=======
/*
* add the timer value to the parameter
*										*/
//=========================================
void
Timer::addTimer (float &time)
{
  struct timeval timerActual;
  gettimeofday (&timerActual, NULL);
  time += timerActual.tv_sec - timerAbsolute.tv_sec;
  time += (float (timerActual.tv_usec) - float (timerAbsolute.tv_usec)) *0.000001;
  timerAbsolute = timerActual;
}
