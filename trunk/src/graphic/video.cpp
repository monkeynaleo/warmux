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
 *  MERCHANTABILITY or FITNESS FOR A ARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU GeneralPublic License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 ******************************************************************************
 * Refresh de la carte vidéo.
 *****************************************************************************/

#include "video.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <SDL_endian.h>
#include <SDL_image.h>
#include "../game/config.h"
#include "../tool/error.h"
#include "../tool/i18n.h"
#include "../include/app.h"
#include "../include/constant.h"

using namespace Wormux;

Video::Video() {
  SetMaxFps (50);
  fullscreen = false;
}

void Video::SetMaxFps(uint max_fps){
  m_max_fps = max_fps;
  if (0 < m_max_fps)
    m_sleep_max_fps = 1000/m_max_fps;
  else
    m_sleep_max_fps = 0;
}

uint Video::GetMaxFps(){
  return m_max_fps;
}

uint Video::GetSleepMaxFps(){
  return m_sleep_max_fps;
}

int Video::GetWidth(void) const{
  return sdlwindow->w;
}

int Video::GetHeight(void) const{
  return sdlwindow->h;
}

bool Video::IsFullScreen(void) const{
  return fullscreen;
}

bool Video::SetConfig(int width, int height, bool _fullscreen){
  // initialize the main window
  if( (sdlwindow == NULL) || 
      (width != GetWidth() || height != GetHeight() ) ){

    sdlwindow = SDL_SetVideoMode(width, height,
				     32, //resolution in bpp
				     SDL_HWSURFACE| SDL_HWACCEL |SDL_DOUBLEBUF);

    if( sdlwindow == NULL ) 
      sdlwindow = SDL_SetVideoMode(width, height,
				       32, //resolution in bpp
				       SDL_SWSURFACE);
    
    if( sdlwindow == NULL )
      return false;
    fullscreen = false;
  }

  if(fullscreen != _fullscreen ){
    SDL_WM_ToggleFullScreen(sdlwindow);
    fullscreen = _fullscreen;
  }

  return true;
}

void Video::InitWindow(){
  sdlwindow = NULL;

  SetConfig(config.tmp.video.width,
                  config.tmp.video.height,
                  config.tmp.video.fullscreen);
  
  SetWindowCaption( std::string("Wormux ") + VERSION );
  SetWindowIcon( config.data_dir + "wormux-32.xpm" );

  if( sdlwindow == NULL )
	  Error("dommage");
}

void Video::SetWindowCaption(std::string caption){
  SDL_WM_SetCaption( caption.c_str(), NULL );
}

void Video::SetWindowIcon(std::string filename){
  SDL_WM_SetIcon( IMG_Load(filename.c_str()), NULL );
}

void Video::InitScreen(){
 if ( SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0 )
   Error( Format( _("Unable to initialize SDL library: %s"), SDL_GetError() ) ); 
}

void Video::Flip(){
  SDL_Flip(sdlwindow);
}
