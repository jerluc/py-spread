# Local macros for automake & autoconf

AC_DEFUN([SPREAD_FUNCTION_CHECKS],[

# Standard Spread list
AC_CHECK_FUNCS(snprintf sprintf\
	memset malloc free \
	socket strnlen \
	alarm bmove \
  chsize ftruncate rint finite fpsetmask fpresetsticky\
  cuserid fcntl fconvert  \
  getrusage getpwuid getcwd getrlimit getwd index locking longjmp \
  perror pread realpath rename \
	bzero )

# This is special for libspread
AC_CHECK_FUNCS(strtok_r)
])

#---START: Used in for client configure
AC_DEFUN([SPREAD_CHECK_ULONG],
[AC_MSG_CHECKING(for type ulong)
AC_CACHE_VAL(ac_cv_ulong,
[AC_TRY_RUN([#include <stdio.h>
#include <sys/types.h>
main()
{
  ulong foo;
  foo++;
  exit(0);
}], ac_cv_ulong=yes, ac_cv_ulong=no, ac_cv_ulong=no)])
AC_MSG_RESULT($ac_cv_ulong)
if test "$ac_cv_ulong" = "yes"
then
  AC_DEFINE(HAVE_ULONG,,[ ])
fi
])

AC_DEFUN([SPREAD_CHECK_UCHAR],
[AC_MSG_CHECKING(for type uchar)
AC_CACHE_VAL(ac_cv_uchar,
[AC_TRY_RUN([#include <stdio.h>
#include <sys/types.h>
main()
{
  uchar foo;
  foo++;
  exit(0);
}], ac_cv_uchar=yes, ac_cv_uchar=no, ac_cv_uchar=no)])
AC_MSG_RESULT($ac_cv_uchar)
if test "$ac_cv_uchar" = "yes"
then
  AC_DEFINE(HAVE_UCHAR,,[ ])
fi
])

AC_DEFUN([SPREAD_CHECK_UINT],
[AC_MSG_CHECKING(for type uint)
AC_CACHE_VAL(ac_cv_uint,
[AC_TRY_RUN([#include <stdio.h>
#include <sys/types.h>
main()
{
  uint foo;
  foo++;
  exit(0);
}], ac_cv_uint=yes, ac_cv_uint=no, ac_cv_uint=no)])
AC_MSG_RESULT($ac_cv_uint)
if test "$ac_cv_uint" = "yes"
then
  AC_DEFINE(HAVE_UINT,,[ ])
fi
])

AC_DEFUN([SPREAD_CHECK_USHORT],
[AC_MSG_CHECKING(for type ushort)
AC_CACHE_VAL(ac_cv_ushort,
[AC_TRY_RUN([#include <stdio.h>
#include <sys/types.h>
main()
{
  ushort foo;
  foo++;
  exit(0);
}], ac_cv_ushort=yes, ac_cv_ushort=no, ac_cv_ushort=no)])
AC_MSG_RESULT($ac_cv_ushort)
if test "$ac_cv_ushort" = "yes"
then
  AC_DEFINE(HAVE_USHORT,,[ ])
fi
])

AC_DEFUN([SPREAD_CHECK_INT_8_16_32],
[AC_MSG_CHECKING([for int8])
AC_CACHE_VAL(ac_cv_int8,
[AC_TRY_RUN([
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

int main()
{
    int8 i;
    return 0;
}
], ac_cv_int8=yes, ac_cv_int8=no, ac_cv_int8=no)])
AC_MSG_RESULT($ac_cv_int8)
if test "$ac_cv_int8" = "yes"
then
  AC_DEFINE(HAVE_INT_8_16_32,,[ ])
fi
])

AC_DEFUN([SPREAD_HEADER_CHECKS],[
AC_HEADER_STDC
AC_CHECK_HEADERS(sgtty.h sys/ioctl.h \
 fcntl.h float.h floatingpoint.h ieeefp.h limits.h \
 memory.h pwd.h select.h \
 stdlib.h stddef.h \
 strings.h string.h synch.h sys/mman.h sys/socket.h \
 sys/timeb.h sys/types.h sys/un.h sys/vadvise.h sys/wait.h term.h \
 unistd.h utime.h sys/utime.h termio.h termios.h sched.h crypt.h alloca.h)
])

AC_DEFUN([SPREAD_TYPE_CHECKS],[

AC_REQUIRE([AC_C_CONST])
AC_REQUIRE([AC_C_INLINE])
AC_CHECK_SIZEOF(char, 1)

AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(long long, 8)
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_TYPE_UID_T

SPREAD_CHECK_ULONG
SPREAD_CHECK_UCHAR
SPREAD_CHECK_UINT
SPREAD_CHECK_USHORT
SPREAD_CHECK_INT_8_16_32

AC_REQUIRE([AC_TYPE_SIGNAL])
])