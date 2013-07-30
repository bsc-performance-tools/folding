# Process this file with autoconf to produce a configure script.

#########################
#    Initializations    #
#########################

# Initialize autoconf & define package name, version and bug-report address 
#  -- keep version in sync with src/common/extrae_version.h
AC_INIT(folding, 0.99, tools@bsc.es)
AC_CONFIG_HEADERS([folding-config.h:folding-config.h.in])

# Safety check to ensure that the directory told with `--srcdir` contains the source code
#AC_CONFIG_SRCDIR(src/mpitrace.c)

# GNU Autotools intermediate files are stored in the following directory
AC_CONFIG_AUX_DIR(config)

# Loads some shell variables like host_cpu and host_os, to get the host information 
AC_CANONICAL_SYSTEM

# Write defines in the output header file for the architecture and operating system
case "${target_cpu}" in
  armv7l)    Architecture="arm"
             AC_DEFINE([ARCH_ARM], [1], [Define if architecture is ARM]) ;;
  i*86|x86_64|amd64)
             Architecture="ia32"
             AC_DEFINE([ARCH_IA32], [1], [Define if architecture is IA32])
             if test "${target_cpu}" = "amd64" -o "${target_cpu}" = "x86_64" ; then
                AC_DEFINE([ARCH_IA32_x64], [1], [Define if architecture is IA32 (with 64bit extensions)])
             fi
             ;;
  powerpc* ) Architecture="powerpc"
             AC_DEFINE([ARCH_PPC], [1], [Define if architecture is PPC]) ;;
  ia64     ) Architecture="ia64"
             AC_DEFINE([ARCH_IA64], [1], [Define if architecture is IA64]) ;;
  alpha*   ) Architecture="alpha"
             AC_DEFINE([ARCH_ALPHA], [1], [Define if architecture is ALPHA]) ;;
  mips     ) Architecture="mips"
             AC_DEFINE([ARCH_MIPS], [1], [Define if architecture is MIPS]) ;;
esac

case "${target_os}" in
  linux*   ) OperatingSystem="linux"
             AC_DEFINE([OS_LINUX], [1], [Define if operating system is Linux]) ;;
  aix*     ) OperatingSystem="aix"
             AC_DEFINE([OS_AIX], [1], [Define if operating system is AIX]) ;;
  osf*     ) OperatingSystem="dec"
             AC_DEFINE([OS_DEC], [1], [Define if operating system is DEC]) ;;
  irix*    ) OperatingSystem="irix"
             AC_DEFINE([OS_IRIX], [1], [Define if operating system is IRIX]) ;;
  freebsd* ) OperatingSystem="freebsd"
             AC_DEFINE([OS_FREEBSD], [1], [Define if operating system is FreeBSD]) ;;
  solaris* ) OperatingSystem="solaris"
             AC_DEFINE([OS_SOLARIS], [1], [Define if operating system is Solaris]) ;;
  darwin*  ) OperatingSystem="darwin"
             AC_DEFINE([OS_DARWIN], [1], [Define if operating system is Darwin]) ;;
esac

# Publish these defines for conditional compilation 
AM_CONDITIONAL(ARCH_IA32,    test "${Architecture}"    = "ia32"    )
AM_CONDITIONAL(ARCH_POWERPC, test "${Architecture}"    = "powerpc" )
AM_CONDITIONAL(ARCH_IA64,    test "${Architecture}"    = "ia64"    )
AM_CONDITIONAL(ARCH_ALPHA,   test "${Architecture}"    = "alpha"   )
AM_CONDITIONAL(ARCH_MIPS,    test "${Architecture}"    = "mips"    )

AM_CONDITIONAL(OS_LINUX,     test "${OperatingSystem}" = "linux"   )
AM_CONDITIONAL(OS_AIX,       test "${OperatingSystem}" = "aix"     )
AM_CONDITIONAL(OS_DEC,       test "${OperatingSystem}" = "dec"     )
AM_CONDITIONAL(OS_IRIX,      test "${OperatingSystem}" = "irix"    )
AM_CONDITIONAL(OS_FREEBSD,   test "${OperatingSystem}" = "freebsd" )
AM_CONDITIONAL(OS_DARWIN,    test "${OperatingSystem}" = "darwin" )
AM_CONDITIONAL(OS_SOLARIS,   test "${OperatingSystem}" = "solaris" )

# Initialize automake
AM_INIT_AUTOMAKE
AM_PROG_MKDIR_P

# Specify the output configuration header file
# AM_CONFIG_HEADER(folding-config.h)

# Search for libtool support
#AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

# Search for C++, C and Fortran compilers
AC_PROG_CC
if test "${CC}" = "" ; then
	AC_MSG_ERROR([Cannot find C compiler])
fi
AC_PROG_CXX
if test "${CXX}" = "" ; then
	AC_MSG_ERROR([Cannot find C++ compiler])
fi
AC_PROG_FC
if test "${FC}" = "" ; then
	AC_MSG_ERROR([Cannot find Fortran compiler])
fi

AC_MSG_CHECKING([for libtools])
AC_ARG_WITH(libtools,
	AC_HELP_STRING(
		[--with-libtools@<:@=ARG@:>@],
		[Specify where libtools (libparaverconfig and libparavertraceparser) are installed]
	),
	[LibToolsDir="$withval"],
	[LibToolsDir=""]
)
if test ! -d ${LibToolsDir} ; then
	AC_MSG_ERROR([Invalid directory specified in --with-libtools])
fi

if test ! -f ${LibToolsDir}/include/ParaverRecord.h -o ! -f ${LibToolsDir}/include/ParaverTrace.h ; then
	AC_MSG_ERROR([Cannot find some header files of the libtools! Make sure that --with-libtools is pointing to the correct place.])
fi
if test ! -f ${LibToolsDir}/lib/libparavertraceconfig.a -a ! -f ${LibToolsDir}/lib64/libparavertraceconfig.a ; then
	AC_MSG_ERROR([Cannot find libparavertraceconfig.a! Make sure that --with-libtools is pointing to the correct place.])
fi
if test ! -f ${LibToolsDir}/lib/libparavertraceparser.a -a ! -f ${LibToolsDir}/lib64/libparavertraceparser.a ; then
	AC_MSG_ERROR([Cannot find libparavertraceparser.a! Make sure that --with-libtools is pointing to the correct place.])
fi
AC_MSG_RESULT([found in ${LibToolsDir}])
LIBTOOLS_DIR=${LibToolsDir}
AC_SUBST(LIBTOOLS_DIR)

AC_ARG_WITH(cube,
	AC_HELP_STRING(
		[--with-cube@<:@=ARG@:>@],
		[Specify where Cube (v 3.5.1 or higher) is installed]
	),
	[CubeDir="$withval"],
	[CubeDir="no"]
)
if test "${CubeDir}" != "no" ; then
	AC_MSG_CHECKING([for Cube])
	if test ! -d ${CubeDir} ; then
		AC_MSG_ERROR([Invalid directory specified in --with-cube])
	fi

	if test ! -f ${CubeDir}/include/cube3/Cube.h ; then
		AC_MSG_ERROR([Cannot find cube header files of the libtools! Make sure that --with-cube is pointing to the correct place.])
	fi
	if test ! -f ${CubeDir}/lib/libcube3.a ; then
		AC_MSG_ERROR([Cannot find libcube3.a! Make sure that --with-cube is pointing to the correct place.])
	fi
	AC_MSG_RESULT([found in ${CubeDir}])
	CUBE_DIR=${CubeDir}
	AC_SUBST(CUBE_DIR)
	CUBE_AVAILABLE="yes"
	AC_DEFINE([HAVE_CUBE], 1, [Defined CUBE is installed and available to use])
fi
AM_CONDITIONAL(HAVE_CUBE, test "${CUBE_AVAILABLE}" = "yes")

AC_MSG_CHECKING([for R statistical package])
AC_ARG_WITH(R,
	AC_HELP_STRING(
		[--with-R@<:@=ARG@:>@],
		[Specify where the R statistical package binary can be found]
	),
	[RBIN="$withval"],
	[RBIN=`which R`]
)
if test -x "${RBIN}" ; then
	AC_MSG_RESULT(${RBIN})

	AC_MSG_CHECKING(that R can process a simple script)
	echo "q()" > ${TMPDIR:-/tmp}/Rconftest
	${RBIN} -f ${TMPDIR:-/tmp}/Rconftest > ${TMPDIR:-/tmp}/Rconftest.out
	nlines=`grep "The R Foundation for Statistical Computing" ${TMPDIR:-/tmp}/Rconftest.out | wc -l`
	if test "${nlines}" -eq 1 ; then
		AC_MSG_RESULT([yes])
		R_AVAILABLE=yes

		AC_MSG_CHECKING(that R has the strucchange package)
		echo "library(strucchange)" > ${TMPDIR:-/tmp}/Rconftest
		echo "Nile" >> ${TMPDIR:-/tmp}/Rconftest
		echo "q()" >> ${TMPDIR:-/tmp}/Rconftest
		${RBIN} -f ${TMPDIR:-/tmp}/Rconftest > ${TMPDIR:-/tmp}/Rconftest.out 2> /dev/null
		nlines=`grep "Time Series" ${TMPDIR:-/tmp}/Rconftest.out | wc -l`
		if test "${nlines}" -eq 1 ; then
			R_STRUCCHANGE_AVAILABLE=yes
			AC_MSG_RESULT([yes])
		else
			AC_MSG_RESULT([no])
		fi
	else
		AC_MSG_RESULT([no])
	fi
else
	AC_MSG_RESULT([Not found])
fi
if test "${R_STRUCCHANGE_AVAILABLE}" = "yes" ; then
	AC_MSG_NOTICE([Segmented linear regression through R+strucchange is available])
	AC_DEFINE([HAVE_R],[1],[Define if have R available])
	AC_DEFINE_UNQUOTED([R_BINARY],["${RBIN}"],[Define binary for R])
elif test "${R_AVAILABLE}" = yes ; then
	AC_MSG_NOTICE([Segmented linear regression through R is not available because strucchange library is missing. Execute install.packages("strucchange") in a R session])
else
	AC_MSG_NOTICE([Segmented linear regression is not available as R cannot be find])
fi

AC_MSG_CHECKING([for clang compiler])
AC_ARG_WITH(clang,
	AC_HELP_STRING(
		[--with-clang@<:@=ARG@:>@],
		[Specify where the clang compiler]
	),
	[CLANGBIN="$withval"],
	[CLANGBIN=`which clang`]
)
if test -x "${CLANGBIN}" ; then
	AC_MSG_RESULT(${CLANGBIN})
	CLANG_AVAILABLE="yes"
fi
if test "${CLANG_AVAILABLE}" = "yes" ; then
	AC_MSG_NOTICE([clang is available and ready for providing AST information])
	AC_DEFINE([HAVE_CLANG],[1],[Define if have clang available])
	AC_DEFINE_UNQUOTED([CLANG_BINARY],["${CLANGBIN}"],[Define binary for clang])
else
	AC_MSG_NOTICE([clang is not available. AST parsing for C/C++ files will not be available])
fi

AC_MSG_CHECKING([for Clustering suite])
AC_ARG_WITH(clustering-suite,
    AC_HELP_STRING(
        [--with-clustering-suite@<:@=ARG@:>@],
        [Specify where to locate the clustering suite]
   ),
   [CLUSTERING_HOME="${withval}"],
   [CLUSTERING_HOME="none"]
)
if test "${CLUSTERING_HOME}" != "none" ; then
   AC_MSG_RESULT(${CLUSTERING_HOME})

   AC_MSG_CHECKING([for headers and libraries of the Clustering suite])
   if test -r ${CLUSTERING_HOME}/include/libClustering.hpp -a \
           -r ${CLUSTERING_HOME}/include/Partition.hpp -a \
           -r ${CLUSTERING_HOME}/include/Point.hpp ; then
      if test -r ${CLUSTERING_HOME}/lib/libClustering.so -a \
              -r ${CLUSTERING_HOME}/lib/libBasicClasses.so ; then
         AC_MSG_RESULT([found])
         AC_DEFINE([HAVE_CLUSTERING_SUITE], [1], [Define if have clustering suite available])
         AC_SUBST([CLUSTERING_HOME])
         CLUSTERING_SUITE_FOUND="yes"
      fi
   fi
   if test ${CLUSTERING_SUITE_FOUND} != "yes" ; then
      AC_MSG_ERROR([Cannot find the headers or the libraries for the Clustering suite])
   fi
fi
AM_CONDITIONAL(HAVE_CLUSTERING_SUITE, test "${CLUSTERING_SUITE_FOUND}" = "yes" )

dnl Example of default-disabled feature
AC_ARG_ENABLE([gui],
    AS_HELP_STRING([--enable-gui], [Enables GUI frontend based on wxPython]))

AM_CONDITIONAL(WANT_GUI_FRONTEND, test "${enable_gui}" = "yes" )

AX_PROG_BOOST

AC_CHECK_FUNC(stat, [AC_DEFINE([HAVE_STAT],[1],[Define if have stat])])
AC_CHECK_FUNC(stat64, [AC_DEFINE([HAVE_STAT64],[1],[Define if have stat64])])
AC_CHECK_FUNC(access, [AC_DEFINE([HAVE_ACCESS],[1],[Define if have access])])

AC_CONFIG_FILES([Makefile src/Makefile src/service-kriger/Makefile src/service-R/Makefile])
AC_OUTPUT
