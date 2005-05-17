/*
 * main.c -- extract files from microsoft TNEF format
 *
 * Copyright (C)1999-2003 Mark Simpson <damned@world.std.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can either send email to this
 * program's maintainer or write to: The Free Software Foundation,
 * Inc.; 59 Temple Place, Suite 330; Boston, MA 02111-1307, USA.
 *
 * Commentary:
 * 	main function and command line parsing.
 */
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <stdio.h>

#include <ctype.h>

#if STDC_HEADERS
#  include <stdlib.h>
#  include <string.h>
#else
extern int strcmp (consr char *, const char *);
extern void abort (void);
extern void exit (int);
#endif

#ifndef _
/* This is for other GNU distributions with internationalized messages.
   When compiling libc, the _ macro is predefined.  */
# ifdef HAVE_LIBINTL_H
#  include <libintl.h>
#  define _(msgid)	gettext (msgid)
# else
#  define _(msgid)	(msgid)
# endif
#endif

#if HAVE_GETOPT_LONG
#  include <getopt.h>
#else
#  include "getopt_long.h"
#endif /* HAVE_GETOPT_LONG */

#include "alloc.h"

#include "tnef.h"

/* COPYRIGHTS & NO_WARRANTY -- defined to make code below a little nicer to
   read */
static const char* COPYRIGHTS = \
"Copyright (C) 1999-2005 by Mark Simpson\n"
"Copyright (C) 1997 by Thomas Boll (original code)";
static const char* NO_WARRANTY = \
"%s comes with ABSOLUTELY NO WARRANTY.\n"
"You may redistribute copies of %s under the terms of the GNU General\n"
"Public License.  For more information about these matters, see the file\n"
"named COPYING.";
static const char* USAGE = \
"-f FILE,--file=FILE     \tuse FILE as input ('-' == stdin)\n"
"-C DIR, --directory=DIR \tunpack files into DIR\n"
"-x SIZE --maxsize=SIZE  \tlimit maximum size of extracted archive (bytes)\n"
"-t,     --list          \tlist files, do not extract\n"
"-w,     --interactive   \task for confirmation for every action\n"
"        --confirmation  \tsame as -w\n"
"        --overwrite     \tOverwrite existing files\n"
"        --number-backups\tInstead of overwriting file FOO,\n"
"                        \t  create FOO.n instead\n"
"        --use-paths     \tUse pathnames for files if found in the TNEF\n" 
"                        \t  file (for security reasons paths to included\n"
"                        \t  files are ignored by default)\n"
"        --save-rtf[=FILE]\t[DEPRECATED] Save the RTF message body to a file\n"
"        --save-body[=FILE]\tSave the message body to a file\n"
"-h,     --help          \tshow this message\n"
"-V,     --version       \tdisplay version and copyright\n"
"-v,     --verbose       \tproduce verbose output\n"
"        --debug     	 \tproduce a lot of output\n"
"\n"
"\nIf FILE is not specified standard input is used\n"
"\nReport bugs to <%s>\n";


static void
usage (char* prog)
{
    fprintf (stdout, "%s: [options] [FILE]\n", prog);
    fprintf (stdout, USAGE, PACKAGE_BUGREPORT);
}


static char*
validate_body_pref (char *optarg)
{
    int i = 0;
    char *pref;
    char *p;

    if (optarg == NULL)
    {
	fprintf (stderr, "--body-pref cannot be null.\n");
	abort();
    }
    if (strlen(optarg) == 0 || strlen(optarg) > 3)
    {
	fprintf (stderr, "'%s' is an invalid setting for --body-pref", optarg);
	abort();
    }
    
    pref = strdup (optarg);
    p = pref;

    /* shift to all lower case */
    while ((*p = tolower(*p))) p++;

    /* 'all' is a special setting, do not validate */
    if (strcmp (pref, "all") != 0)
    {
	i = 0;
	while (pref[i])
	{
	    if (pref[i] != 'r'
		&& pref[i] != 'h'
		&& pref[i] != 't')
	    {
		fprintf (stderr, 
			 "--body-pref setting can only contain R, H or T.\n");
		abort();
	    }
	    i++;
	}
    }
    return pref;
}


static void
parse_cmdline (int argc, char **argv,
               char **in_file,
               char **out_dir,
	       char **body_file,
	       char **body_pref,
               size_t *max_size,
               int *flags)
{
    int i = 0;
    int option_index = 0;
    static struct option long_options[] = 
    { 
        {"confirmation", no_argument, 0, 'w' },
        {"debug", no_argument, 0, 0},
        {"directory", required_argument, 0, 'C' },
        {"file", required_argument, 0, 'f' },
        {"help", no_argument, 0, 'h'},
        {"interactive", no_argument, 0, 'w' },
        {"list", no_argument, 0, 't'}, /* for now same as -n */
        {"maxsize", required_argument, 0, 'x' },
        {"number-backups", no_argument, 0, 0 },
        {"overwrite", no_argument, 0, 0 },
        {"use-paths", no_argument, 0, 0},
	{"save-rtf", optional_argument, 0, 0 },
	{"save-body", optional_argument, 0, 0 },
	{"body-pref", required_argument, 0, 0 },
        {"verbose", no_argument, 0, 'v'},
        {"version", no_argument, 0, 'V'},
        { 0, 0, 0, 0 }
    };

    /* default values */
    (*body_pref) = strdup("rht");

    while ((i = getopt_long (argc, argv, "f:C:d:x:vVwht",
                             long_options, &option_index)) != -1)
    {
        switch (i) 
        {
        case 0:               /* long options with no val field */
            if (strcmp (long_options[option_index].name,
                        "debug") == 0)
            {
                *flags |= DBG_OUT;
            }
            else if (strcmp (long_options[option_index].name,
                             "use-paths") == 0)
            {
                *flags |= PATHS;
            }
            else if (strcmp (long_options[option_index].name,
                             "overwrite") == 0)
            {
                *flags |= OVERWRITE;
            }
            else if (strcmp (long_options[option_index].name,
                             "number-backups") == 0)
            {
                *flags |= NUMBERED;
            }
	    else if (strcmp (long_options[option_index].name,
			     "save-rtf") == 0)
	    {
		fprintf (stderr, "--save-rtf is a deprecated feature."
			 "  Use --save-body[=FILE] and "
			 "--body-pref=r instead.\n");
		*flags |= SAVEBODY;
		(*body_file) = strdup(((optarg) ? optarg : "message"));
		(*body_pref) = strdup("r");
	    }
	    else if (strcmp (long_options[option_index].name,
			     "save-body") == 0)
	    {
		*flags |= SAVEBODY;
		(*body_file) = strdup(((optarg) ? optarg : "message"));
	    }
	    else if (strcmp (long_options[option_index].name,
			     "body-pref") == 0)
	    {
		(*body_pref) = validate_body_pref (optarg);
	    }
            else
            {
                abort ();       /* impossible! */
            }
            break;

        case 'V':
            fprintf (stderr, "%s\n", PACKAGE_STRING);
            fprintf (stderr, COPYRIGHTS);
            fprintf (stderr, "\n");
            fprintf (stderr, NO_WARRANTY, PACKAGE_NAME, PACKAGE_NAME);
            fprintf (stderr, "\n");
            exit (0);
            break;

        case 'v': 
            *flags |= VERBOSE;
            break;

        case 'f':
            if (strcmp (optarg, "-") == 0) (*in_file) = NULL;
            else (*in_file) = optarg;
            break;
            
        case 'C':
        case 'd':
            (*out_dir) = optarg;
            break;

        case 'x':
            {
                char *end_ptr = NULL;
                (*max_size) = strtoul (optarg, &end_ptr, 10);
                if (*end_ptr != '\0')
                {
                    fprintf (stderr, 
                             "Invalid argument to --maxsize/-x option: '%s'\n",
                             optarg);
                    exit (-1);
                }
		if (0 > ((int32)(*max_size)))
		{
		    fprintf (stderr, "Ignoring negative value given for --maxsize/-x option: '%s' \n", optarg);
		    (*max_size) = 0L;
		}
            }
            break;

        case 'h':
            usage(argv[0]);
            exit (0);
            break;

        case 't':
            *flags |= LIST;
            break;

        case 'w':
            *flags |= CONFIRM;
            break;

        case '?':
            fprintf (stderr, "Try '%s --help' for more info.\n", argv[0]);
            exit (1);
            break;

        default:
            abort();            /* this is a problem */
        }
    }
    if (optind < argc)
    {
        (*in_file) = argv[optind++];
        if (optind < argc)
        {
            fprintf (stderr, "%s: Extra parameters -- '", argv[0]);
            while (optind < argc)
                fprintf (stderr, "%s ", argv[optind++]);
            fprintf (stderr, "'\n");
        }
    }
}


int
main (int argc, char *argv[]) 
{
    FILE *fp = NULL;
    char *in_file = NULL;
    char *out_dir = NULL;
    char *body_file = NULL;
    char *body_pref = NULL;
    int flags = NONE;
    size_t max_size = 0;
    
    parse_cmdline (argc, argv, 
		   &in_file, &out_dir, 
		   &body_file, &body_pref,
		   &max_size, 
		   &flags);

    set_alloc_limit (max_size);
    if (flags & DBG_OUT)
    {
        fprintf (stdout, "setting alloc_limit to: %lu\n", 
                 (unsigned long)max_size);
    }

    /* open the file */
    if (in_file)
    {
        fp = fopen (in_file, "rb");
        if (fp == NULL) 
        {
            perror (in_file);
            exit (1);
        }
    }
    else
    {
        fp = stdin;
    }

    if (fp == stdin && flags & CONFIRM)
    {
        fprintf (stderr, 
                 "Cannot read file from STDIN and use "
                 "interactive mode at the same time.\n");
        exit (1);
    }
    
    return parse_file (fp, out_dir, body_file, body_pref, flags);
}
