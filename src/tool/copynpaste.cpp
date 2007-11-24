/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2007 Wormux Team.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 ******************************************************************************
 * Retrieve string pasted depending on OS mechanisms.
 *****************************************************************************/
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <SDL/SDL_syswm.h>
#include "copynpaste.h"

#ifdef HAVE_X11_XLIB_H
static char* getSelection(Display *dpy, Window us, Atom selection)
{
  int    max_events = 50;
  Window owner      = XGetSelectionOwner (dpy, selection);
  int    ret;

  //printf("XConvertSelection on %s\n", XGetAtomName(dpy, selection));
  if (owner == None)
  {
    //printf("No owner\n");
    return NULL;
  }
  XConvertSelection(dpy, selection, XA_STRING, XA_PRIMARY, us, CurrentTime);
  XFlush(dpy);

  while (max_events--)
  {
    XEvent        e;
    
    XNextEvent(dpy, &e);
    if(e.type == SelectionNotify)
    {
      //printf("Received %s\n", XGetAtomName(dpy, e.xselection.selection));
      if(e.xselection.property == None)
      {
        //printf("Couldn't convert\n");
        return NULL;
      }
      
      long unsigned len, left, dummy;
      int           format;
      Atom          type;
      unsigned char *data = NULL;

      XGetWindowProperty(dpy, us, e.xselection.property, 0, 0, False,
                         AnyPropertyType, &type, &format, &len, &left, &data);
      if (left < 1)
        return NULL;

      ret = XGetWindowProperty(dpy, us, e.xselection.property, 0, left, False,
                               AnyPropertyType, &type, &format, &len, &dummy, &data);
      if (ret != Success)
      {
        //printf("Failed to get property: %p on %lu\n", data, len);
        return NULL;
      }

      //printf(">>>  Got %s: len=%lu left=%lu (event %i)\n", data, len, left, 50-max_events);
      return (char*)data;
    }
  }
  printf("Timeout\n");
  return NULL;
}
#endif

bool RetrieveBuffer(std::string& text, std::string::size_type& pos)
{
  SDL_SysWMinfo info;

  printf("Retrieving buffer...\n");
  SDL_VERSION(&info.version);
  if ( SDL_GetWMInfo(&info) )
  {
#ifdef HAVE_X11_XLIB_H
    Display *dpy  = info.info.x11.display;
    Window  us    = info.info.x11.window;
    char    *data = NULL;

    if (!data)
    {
      data = getSelection(dpy, us, XA_PRIMARY);
    }
    if (!data)
    {
      data = getSelection(dpy, us, XA_SECONDARY);
    }
    if (!data)
    {
      Atom XA_CLIPBOARD = XInternAtom(dpy, "CLIPBOARD", 0);
      data = getSelection(dpy, us, XA_CLIPBOARD);
    }
    if (data)
    {
      // check cursor position
      if (pos > text.size()) {
        pos = text.size();
      }

      text.insert(pos, data); 
      pos += strlen(data);
      XFree(data);
    }
#endif
  }
  return false;
}
