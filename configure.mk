# Make sure the first target in the makefile is the right one
configure:	configure.ac aclocal.m4 config.h.in
	autoconf

configure: \
m4/bison.m4 \
m4/c-bool.m4 \
m4/codeset.m4 \
m4/d-type.m4 \
m4/extern-inline.m4 \
m4/fcntl-o.m4 \
m4/flexmember.m4 \
m4/gettext.m4 \
m4/glibc2.m4 \
m4/glibc21.m4 \
m4/host-cpu-c-abi.m4 \
m4/iconv.m4 \
m4/intdiv0.m4 \
m4/intl-thread-locale.m4 \
m4/intl.m4 \
m4/intlmacosx.m4 \
m4/intmax.m4 \
m4/inttypes-pri.m4 \
m4/inttypes.m4 \
m4/inttypes_h.m4 \
m4/lcmessage.m4 \
m4/lib-ld.m4 \
m4/lib-link.m4 \
m4/lib-prefix.m4 \
m4/locale_h.m4 \
m4/lock.m4 \
m4/nls.m4 \
m4/po.m4 \
m4/printf-posix.m4 \
m4/progtest.m4 \
m4/pthread_rwlock_rdlock.m4 \
m4/size_max.m4 \
m4/stat-time.m4 \
m4/stdint_h.m4 \
m4/strtoimax.m4 \
m4/threadlib.m4 \
m4/timespec.m4 \
m4/uintmax_t.m4 \
m4/ulonglong.m4 \
m4/unlocked-io.m4 \
m4/visibility.m4 \
m4/wchar_t.m4 \
m4/wint_t.m4 \
m4/xsize.m4
