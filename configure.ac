#                                               -*- Autoconf -*-
# configure.ac for the trousers project
#

AC_INIT([trousers],[0.3.11.2],[trousers-tech@lists.sf.net])
AC_CONFIG_MACRO_DIR([m4])

TSS_SPEC_MAJOR=1
TSS_SPEC_MINOR=2
TSS_VER_MAJOR=0
TSS_VER_MINOR=3

# compute ${target}:
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([foreign 1.6 subdir-objects])
AM_MAINTAINER_MODE

# Debugging support
AC_ARG_ENABLE([debug],
    [AS_HELP_STRING([--enable-debug],[turn on all trousers debugging flags [default=off]])],
    [AC_MSG_RESULT([*** Enabling debugging at user request ***])],[])

# If the user has not set CFLAGS, do something appropriate
test_CFLAGS=${CFLAGS+set}
if test "x${test_CFLAGS}" != "xset"; then
	if test "x${enable_debug}" = "xyes"; then
		CFLAGS="-O0 -g -DTSS_DEBUG -Wreturn-type"
		CFLAGS_DEBUG_EXTRA="-DTCSD_SINGLE_THREAD_DEBUG"
	else
		CFLAGS="-O2"
	fi
else
	if test "x${enable_debug}" = "xyes"; then
		CFLAGS="${CFLAGS} -O0 -g -DTSS_DEBUG -Wreturn-type"
	fi
fi

# Arch specific stuff
case ${target} in
	*solaris*)
		CFLAGS="${CFLAGS} -DSOLARIS"
		;;
        *)
                ;;
esac

# Non-standard OpenSSL location
AC_MSG_CHECKING([Non-standard OpenSSL location])
AC_ARG_WITH([openssl],
	[AS_HELP_STRING([--with-openssl=PATH],
                        [Location of openssl libs/includes])],
	[OPENSSL_INCLUDE_DIR="${withval}/include"
	 OPENSSL_LIB_DIR="${withval}/lib"
	 if [[ ! -d ${OPENSSL_INCLUDE_DIR} -o ! -d ${OPENSSL_LIB_DIR} ]]; then
		AC_MSG_ERROR([${OPENSSL_INCLUDE_DIR} or ${OPENSSL_LIB_DIR} does NOT exist!])
	 else
		AC_MSG_RESULT([yes])
		CFLAGS="${CFLAGS} -L${OPENSSL_LIB_DIR} -I$OPENSSL_INCLUDE_DIR"
		AC_SUBST([OPENSSL_LIB_DIR])
	 fi],
	[AC_MSG_RESULT([no])
	 AC_SUBST([OPENSSL_LIB_DIR],[])])

# The tspi Makefile will look for trspi/crypto/@CRYPTO_PACKAGE@/crypto.c
# Future crypto packages can go in their own subdir of trspi/crypto
# and a check for them should be made here
AC_CHECK_LIB([crypto],[EVP_DigestUpdate],
		[CRYPTO_PACKAGE='openssl'],
		[AC_MSG_ERROR([openssl is currently the only supported crypto library for trousers. Please install openssl from http://www.openssl.org or the -devel package from your distro])
])
AC_SUBST([CRYPTO_PACKAGE])
AC_SUBST([CRYPTOLIB],[-lcrypto])

AC_ARG_ENABLE([gcov],
		[AS_HELP_STRING([--enable-gcov],[turn on gcov code coverage flags [default=off]])],
		[CFLAGS="${CFLAGS} -ftest-coverage -fprofile-arcs"
		 AC_MSG_RESULT([*** Enabling gcov at user request ***])],[])

# profiling support
AC_ARG_ENABLE([gprof],
		[AS_HELP_STRING([--enable-gprof],[enable profiling with gprof [default=off]])],
		[CFLAGS="${CFLAGS} -pg"
		 AC_MSG_RESULT([*** Enabling profiling at user request ***])],[])

SPEC_COMP=0
# strict spec compliance
AC_ARG_ENABLE([strict-spec-compliance],
		[AS_HELP_STRING([--enable-strict-spec-compliance],[build TrouSerS as strictly spec compliant [default=off]])],
		[CFLAGS="${CFLAGS} -DTSS_SPEC_COMPLIANCE"
		 SPEC_COMP=1
		 AC_MSG_RESULT([*** Enabling spec compliance at user request ***])],[])

# daa math lib: gmp or openssl (default openssl)
MATH_DEFINE=BI_OPENSSL
AC_ARG_WITH([gmp],
	    [AS_HELP_STRING([--with-gmp],[build TrouSerS with the GMP math lib (used in DAA)])],
		[AC_CHECK_LIB([gmp],[__gmp_rand],[],[
                   AC_CHECK_LIB([gmp],[main])
                ])
		AC_CHECK_HEADERS([gmp.h])
		MATH_DEFINE=BI_GMP
                #FIXME: this gets enabled even when "--without" is used:
		AC_MSG_RESULT([*** Enabling GMP lib at user request ***])])

case "${MATH_DEFINE}" in
BI_OPENSSL)
	AC_CHECK_HEADERS([ openssl/bn.h openssl/engine.h])
	;;
esac
CFLAGS="${CFLAGS} -D${MATH_DEFINE}"

GUI=openssl
AC_ARG_WITH([gui],
	    [AS_HELP_STRING([--with-gui],[type of gui popup (gtk/none) [default=gtk]])],
	    [GUI=${withval}],
	    [])

if test "x${GUI}" = "xgtk"; then
	# section imported from Glade compile
	pkg_modules="gtk+-2.0 >= 2.0.0"
	PKG_CHECK_MODULES([GTK],
		[${pkg_modules}],
		AM_CONDITIONAL([HAVE_GTK],[true]),
		[AM_CONDITIONAL([HAVE_GTK],[false])
		AC_MSG_ERROR([Please install the gtk2 package that comes with development headers in it for your distro, or select another gui option.])])
        AC_CHECK_HEADERS([gtk/gtk.h gdk/gdkkeysyms.h])
	AM_CONDITIONAL([OPENSSL_UI],[false])
	AC_SUBST([GTK_CFLAGS])
	AC_SUBST([GTK_LIBS])
elif test "x${GUI}" = "xopenssl"; then
	# We know we have OpenSSL
	AM_CONDITIONAL([OPENSSL_UI],[true])
	AM_CONDITIONAL([HAVE_GTK],[false])
elif test "x${GUI}" = "xnone"; then
	if test ${SPEC_COMP} -eq 1; then
		AC_MSG_ERROR([Popups must be enabled in strict spec compliance mode])
	fi
	AC_MSG_RESULT([*** Disabling GUI popups at user request ***])
	AC_MSG_RESULT([*** WARNING: This may break apps! ***])
	CFLAGS="${CFLAGS} -DTSS_NO_GUI"
	AM_CONDITIONAL([HAVE_GTK],[false])
	AM_CONDITIONAL([OPENSSL_UI],[false])
else
	AC_MSG_ERROR(["gtk", "openssl" and "none" are the only supported gui options for trousers])
fi

#
# The default port that the TCS daemon listens on
#
AC_SUBST([TCSD_DEFAULT_PORT],[30003])
#
# The RPC mechanism to build into both libtspi and the tcsd
#
# AC_SUBST([RPC],["soap"])
AC_SUBST([RPC],["tcstp"])

#
# API= The TSS API level to build by default.
#
# To build a 1.1 TSS, set API=1.1 (./configure --with-api=1.1)
# To build a 1.2 TSS, set API=1.2 (./configure --with-api=1.2)
#
# In order to build a custom TSS API, set API to the lowest API level that
# contains the APIs you need. For instance, if you need only APIs that are
# a subset of the TSS 1.1 API, set this to 1.1. If you need any of the 1.2
# APIs, you will need to set this to 1.2. Send mail to
# trousers-tech@lists.sf.net if you have questions.
#
API=1.2
AC_ARG_WITH([api],
	    [AS_HELP_STRING([--with-api],[Version of the TSS API to build [default=1.2]])],
	    [API=${withval}],
	    [])

if test "x${API}" != "x1.1" && test "x${API}" != "x1.2"; then
	AC_MSG_ERROR(["1.1" and "1.2" are the only supported API versions for trousers.
		      Custom API build options are available by editing 'configure.ac'.])
fi

if test "x${API}" = "x1.1" || test "x${API}" = "x1.2"; then
	# Tspi_Hash_GetHashValue,SetHashValue,UpdateHashValue
	AM_CONDITIONAL([TSS_BUILD_HASH],[true])
	# Tspi_{Get|Set}Attribdata,{Get|Set}AttribUint32
	AM_CONDITIONAL([TSS_BUILD_GETSET],[true])
	# Tspi_TPM_GetRandom,StirRandom
	AM_CONDITIONAL([TSS_BUILD_RANDOM],[true])
	# Tspi_GetCapability (for TSP and TCS capabilities)
	AM_CONDITIONAL([TSS_BUILD_CAPS],[true])
	# Tspi_TPM_GetCapability (for TPM chip capabilities)
	AM_CONDITIONAL([TSS_BUILD_CAPS_TPM],[true])
	# Tspi_GetPolicyObject, Tspi_Policy_SetSecret,FlushSecret,AssignToObject
	AM_CONDITIONAL([TSS_BUILD_POLICY],[true])
	# Tspi_TPM_DirWrite,DirRead
	AM_CONDITIONAL([TSS_BUILD_DIR],[true])
	# Tspi_TPM_GetEvent,GetEvents,GetEventLog
	AM_CONDITIONAL([TSS_BUILD_PCR_EVENTS],[true])
	# Tspi_Hash_Sign,VerifySignature
	AM_CONDITIONAL([TSS_BUILD_SIGN],[true])
	# Tspi_TPM_Quote
	AM_CONDITIONAL([TSS_BUILD_QUOTE],[true])
	# Tspi_PcrComposite_{Set|Get}PcrValue,SelectPcrIndex
	AM_CONDITIONAL([TSS_BUILD_PCR_COMP],[true])
	# Tspi_Data_Seal,Unseal
	AM_CONDITIONAL([TSS_BUILD_SEAL],[true])
	# Tspi_ChangeAuth,ChangeAuthAsym
	AM_CONDITIONAL([TSS_BUILD_CHANGEAUTH],[true])
	# Tspi_Data_Bind,Unbind
	AM_CONDITIONAL([TSS_BUILD_BIND],[true])
	# Tspi_TPM_TakeOwnership,ClearOwner (REQ: EK)
	AM_CONDITIONAL([TSS_BUILD_OWN],[true])
	# Tspi_TPM_CreateEndorsementKey,GetPubEndorsementKey
	AM_CONDITIONAL([TSS_BUILD_EK],[true])
	# Tspi_Context_RegisterKey,UnregisterKey,LoadKeyByUUID,GetKeyByUUID,GetKeyByPublicInfo,
	# GetRegisteredKeysByUUID
	AM_CONDITIONAL([TSS_BUILD_PS],[true])
	# Tspi_TPM_{Set|Get}Status
	AM_CONDITIONAL([TSS_BUILD_ADMIN],[true])
	# Tspi_TPM_CollateIdentityRequest,ActivateIdentity
	AM_CONDITIONAL([TSS_BUILD_AIK],[true])
	# Tspi_Key_CertifyKey
	AM_CONDITIONAL([TSS_BUILD_CERTIFY],[true])
	# Tspi_TPM_CreateMaintenanceArchive,KillMaintenanceFeature,LoadMaintenancePubKey,
	# CheckMaintenancePubKey
	AM_CONDITIONAL([TSS_BUILD_MAINT],[true])
	# Tspi_TPM_AuthorizeMigrationTicket,Key_CreateMigrationBlob,ConvertMigrationBlob
	AM_CONDITIONAL([TSS_BUILD_MIGRATION],[true])
	# Tspi_Context_LoadKeyByBlob,Key_LoadKey,UnloadKey,CreateKey,WrapKey,GetPubKey
	AM_CONDITIONAL([TSS_BUILD_KEY],[true])
	# Tspi_TPM_PcrExtend,PcrRead,PcrReset
	AM_CONDITIONAL([TSS_BUILD_PCR_EXTEND],[true])
	# Tspi_TPM_SelfTestFull,CertifySelfTest,GetTestResult
	AM_CONDITIONAL([TSS_BUILD_SELFTEST],[true])
fi

if test "x${API}" = "x1.2"; then
	AM_CONDITIONAL([TSS_BUILD_TSS12],[true])
	# Do NOT build DAA until the API is fixed - KEY
	AM_CONDITIONAL([TSS_BUILD_DAA],[false])
	AM_CONDITIONAL([TSS_BUILD_PCR_COMP12],[true])
	AM_CONDITIONAL([TSS_BUILD_COUNTER],[true])
	AM_CONDITIONAL([TSS_BUILD_TICK],[true])
	AM_CONDITIONAL([TSS_BUILD_TRANSPORT],[true])
	AM_CONDITIONAL([TSS_BUILD_ASN1],[true])
	AM_CONDITIONAL([TSS_BUILD_NV],[true])
	AM_CONDITIONAL([TSS_BUILD_AUDIT],[true])
	AM_CONDITIONAL([TSS_BUILD_SEALX],[true])
	AM_CONDITIONAL([TSS_BUILD_DELEGATION],[true])
	AM_CONDITIONAL([TSS_BUILD_QUOTE2],[true])
	# CMK depends on MIGRATION
	AM_CONDITIONAL([TSS_BUILD_CMK],[true])
else
	AM_CONDITIONAL([TSS_BUILD_TSS12],[false])
	AM_CONDITIONAL([TSS_BUILD_DAA],[false])
	AM_CONDITIONAL([TSS_BUILD_PCR_COMP12],[false])
	AM_CONDITIONAL([TSS_BUILD_COUNTER],[false])
	AM_CONDITIONAL([TSS_BUILD_TICK],[false])
	AM_CONDITIONAL([TSS_BUILD_TRANSPORT],[false])
	AM_CONDITIONAL([TSS_BUILD_ASN1],[false])
	AM_CONDITIONAL([TSS_BUILD_NV],[false])
	AM_CONDITIONAL([TSS_BUILD_AUDIT],[false])
	AM_CONDITIONAL([TSS_BUILD_SEALX],[false])
	AM_CONDITIONAL([TSS_BUILD_DELEGATION],[false])
	AM_CONDITIONAL([TSS_BUILD_QUOTE2],[false])
	AM_CONDITIONAL([TSS_BUILD_CMK],[false])
fi

#
# There is no need to edit anything below, these conditionals control
# the building of files that support the files above, which all contain
# TSS APIs"
#
AM_CONDITIONAL([TSS_BUILD_NV_LIST],[test -z ${TSS_BUILD_NV_TRUE}])
AM_CONDITIONAL([TSS_BUILD_NV_SUPPORT],[test -z ${TSS_BUILD_NV_TRUE}])
AM_CONDITIONAL([TSS_BUILD_GET_FLAGS],[test -z ${TSS_BUILD_ADMIN_TRUE} || \
				      test -z ${TSS_BUILD_CAPS_TPM_TRUE}])
AM_CONDITIONAL([TSS_BUILD_PCRS_LIST],
               [test -z ${TSS_BUILD_SEAL_TRUE} || \
		test -z ${TSS_BUILD_QUOTE_TRUE} || \
		test -z ${TSS_BUILD_PCRS_TRUE} || \
		test -z ${TSS_BUILD_PCR_COMP_TRUE} || \
		test -z ${TSS_BUILD_SEALX_TRUE}])
AM_CONDITIONAL([TSS_BUILD_HASH_LIST],[test -z ${TSS_BUILD_SIGN_TRUE} || \
                                      test -z ${TSS_BUILD_HASH_TRUE}])
AM_CONDITIONAL([TSS_BUILD_ENCDATA_LIST],
               [test -z ${TSS_BUILD_SEAL_TRUE} || \
		test -z ${TSS_BUILD_CHANGEAUTH_TRUE} || \
		test -z ${TSS_BUILD_BIND_TRUE} || \
		test -z ${TSS_BUILD_SEALX_TRUE}])
AM_CONDITIONAL([TSS_BUILD_RSAKEY_LIST],
               [test -z ${TSS_BUILD_ADMIN_TRUE} || \
		test -z ${TSS_BUILD_EK_TRUE} || \
		test -z ${TSS_BUILD_MIGRATION_TRUE} || \
		test -z ${TSS_BUILD_MAINT_TRUE} || \
		test -z ${TSS_BUILD_CERTIFY_TRUE} || \
		test -z ${TSS_BUILD_AIK_TRUE} || \
		test -z ${TSS_BUILD_QUOTE_TRUE} || \
		test -z ${TSS_BUILD_BIND_TRUE} || \
		test -z ${TSS_BUILD_CHANGEAUTH_TRUE} || \
		test -z ${TSS_BUILD_OWN_TRUE} || \
		test -z ${TSS_BUILD_SIGN_TRUE} || \
		test -z ${TSS_BUILD_PS_TRUE} || \
		test -z ${TSS_BUILD_SEAL_TRUE} || \
		test -z ${TSS_BUILD_DAA_TRUE} || \
		test -z ${TSS_BUILD_KEY_TRUE} || \
		test -z ${TSS_BUILD_SEALX_TRUE}])
AM_CONDITIONAL([TSS_BUILD_AUTH],[test -z ${TSS_BUILD_HASH_TRUE} || \
			         test -z ${TSS_BUILD_CAPS_TRUE} || \
			         test -z ${TSS_BUILD_CAPS_TPM_TRUE} || \
			         test -z ${TSS_BUILD_POLICY_TRUE} || \
			         test -z ${TSS_BUILD_DIR_TRUE} || \
			         test -z ${TSS_BUILD_PCR_EVENTS_TRUE} || \
			         test -z ${TSS_BUILD_SIGN_TRUE} || \
			         test -z ${TSS_BUILD_QUOTE_TRUE} || \
			         test -z ${TSS_BUILD_PCR_COMP_TRUE} || \
			         test -z ${TSS_BUILD_SEAL_TRUE} || \
			         test -z ${TSS_BUILD_SEALX_TRUE} || \
			         test -z ${TSS_BUILD_CHANGEAUTH_TRUE} || \
			         test -z ${TSS_BUILD_BIND_TRUE} || \
			         test -z ${TSS_BUILD_OWN_TRUE} || \
			         test -z ${TSS_BUILD_PS_TRUE} || \
			         test -z ${TSS_BUILD_ADMIN_TRUE} || \
			         test -z ${TSS_BUILD_AIK_TRUE} || \
			         test -z ${TSS_BUILD_EK_TRUE} || \
			         test -z ${TSS_BUILD_CERTIFY_TRUE} || \
			         test -z ${TSS_BUILD_MAINT_TRUE} || \
			         test -z ${TSS_BUILD_MIGRATION_TRUE} || \
			         test -z ${TSS_BUILD_KEY_TRUE} || \
			         test -z ${TSS_BUILD_PCR_EXTEND_TRUE} || \
			         test -z ${TSS_BUILD_SELFTEST_TRUE} || \
			         test -z ${TSS_BUILD_DAA_TRUE}])
AM_CONDITIONAL([TSS_BUILD_ASYM_CRYPTO],
               [test -z ${TSS_BUILD_AIK_TRUE} || \
		test -z ${TSS_BUILD_CERTIFY_TRUE} || \
		test -z ${TSS_BUILD_QUOTE_TRUE} || \
		test -z ${TSS_BUILD_EK_TRUE} || \
		test -z ${TSS_BUILD_CHANGEAUTH_TRUE} || \
		test -z ${TSS_BUILD_BIND_TRUE} || \
		test -z ${TSS_BUILD_OWN_TRUE} || \
		test -z ${TSS_BUILD_SELFTEST_TRUE} || \
		test -z ${TSS_BUILD_SIGN_TRUE} || \
		test -z ${TSS_BUILD_KEY_TRUE} || \
		test -z ${TSS_BUILD_DAA_TRUE}])
AM_CONDITIONAL([TSS_BUILD_SYM_CRYPTO],[test -z ${TSS_BUILD_AIK_TRUE} || \
				       test -z ${TSS_BUILD_TRANSPORT_TRUE}])

# begin Glade section

GETTEXT_PACKAGE=trousers
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["${GETTEXT_PACKAGE}"])

dnl# Add the languages which your application supports here.
ALL_LINGUAS=""
AM_GLIB_GNU_GETTEXT

# end Glade section

# pthreads:
AC_CHECK_HEADER([pthread.h],
                [AC_DEFINE([HAVE_PTHREAD_H],[1],[pthread header])
])
AC_CHECK_LIB([pthread],[pthread_mutex_init])
AC_CHECK_TYPES([pthread_t])

# Checks for programs.
AC_DISABLE_STATIC
AC_PROG_CC
AC_PROG_CXX
AC_PROG_MKDIR_P
AC_PROG_RANLIB
LT_INIT([disable-static])

# Checks for typedefs, structures, and compiler characteristics.
dnl#FIXME: use this with autoheader:
AC_C_BIGENDIAN([AC_DEFINE([_BIG_ENDIAN],[1],[big-endian host])])
AC_CHECK_DECL([htole32],
              [AC_DEFINE([HTOLE_DEFINED],[1],
                         [htole32 function is available])
])
dnl#FIXME: make this ifdef-ed when we transition to autoheader:
AC_C_CONST([AC_DEFINE_UNQUOTED([BSD_CONST],["const"],
                               [define for XPG6-compliance for iconv])
])
AC_C_INLINE
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_CHECK_SIZEOF([long int])
AC_CHECK_ALIGNOF([long int])

# Checks for header files.
AC_CHECK_HEADER([sys/byteorder.h],
                [AC_DEFINE([HAVE_BYTEORDER_H],[1],[sys/byteorder.h header])
])
AC_HEADER_TIME
AC_CHECK_HEADERS([arpa/inet.h err.h fcntl.h langinfo.h limits.h \
                  malloc.h malloc/malloc.h netdb.h netinet/in.h \
                  sys/file.h sys/ioctl.h sys/param.h sys/socket.h \
                  sys/time.h sys/sysctl.h syslog.h utmp.h utmpx.h \
                  wchar.h])

# Checks for library functions.
AC_CHECK_FUNC([daemon],
              [AC_DEFINE([HAVE_DAEMON],[1],[daemon function is available])
])
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([endpwent err ftruncate gethostbyaddr gethostbyname \
                inet_ntoa memset mkdir munmap nl_langinfo setenv \
                socket strcasecmp strdup strerror strncasecmp sysctl])
 
# Final CFLAGS additions:
if test "x${GCC}" = "xyes"; then
	CFLAGS="${CFLAGS} -W -Wall -Wno-unused-parameter -Wsign-compare"
fi

CFLAGS="${CFLAGS} -I../include \
-DTCSD_DEFAULT_PORT=${TCSD_DEFAULT_PORT} \
-DTSS_VER_MAJOR=${TSS_VER_MAJOR} \
-DTSS_VER_MINOR=${TSS_VER_MINOR} \
-DTSS_SPEC_MAJOR=${TSS_SPEC_MAJOR} \
-DTSS_SPEC_MINOR=${TSS_SPEC_MINOR}"
CFLAGS_HARSH="${CFLAGS} -I../include -std=c99 -pedantic -W -Wall"
KERNEL_VERSION=`uname -r`
AC_SUBST([CFLAGS])

# When we build the rpms, prefix will be /usr. This will do some things
# that make sense, like put our sbin stuff in /usr/sbin and our library
# in /usr/lib. It will do some other things that do NOT make sense like
# put our config file in /usr/etc. So, I will just hack it here. If the
# --prefix option is NOT specified during configure, let it all go to
# /usr/local, even /usr/local/etc. :-P
if test x"${prefix}" = x"/usr"; then
	sysconfdir="/etc"
	localstatedir="/var"
	mandir="/usr/share/man"
elif test x"${prefix}" = x"NONE"; then
	localstatedir="/usr/local/var"
fi

AC_CONFIG_FILES([dist/tcsd.conf \
	  dist/fedora/trousers.spec \
	  dist/trousers.spec \
	  Makefile \
	  src/Makefile \
	  src/include/Makefile \
	  src/tcs/Makefile \
	  src/tddl/Makefile \
	  src/tspi/Makefile \
	  src/trspi/Makefile \
	  src/tcsd/Makefile \
	  man/man8/tcsd.8 \
	  man/man5/tcsd.conf.5 \
	  dist/Makefile \
	  man/Makefile \
	  man/man3/Makefile \
	  man/man5/Makefile \
	  man/man8/Makefile])
AC_OUTPUT

echo "CFLAGS=${CFLAGS}"

