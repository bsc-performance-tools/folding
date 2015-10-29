AC_DEFUN([AX_HAVE_CUBE],[
	AC_ARG_WITH(cube,
		AC_HELP_STRING(
			[--with-cube@<:@=ARG@:>@],
			[Specify where Cube (v 4.3 or higher) is installed]
		),
		[CubeDir="$withval"],
		[CubeDir="no"]
	)
	if test "${CubeDir}" != "no" ; then
		AC_MSG_CHECKING([for Cube])
		if test ! -d ${CubeDir} ; then
			AC_MSG_ERROR([Invalid directory specified in --with-cube])
		fi
		if test ! -x ${CubeDir}/bin/cube-config ; then
			AC_MSG_ERROR([Cannot locate cube-config within ${CubeDir}/bin])
		fi
		AC_MSG_RESULT([found])
	
		AC_MSG_CHECKING([for Cube version])
		CUBE_STRING=`${CubeDir}/bin/cube-config --version | cut -f 1 -d\ `
		CUBE_MAJOR_VERSION=`${CubeDir}/bin/cube-config --version | cut -f 2 -d\ | cut -f 1 -d.`
		CUBE_MINOR_VERSION=`${CubeDir}/bin/cube-config --version | cut -f 2 -d\ | cut -f 2 -d.`
		if test "${CUBE_STRING}" != "" -a "${CUBE_MAJOR_VERSION}" != "" -a "${CUBE_MINOR_VERSION}" != "" ; then
			AC_MSG_RESULT([${CUBE_MAJOR_VERSION}.${CUBE_MINOR_VERSION}])
			if test ${CUBE_MAJOR_VERSION} -eq 4 ; then
				if test ${CUBE_MINOR_VERSION} -lt 3 ; then
					AC_MSG_ERROR([Cube 4.3 or higher is required!])
				fi
			fi
		else
			AC_MSG_ERROR([Cannot determine Cube version through cube-config --version])
		fi

		AC_MSG_CHECKING([for Cube necessary header files)])
		if test ! -f ${CubeDir}/include/cube/Cube.h ; then
			AC_MSG_ERROR([Cannot find cube header files of the libtools! Make sure that --with-cube is pointing to the correct place.])
		fi
		AC_MSG_RESULT([found in ${CubeDir}/include])

		AC_MSG_CHECKING([for Cube necessary library files)])
		if test -f ${CubeDir}/lib/libcube4.a ; then
			CUBE_DIR_LIB=${CubeDir}/lib
		elif test -f ${CubeDir}/lib64/libcube4.a ; then
			CUBE_DIR_LIB=${CubeDir}/lib64
		else
			AC_MSG_ERROR([Cannot find libcube4.a! Make sure that --with-cube is pointing to the correct place.])
		fi
		AC_MSG_RESULT([found in ${CUBE_DIR_LIB}])

		CUBE_DIR=${CubeDir}
		CUBE_INCLUDE_PATH=`${CubeDir}/bin/cube-config --cube-include-path`
		CUBE_LDFLAGS=`${CubeDir}/bin/cube-config --cube-ldflags`
		CUBE_GUI_INCLUDE_PATH=`${CubeDir}/bin/cube-config --gui-include-path`
		CUBE_GUI_LDFLAGS=`${CubeDir}/bin/cube-config --gui-ldflags`
		AC_SUBST(CUBE_DIR)
		AC_SUBST(CUBE_DIR_LIB)
		AC_SUBST(CUBE_INCLUDE_PATH)
		AC_SUBST(CUBE_GUI_INCLUDE_PATH)
		AC_SUBST(CUBE_LDFLAGS)
		AC_SUBST(CUBE_GUI_LDFLAGS)
		CUBE_AVAILABLE="yes"
		AC_DEFINE([HAVE_CUBE], 1, [Defined CUBE is installed and available to use])
	fi
	AM_CONDITIONAL(HAVE_CUBE, test "${CUBE_AVAILABLE}" = "yes")
])

