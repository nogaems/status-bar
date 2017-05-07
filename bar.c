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

    /* Maximal number of layouts are hardcoded to 2. 
       I really don't have an idea how to handle this in case 
       of 2+ groups because this method is based on checking of 
       the Xkb Groups which there are 2 by default. 
       I'll handle this behavior *later*, now it's just works. */

    
    char *layouts[MAX_LAYOUTS_COUNT]; 
    char *layout;
    char *rest = vd.layout;
    int counter = 0;
    
    while((layout = strtok_r(rest, ",", &rest)))
    {
        if (counter == MAX_LAYOUTS_COUNT)
        {
            strcpy(result, "undefined");
            return -1;                
        }
        layouts[counter++] = layout;
    }
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
            }
        }
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
main(void)
{
    static Display *dpy;
    char layout[32];
    
    if (!(dpy = XOpenDisplay(NULL)))
    {
        fprintf(stderr, "dwmstatus: cannot open display.\n");
	return 1;
    }
    getCurrentKeyboardLayout(dpy, layout);
    uppercase(layout);
    printf("%s", layout);
    
    return 0;
}
    

