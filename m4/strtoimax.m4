dnl Copyright (C) 2022 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Make sure we replace strtoimax if we don't have a declaration
dnl We can use this as a template for future function checks

AC_DEFUN([BASH_FUNC_STRTOIMAX],
[
  AC_CHECK_FUNCS([strtoimax])
  AC_CHECK_DECLS([strtoimax])

  if test "$ac_cv_func_strtoimax" != "yes" ; then
    AC_LIBOBJ(strtoimax)
  fi
])
