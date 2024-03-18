# Check for bool that conforms to C2023.

dnl Copyright 2022-2024 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_C_BOOL],
[
  AC_CACHE_CHECK([for bool, true, false], [gl_cv_c_bool],
    [AC_COMPILE_IFELSE(
       [AC_LANG_SOURCE([[
          #if true == false
           #error "true == false"
          #endif
          extern bool b;
          bool b = true == false;]])],
       [gl_cv_c_bool=yes],
       [gl_cv_c_bool=no])])
  if test "$gl_cv_c_bool" = yes; then
    AC_DEFINE([HAVE_C_BOOL], [1],
      [Define to 1 if bool, true and false work as per C2023.])
  fi

  AC_CHECK_HEADERS_ONCE([stdbool.h])

])
