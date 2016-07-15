# AX_LIBBSCTOOLS
# -----------
AC_DEFUN([AX_LIBBSCTOOLS],
[

  HaveLibBSCTools="yes"
  AC_MSG_CHECKING([for libbsctools])
  AC_ARG_WITH(libbsctools,
        AC_HELP_STRING(
                [--with-libbsctools@<:@=ARG@:>@],
                [Specify where libbsctools (libparaverconfig and libparavertraceparser) are installed]
        ),
        [LibBSCToolsDir="$withval"],
        [LibBSCToolsDir=""]
  )
  if test ! -d ${LibBSCToolsDir} ; then
        AC_MSG_ERROR([Invalid directory specified in --with-libbsctools])
  fi
  if test ! -f ${LibBSCToolsDir}/include/ParaverRecord.h -o ! -f ${LibBSCToolsDir}/include/ParaverTrace.h ; then
        HaveLibBSCTools="no"
        AC_MSG_WARN([Cannot find some header files of the libbsctools! Make sure that --with-libbsctools is pointing to the correct place.])
  else
        AC_MSG_RESULT([found in ${LibBSCToolsDir}])
  fi

  AC_MSG_CHECKING([for libbsctools libraries placement])
  if test -f ${LibBSCToolsDir}/lib/libparavertraceconfig.a -a \
          -f ${LibBSCToolsDir}/lib/libparavertraceparser.a ; then
          LibBSCToolsDir_Lib=${LibBSCToolsDir}/lib
          AC_MSG_RESULT([found in ${LibBSCToolsDir_Lib}])
  elif test -f ${LibBSCToolsDir}/lib64/libparavertraceconfig.a -a \
            -f ${LibBSCToolsDir}/lib64/libparavertraceparser.a; then
          LibBSCToolsDir_Lib=${LibBSCToolsDir}/lib64
          AC_MSG_RESULT([found in ${LibBSCToolsDir_Lib}])
  else
          HaveLibBSCTools="no"
          AC_MSG_WARN([Cannot find libparavertraceconfig.a! Make sure that --with-libbsctools is pointing to the correct place.])
  fi

  AC_MSG_CHECKING([whether libbsctools libraries have shared versions])
  if test -f ${LibBSCToolsDir_Lib}/libparavertraceconfig.so -a \
          -f ${LibBSCToolsDir_Lib}/libparavertraceparser.so ; then
          LibBSCToolsDir_HasShared="yes"
  else
          LibBSCToolsDir_HasShared="no"
  fi
  AC_MSG_RESULT([${LibBSCToolsDir_HasShared}])

  AC_MSG_CHECKING([for prvparser-config])
  if test ! -f ${LibBSCToolsDir}/bin/prvparser-config ; then
    HaveLibBSCTools="no"
    AC_MSG_RESULT([no])
  else
    AC_MSG_RESULT([yes])
  fi

  if test "${HaveLibBSCTools}" = "yes" ; then
    LIBBSCTOOLS_DIR=${LibBSCToolsDir}
    LIBBSCTOOLS_LIB_DIR=${LibBSCToolsDir_Lib}
    LIBBSCTOOLS_CFLAGS="-I$LIBBSCTOOLS_DIR/include"
    LIBBSCTOOLS_CXXFLAGS="-I$LIBBSCTOOLS_DIR/include"
    LIBBSCTOOLS_LDFLAGS="-L$LIBBSCTOOLS_LIB_DIR -lparavertraceparser"
    PRVPARSER_CONFIG="${LibBSCToolsDir}/bin/prvparser-config"
    AC_SUBST(LIBBSCTOOLS_DIR)
    AC_SUBST(LIBBSCTOOLS_LIB_DIR)
    AC_SUBST(LIBBSCTOOLS_CFLAGS)
    AC_SUBST(LIBBSCTOOLS_CXXFLAGS)
    AC_SUBST(LIBBSCTOOLS_LDFLAGS)
    AC_SUBST(PRVPARSER_CONFIG)
    AC_DEFINE(HAVE_LIBBSCTOOLS, 1,[Define if libbsctools are available])
  fi

  AM_CONDITIONAL(LIBBSCTOOLS_HAS_SHARED_LIBRARIES, test "${LibBSCToolsDir_HasShared}" = "yes")
  AM_CONDITIONAL(HAVE_LIBBSCTOOLS, test "${HaveLibBSCTools}" = "yes")
])

