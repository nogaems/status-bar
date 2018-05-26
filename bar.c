/*
 * This file is part of the status-bar distribution (https://github.com/nogaems/status-bar)
 * Copyright (c) 2017 by nogaems <nomad@ag.ru>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBfile.h>
#include <X11/extensions/XKBconfig.h>
#include <X11/extensions/XKBrules.h>

#define MAX_LAYOUTS_COUNT 2

/* Settings */
#define INTERFACE "eth1"
static char *tz = "Europe/Moscow";

static Display *dpy;

char *
smprintf(char *fmt, ...)
{
	va_list fmtargs;
	char *ret;
	int len;

	va_start(fmtargs, fmt);
	len = vsnprintf(NULL, 0, fmt, fmtargs);
	va_end(fmtargs);

	ret = malloc(++len);
	if (ret == NULL) {
		perror("malloc");
		exit(1);
	}

	va_start(fmtargs, fmt);
	vsnprintf(ret, len, fmt, fmtargs);
	va_end(fmtargs);

	return ret;
}

int
getCurrentKeyboardLayout(Display *dpy, char *result)
{    
    XkbRF_VarDefsRec vd;
    char *tmp = NULL;

    /* Getting all the stored Xkb options */
    
    if (!XkbRF_GetNamesProp(dpy, &tmp, &vd) || !tmp)
    {
        /* For some reason Xkb rules are not defined */
        strcpy(result, "undefined");
        return -1;
    }
    free(tmp);

    /* Maximal number of layouts are hardcoded to 2. 
       I really don't have an idea how to handle this in case 
       of 2+ groups because this method is based on checking of 
       the Xkb Groups which there are 2 by default. 
       I'll handle this behavior *later*, now it's just works. */

    
    char layouts[MAX_LAYOUTS_COUNT][256];
    char *layout;
    char *rest = strdup(vd.layout);
    tmp = rest;
    int counter = 0;
    if (vd.layout)
        free(vd.layout);
    if (vd.model)
        free(vd.model);
    if (vd.options)
        free(vd.options);
    if (vd.variant)
        free(vd.variant);

    while((layout = strtok_r(rest, ",", &rest)))
    {
        if (counter == MAX_LAYOUTS_COUNT)
        {
            strcpy(result, "undefined");
            return -1;                
        }
        strcpy(layouts[counter++], layout);
    }
    free(tmp);
    if (counter == 1)
    {
        /* There is defined only one keyboard layout */
        strcpy(result, layouts[0]);
        return 0;
    }

    XkbDescPtr xkb;
    int xkbmajor = XkbMajorVersion, xkbminor = XkbMinorVersion;
    int xkbopcode, xkbevent, xkberror;
    int i, j;
    int grp2;

    if (XkbQueryExtension(dpy, &xkbopcode, &xkbevent, &xkberror, &xkbmajor, &xkbminor) &&
        (xkb = XkbAllocKeyboard()) != NULL)
    {
	if (XkbGetNames(dpy, XkbIndicatorNamesMask, xkb) == Success) {
	    Atom iatoms[XkbNumIndicators];
	    char *iatomnames[XkbNumIndicators];
	    Bool istates[XkbNumIndicators];
	    int inds[XkbNumIndicators];            

            /* Getting all Atoms of keyboard indicators */
            
	    for (i = 0, j = 0; i < XkbNumIndicators; i++) {
		if (xkb->names->indicators[i] != None) {                    
		    iatoms[j++] =  xkb->names->indicators[i];
		}
	    }

            /* Extracting indicators names from the Atoms */
            
	    if (XGetAtomNames(dpy, iatoms, j, iatomnames)) {                
		for (i = 0; i < j; i++) {
		    XkbGetNamedIndicator(dpy, iatoms[i], &inds[i],
                                         &istates[i], NULL, NULL);
		}
	    }
            grp2 = -1;
            for (i = 0; i < j; i++) {
                if (!strcmp(iatomnames[i], "Group 2"))
                {
                    /* Saving status if "Group 2" indicator */
                    grp2 = istates[i];
                }
                free(iatomnames[i]);
            }
        }
        XkbFreeNames(xkb, XkbAllNamesMask, true);
        free(xkb);
    }
    if (grp2 == -1)
    {
        /* Xkb settings has no status of "Group 2" indicator 
           but there is more than one layout. */
        strcpy(result, "undefined");
        return -1;
    }
    else
    {
        strcpy(result, layouts[grp2]);
    }
    return 0;
}

void
uppercase(char *str)
{
    int i;
    for (i = 0; i < strlen(str); i++ )
    {
        str[i] = toupper(str[i]);
    }
}

int
parse_netdev(unsigned long long int *receivedabs, unsigned long long int *sentabs)
{
	char buf[255];
	char *datastart;
	static int bufsize;
	int rval;
	FILE *devfd;
	unsigned long long int receivedacc, sentacc;

	bufsize = 255;
	devfd = fopen("/proc/net/dev", "r");
	rval = 1;

	// Ignore the first two lines of the file
	fgets(buf, bufsize, devfd);
	fgets(buf, bufsize, devfd);

	while (fgets(buf, bufsize, devfd)) {
	    if ((datastart = strstr(buf, INTERFACE)) != NULL) {
		datastart = strstr(buf, ":");

		// With thanks to the conky project at http://conky.sourceforge.net/
		sscanf(datastart + 1, "%llu  %*d     %*d  %*d  %*d  %*d   %*d        %*d       %llu",\
		       &receivedacc, &sentacc);
		*receivedabs += receivedacc;
		*sentabs += sentacc;
		rval = 0;
	    }
	}

	fclose(devfd);
	return rval;
}

void
calculate_speed(char *speedstr, unsigned long long int newval, unsigned long long int oldval)
{
	double speed;
	speed = (newval - oldval) / 1024.0;
	if (speed > 1024.0) {
	    speed /= 1024.0;
	    sprintf(speedstr, "%.3f MB/s", speed);
	} else {
	    sprintf(speedstr, "%.2f KB/s", speed);
	}
}

char *
get_netusage(unsigned long long int *rec, unsigned long long int *sent)
{
	unsigned long long int newrec, newsent;
	newrec = newsent = 0;
	char downspeedstr[15], upspeedstr[15];
	static char retstr[42];
	int retval;

	retval = parse_netdev(&newrec, &newsent);
	if (retval) {
	    fprintf(stdout, "Error when parsing /proc/net/dev file.\n");
	    exit(1);
	}

	calculate_speed(downspeedstr, newrec, *rec);
	calculate_speed(upspeedstr, newsent, *sent);

	sprintf(retstr, "D: %s U: %s", downspeedstr, upspeedstr);

	*rec = newrec;
	*sent = newsent;
	return retstr;
}

void
setstatus(char *str)
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

void
settz(char *tzname)
{
	setenv("TZ", tzname, 1);
}
char *
mktimes(char *fmt, char *tzname)
{
	char buf[129];
	time_t tim;
	struct tm *timtm;

	bzero(buf, sizeof(buf));
	settz(tzname);
	tim = time(NULL);
	timtm = localtime(&tim);
	if (timtm == NULL) {
		perror("localtime");
		exit(1);
	}

	if (!strftime(buf, sizeof(buf)-1, fmt, timtm)) {
		fprintf(stderr, "strftime == 0\n");
		exit(1);
	}

	return smprintf("%s", buf);
}

int
main(void)
{    
    char *status;
    char layout[32];
    char *netstats;
    char *tm;
    static unsigned long long int rec, sent;

    parse_netdev(&rec, &sent);
    
    if (!(dpy = XOpenDisplay(NULL)))
    {
        fprintf(stderr, "dwmstatus: cannot open display.\n");
	return 1;
    }
    for (;;sleep(1)) {        
        getCurrentKeyboardLayout(dpy, layout);
        uppercase(layout);
        netstats = get_netusage(&rec, &sent);
        tm = mktimes("%a %x %X", tz);
        status = smprintf("%s | %s | %s " ,
                          layout, netstats, tm);
        setstatus(status);
        free(tm);
        free(status);
    }
    free(dpy);
    return 0;
}
    

