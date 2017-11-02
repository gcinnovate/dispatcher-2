#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.1)
AC_INIT([dispatcher2d], [2.1], sekiskylink@gmail.com)



dnl set build date
AC_MSG_CHECKING([Retrieving dispatcher2 build version])
BUILD_VERSION=`head -1 ChangeLog | cut -f 1 -d ' ' | sed 's/-//g'`

AC_MSG_RESULT([$BUILD_VERSION])

AC_CONFIG_AUX_DIR(autotools)
AC_CONFIG_SRCDIR([src/dispatcher2.c])
#AM_INIT_AUTOMAKE([dispatcher2d],[2.1])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER([src/dispatcher2-config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
PKG_PROG_PKG_CONFIG([0.25])
PKG_CHECK_MODULES([jansson], [jansson >= 2.7],
    [jansson_flags=`$PKG_CONFIG --cflags --libs jansson`],
    [jansson_flags=;])
if test "x$jansson_flags" = x; then
    AC_MSG_ERROR([jansson JSON C library not found])
else
    LIBS="$LIBS $jansson_flags"
fi

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h sys/time.h syslog.h unistd.h arpa/inet.h netdb.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([localtime_r memset mkdir strcasecmp strchr strerror strncasecmp strrchr strstr strtoul strtoull gethostname])

dnl Change a few things (a la kannel config)
EXE_EXT=""
LIB_EXT="a"
case "$host" in
  *-sun-solaris* | *SunOS*)
    CFLAGS="$CFLAGS -DSunOS=1 -D_POSIX_PTHREAD_SEMANTICS"
    ;;
  *-cygwin*)
    EXE_EXT=".exe"
    ;;
  *apple-darwin*)
    CFLAGS="$CFLAGS -DDARWIN=1 -O4 -Wall"
    LIB_EXT="dylib"
    ;;
  *-linux*)
    CFLAGS="$CFLAGS -D_XOPEN_SOURCE=600 -D_BSD_SOURCE -O4 -Wall"
    LDFLAGS="$LDFLAGS -rdynamic"
    ;;
  *-*-openbsd* | *-*-freebsd*)
    CFLAGS="$CFLAGS -pthread"
        AC_CHECK_LIB(c_r, pthread_exit, [LIBS="$LIBS -lc_r -lpthread"; pthread="yes"])
    ;;
esac


dnl Implement the --with-kannel-dir option
AC_ARG_WITH(kannel-dir,
[  --with-kannel-dir=DIR    where to look for Kannel Gateway libs and header files
                                                DIR points to the installation [/usr/local] ]
,
[
gwloc=""
if test -d "$withval" ; then
        gwloc="$withval"
fi
])

AC_PATH_PROG(GW_CONFIG, gw-config, no, [$gwloc/bin:$gwloc:$PATH])
dnl check for Kannel gw-config
if test "$GW_CONFIG" = "no"; then
found=""
        for loc in $pgsqlloc /usr /usr/local /opt/kannel; do
                if test "x$found" = "x" ; then
                        AC_MSG_CHECKING([for Kannel include files in])
                        AC_MSG_RESULT($loc)
                        AC_CHECK_FILE("$loc/bin/gw-config",
                                [CFLAGS="$CFLAGS `$loc/bin/gw-config --cflags` -DUSE_GWMEM_SLOW -DUSE_GWMEM_CHECK";
                                LIBS="$LIBS `$loc/bin/gw-config --libs`";
                                found=1
                                ])
                fi
        done;
        if test "x$found" != "x1" ; then
                AC_MSG_ERROR([Unable to find gw-config, please provide a --with-kannel-dir=
<dir> location])
        fi
else
        dnl gw_config found
        AC_MSG_CHECKING([Kannel version])
        gw_version=`$GW_CONFIG --version`
        AC_MSG_RESULT([$gw_version])
        AC_MSG_CHECKING([Kannel libs])
        if ! $GW_CONFIG --libs &>/dev/null ; then
                LIBS="$LIBS `$GW_CONFIG --libs`"
                gw_libdir=`$GW_CONFIG --libs`
                AC_MSG_RESULT([$gw_libdir])
        fi
        AC_MSG_CHECKING([Kannel includes])
        if ! $GW_CONFIG --cflags &>/dev/null ; then
                CFLAGS="$CFLAGS `$GW_CONFIG --cflags` -DUSE_GWMEM_SLOW -DUSE_GWMEM_CHECK"
                gw_incdir=`$GW_CONFIG --cflags`
                AC_MSG_RESULT([$gw_incdir])
        fi
fi

AC_CHECK_LIB([gwlib], [cfg_create], [], AC_MSG_ERROR([Kannel gwlib is required!]))
AC_CHECK_LIB([wap], [wsp_headers_pack], [], AC_MSG_ERROR([Kannel WAP lib is required!]))

dnl Implement the --with-pgsql-dir option.
pgsqlloc="/usr/local/pgsql"
AC_ARG_WITH(pgsql-dir,
[  --with-pgsql-dir=DIR    where to look for PostgreSQL libs and header files
                                                DIR points to the installation [/usr/local/pgsql] ],
[
pgsqlloc=""
if test -d "$withval" ; then
        pgsqlloc="$withval"
fi
])

AC_PATH_PROG(PGSQL_CONFIG, pg_config, no, [$pgsqlloc/bin:$pgsqlloc:/usr/lib/postgresql/bin:${PATH}])
dnl check for PgSQL >= 7.2 style pg_config information
if test "$PGSQL_CONFIG" = "no"; then
found=""
        for loc in $pgsqlloc /usr /usr/local ; do
                if test "x$found" = "x" ; then
                        AC_MSG_CHECKING([for PostgresSQL include files in])
                        AC_MSG_RESULT($loc)
                        AC_CHECK_FILE("$loc/include/postgresql/libpq-fe.h",
                                [CFLAGS="$CFLAGS -I$loc/include/postgresql"; LIBS="-L$loc/lib/postgresql -lpq $LIBS"]; found=1,
                                [AC_CHECK_FILE("$loc/include/pgsql/libpq-fe.h",
                                [CFLAGS="$CFLAGS -I$loc/include/pgsql"; LIBS=" -L$loc/lib/pgsql $LIBS -lpq"]; found=1,
                                [AC_CHECK_FILE("$loc/pgsql/include/libpq-fe.h",
                                        [CFLAGS="-I$loc/pgsql/include $CFLAGS"; LIBS="-L$loc/pgsql/lib $LIBS -lpq"]; found=1,
                                )]
                                )]
                        )
                fi
        done
        if test "x$found" != "x1" ; then
                AC_MSG_ERROR([Unable to find libpq-fe.h, please provide a --with-pgsql-dir=<dir> location])
        fi
else
        dnl pg_config found
        AC_MSG_CHECKING([PostgreSQL version])
        pgsql_version=`$PGSQL_CONFIG --version`
        AC_MSG_RESULT([$pgsql_version])
        AC_MSG_CHECKING([PostgreSQL libs])
        if $PGSQL_CONFIG --libdir &>/dev/null ; then
                LIBS="-L`$PGSQL_CONFIG --libdir` $LIBS"
                pg_libdir=`$PGSQL_CONFIG --libdir`
                AC_MSG_RESULT([$pg_libdir])
        fi
        AC_MSG_CHECKING([PostgreSQL includes])
        if $PGSQL_CONFIG --includedir &>/dev/null ; then
                CFLAGS="-I`$PGSQL_CONFIG --includedir` $CFLAGS"
                pg_incdir=`$PGSQL_CONFIG --includedir`
                AC_MSG_RESULT([$pg_incdir])
        fi
fi
AC_CHECK_LIB([pq], [PQexec])
# AC_CHECK_LIB([ssl], [SSL_library_init])
# AC_CHECK_LIB([pgtypes], [PGTYPESdate_fmt_asc])
#AC_CHECK_LIB([ecpg], [ECPGstatus])
AC_CHECK_LIB([crypto], [DES_set_key])
AC_CHECK_LIB([readline], [readline])

dnl Checking for libxslt
xslt_ver_required="1.0.0"
AC_PATH_PROGS(XSLT_CONFIG, xslt-config, no)
if test "$XSLT_CONFIG" = "no"; then
  AC_MSG_ERROR([You MUST have the libxslt library installed])
else
  AC_MSG_CHECKING([libxslt version])
  xslt_version=`$XSLT_CONFIG --version`
  AC_MSG_RESULT([$xslt_version])
  LIBS="$LIBS `$XSLT_CONFIG --libs`"
  CFLAGS="$CFLAGS `$XSLT_CONFIG --cflags`"
fi


dnl Implement the --with-ssl option.

AC_ARG_WITH(ssl,
[  --with-ssl[=DIR]          where to look for OpenSSL libs and header files
                          DIR points to the installation @<:@/usr/local/ssl@:>@],
[ if test -d "$withval"; then
    ssllib="$withval/lib";
    sslinc="$withval/include"
  else
    AC_MSG_ERROR(Unable to find OpenSSL libs and/or directories at $withval)
  fi
])

dnl Implement --enable-ssl option.

AC_MSG_CHECKING([whether to compile with SSL support])
AC_ARG_ENABLE(ssl,
[  --enable-ssl            enable SSL client and server support @<:@enabled@:>@], [
  if test "$enableval" = no ; then
    AC_MSG_RESULT(disabled)
    ssl=no
  else
    ssl=yes
  fi
],[
  ssl=yes
])

if test "$ssl" = "yes" ; then
  dnl test only if --with-ssl has not been used
  if test "x$ssllib" = "x" && test "x$sslinc" = "x"; then
    for loc in /usr/lib /usr/lib64 /usr/local/ssl/lib /usr/local/ssl/lib /opt/local/lib; do
      if test -f "$loc/libssl.a" -o -f "$loc/libssl.so" -o -f "$loc/libssl.dylib" ; then
        ssllib="$loc"
      fi
    done
    if test "x$sslib" = "x"; then
      for lib in libssl.a libssl.so libssl.dylib; do
        name=`gcc -print-file-name=$lib`
        if test -f "$name"; then
          ssllib=`dirname $name`
        fi
      done
    fi
    for loc in /usr/include/ssl /usr/include/openssl /usr/local/ssl/include \
               /usr/local/openssl/include /opt/local/include/openssl; do
      if test -d "$loc"; then
        sslinc="$loc"
      fi
    done
  fi
  AC_MSG_RESULT(trying $ssllib $sslinc)
fi

dnl Implement the SSL library checking routine.
dnl This will define HAVE_LIBSSL in gw-config.h

if test "x$ssllib" != "x" && test "x$sslinc" != "x"; then
  CFLAGS="$CFLAGS -I$sslinc"
  LIBS="$LIBS -L$ssllib"

  AC_PATH_PROG(OPENSSL, openssl, no)
  if test "$OPENSSL" = "yes"; then
    AC_MSG_CHECKING([openssl version])
    openssl_version=`$OPENSSL version | awk '{print $2}'`
    AC_MSG_RESULT([$openssl_version])
  fi
  AC_CHECK_LIB(crypto, CRYPTO_lock,
  [ LIBS="$LIBS -lcrypto"
    AC_CHECK_LIB(ssl, SSL_library_init,
    [ AC_CHECK_LIB(ssl, SSL_connect)
      AC_CHECK_HEADERS(openssl/x509.h openssl/rsa.h openssl/crypto.h \
                       openssl/pem.h openssl/ssl.h openssl/err.h \
                       openssl/hmac.h)
      AC_MSG_CHECKING(whether the OpenSSL library is multithread-enabled)
      AC_TRY_RUN([
        #define OPENSSL_THREAD_DEFINES
        #include <openssl/opensslconf.h>
        int main(void) {
        #if defined(THREADS)
          exit(0);
        #elif defined(OPENSSL_THREADS)
          exit(0);
        #else
          exit(1);
        #endif
        }
      ], [
        AC_MSG_RESULT(yes)
        AC_DEFINE(HAVE_LIBSSL)
        LIBS="$LIBS -lssl"
        AC_MSG_CHECKING([whether to compile with SSL support])
        AC_MSG_RESULT(yes)
      ], [
        AC_ARG_ENABLE(ssl-thread-test,
        [  --disable-ssl-thread-test  disable the multithread test for the OpenSSL library
                          this will force to continue even if the test fails],
        [ if test "$enableval" = no ; then
            AC_MSG_RESULT([no, continue forced])
          fi
        ], [
          AC_MSG_RESULT(no)
          AC_MSG_ERROR(Either get a multithread-enabled SSL or configure with --disable-ssl)
        ])
      ], echo "Cross-compiling; make sure your SSL library is multithread-enabled"
      )
    ])
  ])
fi

dnl define build version
AC_DEFINE_UNQUOTED(DISPATCHER2_BUILD_VERSION, "$BUILD_VERSION", "dispatcher2 Build Version")

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 contrib/dispatcher2d.spec])

AC_OUTPUT

cat<<X
License:
  dispatcher2 $VERSION (Build $BUILD_VERSION) -  Data Exchange Middleware

 Copyright (C) 2016, GoodCitizen Company Ltd. - http://www.gcinnovate.com

X

