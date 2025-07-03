# unlocked-io.m4 serial 16

# Copyright (C) 1998-2006, 2009-2024 Free Software Foundation, Inc.
#
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

dnl From Jim Meyering.
dnl
dnl Adapted from gnulib:m4/unlocked-io.m4
AC_DEFUN([BASH_FUNC_UNLOCKED_IO],
[
  dnl Persuade glibc and Solaris <stdio.h> to declare
  dnl fgets_unlocked(), fputs_unlocked() etc.
  AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])

  AC_CHECK_DECLS_ONCE([clearerr_unlocked])
  AC_CHECK_DECLS_ONCE([feof_unlocked])
  AC_CHECK_DECLS_ONCE([ferror_unlocked])
  AC_CHECK_DECLS_ONCE([fflush_unlocked])
  AC_CHECK_DECLS_ONCE([fgets_unlocked])
  AC_CHECK_DECLS_ONCE([fputc_unlocked])
  AC_CHECK_DECLS_ONCE([fputs_unlocked])
  AC_CHECK_DECLS_ONCE([fread_unlocked])
  AC_CHECK_DECLS_ONCE([fwrite_unlocked])
  AC_CHECK_DECLS_ONCE([getc_unlocked])
  AC_CHECK_DECLS_ONCE([getchar_unlocked])
  AC_CHECK_DECLS_ONCE([putc_unlocked])
  AC_CHECK_DECLS_ONCE([putchar_unlocked])
])
