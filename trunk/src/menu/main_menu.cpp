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
 * Main_Menu du jeu permettant de lancer une partie, modifier les options, d'obtenir
 * des informations, ou encore quitter le jeu.
 *****************************************************************************/

#include "main_menu.h"
#include <string>
#include "../game/config.h"
#include "../game/time.h"
#include "../graphic/effects.h"
#include "../graphic/font.h"
#include "../graphic/fps.h"
#include "../include/app.h"
#include "../include/constant.h"
#include "../sound/jukebox.h"
#include "../tool/i18n.h"
#include "../tool/file_tools.h"
#include "../tool/resource_manager.h"
#include <iostream>

#define NETWORK_BUTTON 

#ifndef WIN32
#include <dirent.h>
#endif


// Position du texte de la version
const int VERSION_DY = -40;

const int DEFAULT_SCREEN_HEIGHT = 768 ;

// Animation time constant
const uint bfall_end = 500; //Buttons parameters
const uint boscill_end = 1250;
const uint tfall_end = 800; //Title parameters
const uint toscill_end = 1400;
const uint sfall_end = 1300; //Skin parameters
const uint soscill_end = 2000;

//duration before full background refresh
const int bg_refresh = soscill_end;

//Fps limiters limit
const int max_fps = 20;

Main_Menu::~Main_Menu(){
  delete background;
  delete title.spr;
  delete skin_left.spr;
  delete skin_right.spr;

  delete version_text;
  delete website_text;
}

Main_Menu::Main_Menu()
{
  int x_button;
  double y_scale;

  normal_font = Font::GetInstance(Font::FONT_NORMAL);
  large_font = Font::GetInstance(Font::FONT_LARGE);

  Config * config = Config::GetInstance();
  background = new Sprite(Surface( 
				  config->GetDataDir() + PATH_SEPARATOR 
				  + "menu" + PATH_SEPARATOR 
				  + "img" + PATH_SEPARATOR
				  + "background.png" ));
  background->cache.EnableLastFrameCache();

  title.spr = new Sprite(Surface( 
				 config->GetDataDir() + PATH_SEPARATOR
				 + "menu" + PATH_SEPARATOR
				 + "img" + PATH_SEPARATOR 
				 + "title.png" ));

  skin_left.spr = new Sprite(Surface( 
				     config->GetDataDir() + PATH_SEPARATOR 
				     + "menu" + PATH_SEPARATOR 
				     + "img" + PATH_SEPARATOR
				     "skin01.png" ));

  skin_right.spr = new Sprite(Surface( 
				      config->GetDataDir() + PATH_SEPARATOR
				      + "menu" + PATH_SEPARATOR 
				      + "img" + PATH_SEPARATOR 
				      + "skin02.png" ));

  button_height = 64;
  button_width = 402;
  skin_offset = 50;
  title_offset = 50;

  y_scale = (double)AppWormux::GetInstance()->video.window.GetHeight() / DEFAULT_SCREEN_HEIGHT ;

  x_button = AppWormux::GetInstance()->video.window.GetWidth()/2 - button_width/2;

  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml", false);

  int y = 300;
  const int y2 = 580;
#ifdef NETWORK_BUTTON  
  int dy = (y2-y)/4;
#else  
  int dy = (y2-y)/3;
#endif  

  play = new ButtonText( Point2i(x_button, (int)(y * y_scale)),
			res, "main_menu/button",
			_("Play"),
			large_font);
  y += dy;

#ifdef NETWORK_BUTTON  
  network = new ButtonText( Point2i(x_button, (int)(y * y_scale)),
			   res, "main_menu/button",
			   _("Network Game"),
			   large_font );
  y += dy;
#else
  network = NULL;
#endif
  
  options = new ButtonText( Point2i(x_button, (int)(y * y_scale) ),
			   res, "main_menu/button",
			   _("Options"),
			   large_font);
  y += dy;

  infos =  new ButtonText( Point2i(x_button, (int)(y * y_scale) ),
			  res, "main_menu/button",
			  _("Credits"),
			  large_font);
  y += dy;

  quit =  new ButtonText( Point2i(x_button,(int)(y * y_scale) ),
			 res, "main_menu/button",
			 _("Quit"),
			 large_font);

  widgets.AddWidget(play);
#ifdef NETWORK_BUTTON 
  widgets.AddWidget(network);
#endif
  widgets.AddWidget(options);
  widgets.AddWidget(infos);
  widgets.AddWidget(quit);

  resource_manager.UnLoadXMLProfile( res);

  std::string s("Version "+Constants::VERSION);
  version_text = new Text(s, green_color, normal_font, false);

  std::string s2(Constants::WEB_SITE);
  website_text = new Text(s2, green_color, normal_font, false);
}

void Main_Menu::button_clic()
{
  jukebox.Play("share", "menu/clic");
}

void Main_Menu::OnClic(const Point2i &mousePosition, int button)
{
  Widget* b = widgets.Clic(mousePosition,button);
  if(b == play) choice = menuPLAY;
#ifdef NETWORK_BUTTON  
  else if(b == network) choice = menuNETWORK;
#endif  
  else if(b == options) choice = menuOPTIONS;
  else if(b == infos) choice = menuCREDITS;
  else if(b == quit) choice = menuQUIT;

  if(b != quit && b != NULL) button_clic();
}

menu_item Main_Menu::Run ()
{
  int x=0, y=0;
  uint sleep_fps=0;

  background->ScaleSize(AppWormux::GetInstance()->video.window.GetWidth(), AppWormux::GetInstance()->video.window.GetHeight());
  DrawBackground(); //Display background
  fps.Reset();
  start_time = Time::GetInstance()->Read();
  last_refresh = start_time;
  anim_finished = false;
  choice = menuNULL;
  while (choice == menuNULL){
    
    // Poll and treat events
    SDL_Event event;
    unsigned int start = SDL_GetTicks();
    
    while( SDL_PollEvent( &event) )
      {
	if( event.type == SDL_MOUSEBUTTONUP )
	  OnClic( Point2i(event.button.x, event.button.y), event.button.button);
	else if( event.type == SDL_KEYDOWN )
	  {
	    if( event.key.keysym.sym == SDLK_ESCAPE )
	      {
		choice = menuQUIT;
		break;
	      }       
	    if( event.key.keysym.sym == SDLK_RETURN )
	      {
		choice = menuPLAY;
		break;
	      }       
	  }
	else if ( event.type == SDL_QUIT)
	  {
	    choice = menuQUIT;
	    break;
	  }
      }

    SDL_GetMouseState( &x, &y);
    
    Draw( Point2i(x, y) );
    
    last_refresh = Time::GetInstance()->Read();
    fps.Refresh();
    fps.AddOneFrame();
    //fps.Draw();
    AppWormux::GetInstance()->video.Flip();

    //fps limiter
    uint delay = SDL_GetTicks()-start;
    if (delay < 1000 / max_fps)
      sleep_fps = 1000 / max_fps - delay;
    else
      sleep_fps = 0;
    SDL_Delay(sleep_fps);  
  }

  return choice;
}

void Main_Menu::Draw(const Point2i &mousePosition)
{
  uint dt = Time::GetInstance()->Read() - start_time;

  if (anim_finished) {
    widgets.Draw(mousePosition);
    return;
  }

  if( last_refresh / bg_refresh != Time::GetInstance()->Read() / bg_refresh
      || dt > soscill_end)
  {
    //Refresh all the screen    
    //std::cout << "Refresh all the screen"<< std::endl;
    DrawBackground();
    DrawSkins(dt);
    DrawTitle(dt);
    DrawButtons(mousePosition, dt);
    anim_finished = true;
  }
  else
  {
    EraseGfx(dt);
    DrawGfx(mousePosition, dt);
  }
}

void Main_Menu::DrawBackground()
{
  AppWormux * app = AppWormux::GetInstance();
  
  background->Blit( app->video.window, 0, 0);
  version_text->DrawCenter( app->video.window.GetWidth()/2,
			    app->video.window.GetHeight() + VERSION_DY);
  website_text->DrawCenter( app->video.window.GetWidth()/2,
			    app->video.window.GetHeight() + VERSION_DY/2);
}

void Main_Menu::EraseGfx(uint dt)
{
  if( dt <= toscill_end && dt >= bfall_end )
  {
    // Erase previous title
    Redraw(Rectanglei(title.pos.GetX(), title.pos.GetY(),
		      title.spr->GetWidth(), title.spr->GetHeight()));
  }

  if( dt <= soscill_end && dt >= tfall_end )
  {
    // Erase previous left skin
    Redraw(Rectanglei(skin_left.pos.GetX(), skin_left.pos.GetY(), 
		      skin_left.spr->GetWidth(), skin_left.spr->GetHeight()));

    // Erase previous left right
    Redraw(Rectanglei(skin_right.pos.GetX(), skin_right.pos.GetY(), 
		      skin_right.spr->GetWidth(), skin_right.spr->GetHeight()));
  }
}

void Main_Menu::Redraw(const Rectanglei& rect)
{
  background->Blit(AppWormux::GetInstance()->video.window, rect, rect.GetPosition());

  // Redraw part of the Left skin
  
  // Redraw part of the Right skin
}

void Main_Menu::DrawGfx(const Point2i &mousePosition, uint dt)
{    
  if( dt <= toscill_end && dt >= bfall_end )
    DrawTitle(dt);
  if( dt <= soscill_end && dt >= tfall_end )
    DrawSkins(dt);
  DrawButtons(mousePosition, dt);
}

void Main_Menu::DrawSkins(uint dt)
{
  AppWormux * app = AppWormux::GetInstance();

  //Calculate inital position of the buttons depending on windows size.
  double y_scale = (double)app->video.window.GetHeight() / DEFAULT_SCREEN_HEIGHT ;
  int x_button = app->video.window.GetWidth()/2 - button_width/2;

  ////////////////////////////////  Skins XY, scale ///////////////////////////
  if(dt >= soscill_end)
  {
    //Skin final position
    skin_left.spr->cache.EnableLastFrameCache();
    skin_left.spr->Scale(y_scale, y_scale);
    skin_left.pos.SetValues(x_button/2 - skin_left.spr->GetWidth()/2,
			    app->video.window.GetHeight() - skin_left.spr->GetHeight() - 50);

    skin_right.spr->cache.EnableLastFrameCache();
    skin_right.spr->Scale(y_scale, y_scale);
    skin_right.pos.SetValues(app->video.window.GetWidth() - x_button/2 - skin_right.spr->GetWidth()/2,
			     app->video.window.GetHeight() - skin_right.spr->GetHeight() - 50);
  }
  else
  if(dt >= sfall_end)
  {
    skin_left.spr->cache.DisableLastFrameCache(); //Disabled to get non smoothed scale
    skin_left.spr->Scale(y_scale, y_scale);

    //Make the skins oscillate
    int skin_dh = 0;
    int skin_dy = 0;
    Gelatine(skin_dy, skin_dh, start_time + sfall_end, skin_left.spr->GetHeight()/8, soscill_end - sfall_end, 2);

    skin_left.spr->cache.DisableLastFrameCache(); //Disabled to get non smoothed scale
    skin_left.spr->ScaleSize(skin_left.spr->GetWidth(), skin_left.spr->GetHeight()+skin_dh);
    skin_left.pos.SetValues(x_button/2 - skin_left.spr->GetWidth()/2,
			    app->video.window.GetHeight() - skin_left.spr->GetHeight() - 50 + skin_dy);

    skin_right.spr->cache.DisableLastFrameCache();
    skin_right.spr->ScaleSize(skin_right.spr->GetWidth(), skin_right.spr->GetHeight()+skin_dh);
    skin_right.pos.SetValues(app->video.window.GetWidth() - x_button/2 - skin_right.spr->GetWidth()/2,
			     app->video.window.GetHeight() - skin_right.spr->GetHeight() - 50 + skin_dy);

  }
  else
  if(dt >= tfall_end) //Skins begins to fall when title ends falling
  {
    uint dt2 = dt - tfall_end;
    uint fall_duration = sfall_end - tfall_end;

    skin_left.spr->cache.EnableLastFrameCache();
    skin_left.spr->Scale(y_scale, y_scale);
    skin_left.pos.SetValues(x_button/2 - skin_left.spr->GetWidth()/2,
			    (dt2*dt2*app->video.window.GetHeight()/fall_duration/fall_duration) - skin_right.spr->GetHeight() - 50); 
 
    skin_right.spr->cache.EnableLastFrameCache();
    skin_right.spr->Scale(y_scale, y_scale);
    skin_right.pos.SetValues (app->video.window.GetWidth() - x_button/2 - skin_right.spr->GetWidth()/2,
			      (dt2*dt2*app->video.window.GetHeight()/fall_duration/fall_duration) - skin_right.spr->GetHeight() - 50); 
  }
  else
  {
    skin_left.pos.SetValues(x_button/2 - skin_left.spr->GetWidth()/2,
			    -skin_left.spr->GetHeight());
    skin_right.pos.SetValues(app->video.window.GetWidth() - x_button/2 - skin_right.spr->GetWidth()/2,
			     -skin_right.spr->GetHeight());
  }

  //Draw the skins at final position/scale +- delta
  skin_left.spr->Blit(app->video.window, skin_left.pos);
  skin_right.spr->Blit(app->video.window, skin_right.pos);
}

void Main_Menu::DrawTitle(uint dt)
{
  AppWormux * app = AppWormux::GetInstance();

  //Calculate inital position of the buttons depending on windows size.
  double y_scale = (double)app->video.window.GetHeight() / DEFAULT_SCREEN_HEIGHT ;

  ////////////////////////////////  Title XY, scale ///////////////////////////
  if(dt >= toscill_end)
  {
    //Title final position
    title.spr->cache.EnableLastFrameCache();
    title.spr->Scale(y_scale, y_scale);
    title.pos.SetValues(app->video.window.GetWidth()/2 - title.spr->GetWidth()/2, 50);
  }
  else
  if(dt >= tfall_end)
  {
    //Make the title oscillate
    int title_dh = 0;
    int title_dy = 0;
    Gelatine(title_dy, title_dh, start_time + tfall_end, title.spr->GetHeight()/4, toscill_end - tfall_end, 2);

    title.spr->cache.DisableLastFrameCache();
    title.spr->Scale(y_scale, y_scale);
    title.spr->ScaleSize(title.spr->GetWidth(), title.spr->GetHeight()+title_dh);
    title.pos.SetValues(app->video.window.GetWidth()/2 - title.spr->GetWidth()/2, 50 + title_dy);
  }
  else
  if(dt >= bfall_end) //Skins begins to fall when title ends falling
  {
    uint dt2 = dt - bfall_end;
    uint fall_duration = tfall_end - bfall_end;
    title.spr->cache.EnableLastFrameCache();
    title.spr->Scale(y_scale, y_scale);
    title.pos.SetValues(app->video.window.GetWidth()/2 - title.spr->GetWidth()/2,
			(int)((dt2*dt2*(title.spr->GetHeight()+50)/fall_duration/fall_duration) - title.spr->GetHeight()));
  }
  else
  {
    title.pos.SetValues(app->video.window.GetWidth()/2 - title.spr->GetWidth()/2,
			-title.spr->GetHeight());
  }
  //Draw the title at final position/scale +- delta
  title.spr->Blit(app->video.window, title.pos);
}

void Main_Menu::DrawButtons(const Point2i &mousePosition, uint dt)
{
  AppWormux * app = AppWormux::GetInstance();

  //Calculate inital position of the buttons depending on windows size.
  double y_scale = (double)app->video.window.GetHeight() / DEFAULT_SCREEN_HEIGHT ;
  int x_button = app->video.window.GetWidth()/2 - button_width/2;

  //////////////////////////// Buttons position / scale //////////////////

  int y = 300;
  const int y2 = 580;
#ifdef NETWORK_BUTTON  
  int dy = (y2-y)/4;
#else  
  int dy = (y2-y)/3;
#endif  
  int y_play = (int)(y * y_scale);//Position
  y += dy;
#ifdef NETWORK_BUTTON  
  int y_network = (int)(y * y_scale);//Position
  y += dy;
#endif
  int y_options = (int)(y * y_scale); //Position
  y += dy;
  int y_infos =  (int)(y * y_scale); //Position
  y += dy;
  int y_quit =  (int)(y * y_scale); //Position

  if ( dt >= boscill_end)
  {
    if (!anim_finished) {
      //std::cout << "if ( dt >= boscill_end )" << std::endl;
      
      // Finish the animation for buttons
      play->GetSprite()->cache.EnableLastFrameCache();
      play->SetSizePosition( Rectanglei(x_button, y_play, button_width, button_height) );
      play->ForceRedraw();
#ifdef NETWORK_BUTTON  
      network->GetSprite()->cache.EnableLastFrameCache();
      network->SetSizePosition( Rectanglei(x_button, y_network, button_width, button_height) );
      network->ForceRedraw();
#endif
	
      options->GetSprite()->cache.EnableLastFrameCache();
      options->SetSizePosition( Rectanglei(x_button, y_options, button_width, button_height) );
      options->ForceRedraw();
      
      infos->GetSprite()->cache.EnableLastFrameCache();
      infos->SetSizePosition( Rectanglei(x_button, y_infos, button_width, button_height) );
      infos->ForceRedraw();	
      
      quit->GetSprite()->cache.EnableLastFrameCache();
      quit->SetSizePosition( Rectanglei(x_button, y_quit, button_width, button_height) );
      quit->ForceRedraw();
    }
  }
  else if ( dt >= bfall_end && dt <= boscill_end)
  {    
    //std::cout <<  "else if ( dt >= bfall_end && dt <= boscill_end)" << std::endl;

    //Buttons finished falling, make them oscillate
    int button_dy, button_dh;

    Gelatine(button_dy, button_dh, start_time + bfall_end, dy - button_height, boscill_end - bfall_end, 2);
    play->GetSprite()->cache.DisableLastFrameCache();
    play->SetSize(button_width, button_height + button_dh);
    play->SetXY(x_button, y_play + button_dy);
    play->ForceRedraw();
#ifdef NETWORK_BUTTON
    network->GetSprite()->cache.DisableLastFrameCache();
    network->SetSize(button_width, button_height + button_dh);
    network->SetXY(x_button, y_network + button_dy);
    network->ForceRedraw();
#endif
    options->GetSprite()->cache.DisableLastFrameCache();
    options->SetSize(button_width, button_height + button_dh);
    options->SetXY(x_button, y_options + button_dy);
    options->ForceRedraw();
    infos->GetSprite()->cache.DisableLastFrameCache();
    infos->SetSize(button_width, button_height + button_dh);
    infos->SetXY(x_button, y_infos + button_dy);
    infos->ForceRedraw();
    quit->GetSprite()->cache.DisableLastFrameCache();
    quit->SetSize(button_width, button_height + button_dh);
    quit->SetXY(x_button, y_quit + button_dy);
    quit->ForceRedraw();
  }
  else
  {
    //std::cout <<  "else //Buttons are falling " << std::endl;

    uint fall_duration = bfall_end;
    //Buttons are falling
    play   ->GetSprite()->cache.EnableLastFrameCache();
    play   ->SetXY(x_button, (dt*dt*app->video.window.GetHeight()/fall_duration/fall_duration) - app->video.window.GetHeight() + y_play);
#ifdef NETWORK_BUTTON
    network->GetSprite()->cache.EnableLastFrameCache();
    network->SetXY(x_button, (dt*dt*app->video.window.GetHeight()/fall_duration/fall_duration) - app->video.window.GetHeight() + y_network);
#endif
    options->GetSprite()->cache.EnableLastFrameCache();
    options->SetXY(x_button, (dt*dt*app->video.window.GetHeight()/fall_duration/fall_duration) - app->video.window.GetHeight() + y_options);
    infos  ->GetSprite()->cache.EnableLastFrameCache();
    infos  ->SetXY(x_button, (dt*dt*app->video.window.GetHeight()/fall_duration/fall_duration) - app->video.window.GetHeight() + y_infos);

    quit   ->GetSprite()->cache.EnableLastFrameCache();
    quit   ->SetXY(x_button, (dt*dt*app->video.window.GetHeight()/fall_duration/fall_duration) - app->video.window.GetHeight() + y_quit);
  }

  widgets.Draw(mousePosition);
}
