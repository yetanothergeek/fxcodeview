AC_INIT([fxcodeview],0.1, [yetanothergeek@gmail.com])

PACKAGE="fxcodeview"
VERSION=0.1

AC_SUBST([VERSION])
AC_SUBST([PACKAGE])

AM_INIT_AUTOMAKE([-Wall])

# AM_GNU_GETTEXT([external])

AC_PROG_CC
AC_PROG_CXX
AC_LANG_C
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_HEADER_STDC

dnl #Let pkgconfig handle FOX flags unless they are set in the environment
if test "x${FOX_LIBS}${FOX_CFLAGS}" = "x"
then
  PKG_PROG_PKG_CONFIG()
  FOX_PKG_NAME=fox
  for L in fox18 fox17 fox
  do
    AC_MSG_CHECKING([for $L.pc])
    if ${PKG_CONFIG} --exists ${L}
    then
      AC_MSG_RESULT([yes])
      FOX_PKG_NAME=${L}
      break
    else
      AC_MSG_RESULT([no])
    fi
  done
  PKG_CHECK_MODULES([FOX], [${FOX_PKG_NAME} >= 1.7],
    [
      AC_MSG_RESULT([Found FOX-1.7 or newer])
      AC_DEFINE(FOX_1_7)
    ]
  )
else
  if test "x${FOX_LIBS}" = "x"
  then
    AC_MSG_ERROR([If you set FOX_CFLAGS, you must also set FOX_LIBS])
  fi
  if test "x${FOX_CFLAGS}" = "x"
  then
    AC_MSG_ERROR([If you set FOX_LIBS, you must also set FOX_CFLAGS])
  fi
fi


AC_CANONICAL_HOST
case $host_os in
  *mingw*)
    FOX_LIBS="${FOX_LIBS} -lgdi32 -limm32 -lws2_32 -lole32 -luuid -mwindows"
  ;;
  *)
    PKG_CHECK_MODULES([X11],[x11])
  ;;
esac


case "${srcdir}" in
  */*)
    JEFFX_CFLAGS="-I${srcdir}/jef"
  ;;
  *)
    JEFFX_CFLAGS="-I${srcdir}/../jef"
  ;;
esac

AC_SUBST(FOX_CFLAGS)
AC_SUBST(FOX_LIBS)

AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LIBS)

AC_SUBST(JEFFX_CFLAGS)

AC_OUTPUT( Makefile src/Makefile jef/Makefile)




