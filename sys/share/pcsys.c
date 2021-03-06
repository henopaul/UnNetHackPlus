/* File modified during XXI century in process of creating UnNetHackPlus */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *  System related functions for MSDOS, OS/2, and Windows NT
 */

#define NEED_VARARGS
#include "hack.h"
#include "wintty.h"

#include <ctype.h>
#include <fcntl.h>
#if !defined(MSDOS) && !defined(WIN_CE) 	/* already done */
#include <process.h>
#endif
#ifdef __GO32__
#define P_WAIT		0
#define P_NOWAIT	1
#endif
#if defined(MSDOS) && !defined(__GO32__)
#define findfirst findfirst_file
#define findnext findnext_file
#define filesize filesize_nh
#endif


#if defined(MICRO) || defined(WIN32) || defined(OS2)
void FDECL(nethack_exit,(int));
#else
#define nethack_exit exit
#endif
static void NDECL(msexit);


#ifdef MOVERLAY
extern void __far __cdecl _movepause( void );
extern void __far __cdecl _moveresume( void );
extern unsigned short __far __cdecl _movefpause;
extern unsigned short __far __cdecl _movefpaused;
#define     __MOVE_PAUSE_DISK	  2   /* Represents the executable file */
#define     __MOVE_PAUSE_CACHE	  4   /* Represents the cache memory */
#endif /* MOVERLAY */

#ifdef MFLOPPY
STATIC_DCL boolean NDECL(record_exists);
STATIC_DCL boolean NDECL(comspec_exists);
#endif

#ifdef WIN32CON
extern int GUILaunched;    /* from nttty.c */
#endif

#if defined(MICRO) || defined(WIN32)

void
flushout()
{
	(void) fflush(stdout);
	return;
}

static const char *COMSPEC = "COMSPEC";

#define getcomspec() nh_getenv(COMSPEC)

# ifdef MFLOPPY

void
eraseall(path, files)
const char *path, *files;
{
	char buf[PATHLEN];
	char *foundfile;

	foundfile = foundfile_buffer();
	Sprintf(buf, "%s%s", path, files);
	if (findfirst(buf))
	    do {
	       Sprintf(buf, "%s%s", path, foundfile);
		(void) unlink(buf);
	    } while (findnext());
	return;
}

/*
 * Rewritten for version 3.3 to be faster
 */
void
copybones(mode)
int mode;
{
	char from[PATHLEN], to[PATHLEN], last[13];
	char *frompath, *topath;
	char *foundfile;
	int status;
	char copy[8], *comspec;

	if (!ramdisk)
		return;

	/* Find the name of the last file to be transferred
	 */
	frompath = (mode != TOPERM) ? permbones : levels;
	foundfile = foundfile_buffer();
	last[0] = '\0';
	Sprintf(from, "%s%s", frompath, allbones);
	topath = (mode == TOPERM) ? permbones : levels;
	if (findfirst(from))
		do {
			Strcpy(last, foundfile);
		} while (findnext());
	if (last[0]) {
		Sprintf(copy, "%cC copy",switchar());

		/* Remove any bones files in `to' directory.
		 */
		eraseall(topath, allbones);

		/* Copy `from' to `to' */
		Sprintf(to, "%s%s", topath, allbones);
		comspec = getcomspec();
		status =spawnl(P_WAIT, comspec, comspec, copy, from,
			to, "> nul", (char *)0);
	} else
		return;

	/* See if the last file got there.  If so, remove the ramdisk bones
	 * files.
	 */
	Sprintf(to, "%s%s", topath, last);
	if (findfirst(to)) {
		if (mode == TOPERM)
			eraseall(frompath, allbones);
		return;
	}

	/* Last file didn't get there.
	 */
	Sprintf(to, "%s%s", topath, allbones);
	msmsg("Can't copy \"%s\" to \"%s\" -- ", from, to);
	if (status < 0)
	    msmsg("can't spawn \"%s\"!", comspec);
	else
	    msmsg((freediskspace(topath) < filesize(from)) ?
	    "insufficient disk space." : "bad path(s)?");
	if (mode == TOPERM) {
		msmsg("Bones will be left in \"%s\"\n",
			*levels ? levels : hackdir);
	} else {
		/* Remove all bones files on the RAMdisk */
		eraseall(levels, allbones);
		playwoRAMdisk();
	}
	return;
}

void
playwoRAMdisk()
{
	int c;

	msmsg("Do you wish to play without a RAMdisk? [yn] (n)");

	/* Set ramdisk false *before* exit-ing (because msexit calls
	 * copybones)
	 */
	ramdisk = FALSE;
	c = tgetch(); if (c == 'Y') c = 'y';
	if (c != 'y') {
		setty(get_goodbye_message());
		settty("\n");
		nethack_exit(EXIT_SUCCESS);
	}
	set_lock_and_bones();
	return;
}

int
saveDiskPrompt(start)
int start;
{
	char buf[BUFSIZ], *bp;
	char qbuf[QBUFSZ];

	int fd;

	if (flags.asksavedisk) {
		/* Don't prompt if you can find the save file */
		if ((fd = open_savefile()) >= 0) {
			(void) close(fd);
			return 1;
		}
		clear_nhwindow(WIN_MESSAGE);
		pline("If save file is on a save disk, insert that disk now.");
		mark_synch();
		Sprintf(qbuf,"File name (default \"%s\"%s) ?", SAVEF,
			start ? "" : ", <Esc> cancels save");
		getlin(qbuf, buf);
		clear_nhwindow(WIN_MESSAGE);
		if (!start && *buf == '\033')
			return 0;

		/* Strip any whitespace. Also, if nothing was entered except
		 * whitespace, do not change the value of SAVEF.
		 */
		for (bp = buf; *bp; bp++)
			if (!isspace(*bp)) {
				strncpy(SAVEF, bp, PATHLEN);
				break;
			}
	}
	return 1;
}

/* Return 1 if the record file was found */
STATIC_OVL boolean
record_exists()
{
	FILE *fp;

	fp = fopen_datafile(RECORD, "r", TRUE);
	if (fp) {
		fclose(fp);
		return TRUE;
	}
	return FALSE;
}
#endif /* MFLOPPY */

#ifdef MFLOPPY
/* Return 1 if the comspec was found */
STATIC_OVL boolean
comspec_exists()
{
	int fd;
	char *comspec;

	if ((comspec = getcomspec()))
		if ((fd = open(comspec, O_RDONLY)) >= 0) {
			(void) close(fd);
			return TRUE;
		}
	return FALSE;
}
#endif /* MFLOPPY */


# ifdef MFLOPPY
/* Prompt for game disk, then check for record file.
 */
void
gameDiskPrompt()
{
	if (flags.asksavedisk) {
		if (record_exists() && comspec_exists())
			return;
		(void) putchar('\n');
		getreturn("when the game disk has been inserted");
	}
	if (comspec_exists() && record_exists())
		return;

	if (!comspec_exists())
		msmsg("\n\nWARNING: can't find command processor \"%s\"!\n", getcomspec());
	if (!record_exists())
		msmsg("\n\nWARNING: can't find record file \"%s\"!\n", RECORD);
	msmsg("If the game disk is not in, insert it now.\n");
	getreturn("to continue");
	return;
}
# endif /* MFLOPPY */
#endif /* MICRO */

/*
 * Add a backslash to any name not ending in /, \ or :	 There must
 * be room for the \
 */
void
append_slash(name)
char *name;
{
	char *ptr;

	if (!*name)
		return;
	ptr = name + (strlen(name) - 1);
	if (*ptr != '\\' && *ptr != '/' && *ptr != ':') {
		*++ptr = '\\';
		*++ptr = '\0';
	}
	return;
}

#ifdef WIN32
boolean getreturn_enabled;
#endif

void
getreturn(str)
const char *str;
{
#ifdef WIN32
	if (!getreturn_enabled) return;
#endif
	msmsg("Hit <Enter> %s.", str);
	while (Getchar() != '\n') ;
	return;
}

#ifndef WIN32CON
void
msmsg VA_DECL(const char *, fmt)
	VA_START(fmt);
	VA_INIT(fmt, const char *);
# if defined(MSDOS) && defined(NO_TERMS)
	if (iflags.grmode)
		gr_finish();
# endif
	Vprintf(fmt, VA_ARGS);
	flushout();
	VA_END();
	return;
}
#endif

/*
 * Follow the PATH, trying to fopen the file.
 */
#define PATHSEP ';'

FILE *
fopenp(name, mode)
const char *name, *mode;
{
	char buf[BUFSIZ], *bp, *pp, lastch = 0;
	FILE *fp;

	/* Try the default directory first.  Then look along PATH.
	 */
	(void) strncpy(buf, name, BUFSIZ - 1);
	buf[BUFSIZ-1] = '\0';
	if ((fp = fopen(buf, mode)))
		return fp;
	else {
		int ccnt = 0;
		pp = getenv("PATH");
		while (pp && *pp) {
			bp = buf;
			while (*pp && *pp != PATHSEP) {
				lastch = *bp++ = *pp++;
				ccnt++;
			}
			if (lastch != '\\' && lastch != '/') {
				*bp++ = '\\';
				ccnt++;
			}
			(void) strncpy(bp, name, (BUFSIZ - ccnt) - 2);
			bp[BUFSIZ - ccnt - 1] = '\0';
			if ((fp = fopen(buf, mode)))
				return fp;
			if (*pp)
				pp++;
		}
	}
#ifdef OS2_CODEVIEW /* one more try for hackdir */
	(void) strncpy(buf, hackdir, BUFSZ);
	buf[BUFSZ-1] = '\0';
	if ((strlen(name) + 1 + strlen(buf)) < BUFSZ - 1) {
		append_slash(buf);
		Strcat(buf,name);
	} else 
		impossible("fopenp() buffer too small for complete filename!");
	if(fp = fopen(buf,mode))
		return fp;
#endif
	return (FILE *)0;
}

#if defined(MICRO) || defined(WIN32) || defined(OS2)
void nethack_exit(code)
int code;
{
	msexit();
	exit(code);
}

static void msexit()
{
#ifdef CHDIR
	extern char orgdir[];
#endif

	flushout();
#ifndef WIN32
	enable_ctrlP(); 	/* in case this wasn't done */
#endif
#ifdef MFLOPPY
	if (ramdisk) copybones(TOPERM);
#endif
#if defined(CHDIR) && !defined(NOCWD_ASSUMPTIONS)
	chdir(orgdir);		/* chdir, not chdirx */
	chdrive(orgdir);
#endif
#ifdef WIN32CON
	/* Only if we started from the GUI, not the command prompt,
	 * we need to get one last return, so the score board does
	 * not vanish instantly after being created.
	 * GUILaunched is defined and set in nttty.c.
	 */
	synch_cursor();
	if (GUILaunched) getreturn("to end");
	synch_cursor();
#endif
	return;
}
#endif /* MICRO || WIN32 || OS2 */
