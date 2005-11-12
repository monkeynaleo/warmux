/******************************************************************************
 *  Wormux, a free clone of the game Worms from Team17.
 *  Copyright (C) 2001-2004 Lawrence Azzoug.
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
 * Menu informations.
 *****************************************************************************/

#include "infos_menu.h"
//-----------------------------------------------------------------------------
#include <stdio.h>
#include "../include/constant.h"
#include "../tool/i18n.h"
using namespace Wormux;
//-----------------------------------------------------------------------------
MenuInfos menu_infos;
//-----------------------------------------------------------------------------

void MenuInfos::Run()
{
#ifdef CL
  //TODO:open the authors file from /usr/include according to the configure
  //TODO:Move the window depending on the screen resolution(?)
  FILE* authors=fopen("../AUTHORS","r");
  char* txt;
  if(authors!=NULL)
  {
    fseek(authors,0,SEEK_END);
    long size=ftell(authors);
    txt=new char[size+1];
    rewind(authors);
    fread(txt,1,size,authors);
    txt[size]='\0';
    fclose(authors);
  }
  else
  {
    txt=new char[strlen("Wormux team!")+1];
    strcpy(txt,"Wormux team!");
  }
  PG_MessageBox* msgbox=new PG_MessageBox(NULL,
                                          PG_Rect(40,50,560,390),
                                          _("Authors"),txt,
                                          PG_Rect(530, 0,30, 30),
                                          _("Ok"),
                                          PG_Label::CENTER);

  msgbox->Show();
  msgbox->WaitForClick();
  delete msgbox;
  delete []txt;
#endif
}

//-----------------------------------------------------------------------------
