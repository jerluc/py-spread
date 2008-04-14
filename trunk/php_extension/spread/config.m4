dnl $Id$
dnl config.m4 for extension spread

sinclude(ext/spread/libspread/acinclude.m4)
sinclude(ext/spread/libspread/spread.m4)
sinclude(libspread/acinclude.m4)
sinclude(libspread/spread.m4)

PHP_ARG_WITH(spread, for spread support,
[  --with-spread             Include spread support])

PHP_ARG_ENABLE(spread, whether to enable spread support,
[  --enable-spread           Enable spread support])

if test "$PHP_SPREAD" = "yes"; then
  SPREAD_CHECKS
  sources="libspread/fl.c libspread/scatp.c libspread/alarm.c libspread/events.c libspread/memory.c libspread/sp.c \
		libspread/stdarr.c libspread/stdcarr.c libspread/stddll.c libspread/stderror.c libspread/stdfd.c \
		libspread/stdhash.c libspread/stdit.c libspread/stdskl.c libspread/stdthread.c libspread/stdtime.c libspread/stdutil.c"
  PHP_NEW_EXTENSION(spread, php_spread.c $sources, $ext_shared,,-I@ext_srcdir@/libspread -DHAVE_SIGNAL_H)
  PHP_ADD_BUILD_DIR($ext_builddir/libspread)
fi
