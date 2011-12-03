*** ../bash-3.0/arrayfunc.c	Fri Dec 19 00:03:09 2003
--- arrayfunc.c	Sun Aug  1 20:43:00 2004
***************
*** 612,616 ****
  
    free (t);
!   return var;
  }
  
--- 612,616 ----
  
    free (t);
!   return (var == 0 || invisible_p (var)) ? (SHELL_VAR *)0 : var;
  }
  
