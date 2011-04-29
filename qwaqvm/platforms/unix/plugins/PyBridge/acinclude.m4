
# Require python 2.4
if test -d /usr/lib/python2.4 ; then

#**	And require a local build of python source -
#	This file, as well as the include rules and stub headers
# 	will need to be changed if the python source build 
#**	is not in /home/qwaq/python2.4-2.4.4

	if test -d /usr/include/python2.4 -a -x /usr/lib/libpython2.4.so ; then
		AC_PLUGIN_USE_LIB(python2.4)
		py_libpath=
		py_includes=/usr/include/python2.4
	elif test -d /home/qwaq/python2.4-2.4.4 ; then
		AC_PLUGIN_USE_LIB(python2.4)
		py_libpath=-L/home/qwaq/python2.4-2.4.4
		py_includes=/home/qwaq/python2.4-2.4.4/Include
	else
		AC_PLUGIN_DISABLE
	fi
else
	AC_PLUGIN_DISABLE
fi

PYLIBPATH=${py_libpath}
PYINCLUDES=${py_includes}

AC_SUBST(PYLIBPATH)
AC_SUBST(PYINCLUDES)
