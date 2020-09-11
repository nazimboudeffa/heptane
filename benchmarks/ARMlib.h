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

#ifndef ARM_LIB
#define ARM_LIB

 /* Fonctions de libgcc redefinies ici (functions sans retour car branchement à une adresse d'une autre fonction).*/

// int
void __aeabi_idiv() {}
void __aeabi_i2d() {}
void __aeabi_uidivmod() {}
void __aeabi_fcmpge() {}
void __aeabi_i2f() {}


// double
void __aeabi_ddiv() {}
void __aeabi_dsub() {}
void __aeabi_dmul() {}
void __aeabi_dcmpge() {}
void __aeabi_dcmpeq() {}
void __aeabi_dcmpgt() {}
void __aeabi_dcmple(){}
void __aeabi_d2f(){}
void __aeabi_dadd() {}
void __aeabi_d2iz() {}

// float
void __aeabi_fcmplt(){}
void __aeabi_fcmpgt(){}
void __aeabi_fdiv(){}
void __aeabi_fcmpeq(){}
void __aeabi_fmul(){}
void __aeabi_fsub(){}
void __aeabi_f2d(){}
void __aeabi_fadd(){}


void __aeabi_ui2(){}
void __aeabi_ui2d(){}

void __aeabi_dcmpun(){}
void __aeabi_dcmplt(){}

#endif
