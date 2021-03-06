/* File modified during XXI century in process of creating UnNetHackPlus */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef CONFIG1_H
#define CONFIG1_H

/*
 * MS DOS - compilers
 *
 * Microsoft C auto-defines MSDOS,
 * Borland C   auto-defines __MSDOS__,
 * DJGPP       auto-defines MSDOS.
 */

/* #define MSDOS */	/* use if not defined by compiler or cases below */

#ifdef __MSDOS__	/* for Borland C */
# ifndef MSDOS
# define MSDOS
# endif
#endif

#ifdef __TURBOC__
# define __MSC		/* increase Borland C compatibility in libraries */
#endif

#ifdef MSDOS
# undef UNIX
#endif

/*
 * Mac Stuff.
 */
#ifdef macintosh	/*	Auto-defined symbol for MPW compilers (sc and mrc) */
# define MAC
#endif

#ifdef THINK_C		/* Think C auto-defined symbol */
# define MAC
# define NEED_VARARGS
#endif

#ifdef __MWERKS__	/* defined by Metrowerks' Codewarrior compiler */
# ifndef __BEOS__	/* BeOS */
#  define MAC
# endif
# define NEED_VARARGS
# define USE_STDARG
#endif

#if defined(MAC) || defined(__BEOS__)
# define DLB
# undef UNIX
#endif

#ifdef __BEOS__
# define NEED_VARARGS
#endif


/*
 * Windows NT Autodetection
 */
#ifdef _WIN32_WCE
#define WIN_CE
# ifndef WIN32
# define WIN32
# endif
#endif

#ifdef WIN32
# undef UNIX
# undef MSDOS
# define NHSTDC
# define USE_STDARG
# define NEED_VARARGS

#ifndef WIN_CE
# define STRNCMPI
# define STRCMPI
#endif

#endif


#if defined(__linux__) && defined(__GNUC__) && !defined(_GNU_SOURCE)
/* ensure _GNU_SOURCE is defined before including any system headers */
# define _GNU_SOURCE
#endif

#ifdef VMS	/* really old compilers need special handling, detected here */
# undef UNIX
# ifdef __DECC
#  ifndef __DECC_VER	/* buggy early versions want widened prototypes */
#   define NOTSTDC	/* except when typedefs are involved		*/
#   define USE_VARARGS
#  else
#   define NHSTDC
#   define USE_STDARG
#   define POSIX_TYPES
#   define _DECC_V4_SOURCE	/* avoid some incompatible V5.x changes */
#  endif
#  undef __HIDE_FORBIDDEN_NAMES /* need non-ANSI library support functions */
# else
#  ifdef VAXC	/* vaxc v2.2 or older is not supported */
#   if defined(USE_PROTOTYPES) /* this breaks 2.2 (*forces* use of ANCIENT)*/
#    define __STDC__ 0 /* vaxc is not yet ANSI compliant, but close enough */
#    define signed	/* well, almost close enough */
#include <stddef.h>
#   endif
#   define USE_STDARG
#  endif /*VAXC*/
# endif /*__DECC*/
# ifdef VERYOLD_VMS	/* v4.5 or earlier; no longer available for testing */
#  define USE_OLDARGS	/* <varargs.h> is there, vprintf & vsprintf aren't */
#  ifdef USE_VARARGS
#   undef USE_VARARGS
#  endif
#  ifdef USE_STDARG
#   undef USE_STDARG
#  endif
# endif
#endif /*VMS*/

#endif	/* CONFIG1_H */
