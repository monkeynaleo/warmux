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
#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include "infos_menu.h"
#include "../game/config.h"
#include "../game/time.h"
#include "../graphic/effects.h"
#include "../graphic/font.h"
#include "../graphic/fps.h"
#include "../include/app.h"
#include "../include/global.h"
#include "../include/constant.h" // VERSION
#include "../sound/jukebox.h"
#include "../tool/i18n.h"
#include "../tool/file_tools.h"
#include "../tool/resource_manager.h"

//#define NETWORK_BUTTON 

#ifndef WIN32
#include <dirent.h>
#endif

using namespace Wormux;
using namespace std;

// Position du texte de la version
const int VERSION_DY = -40;

const int DEFAULT_SCREEN_HEIGHT = 768 ;

// Animation time constant
const uint bfall_end = 500; //Buttons parameters
const uint boscill_end = 1250;
const uint tfall_end = 1000; //Title parameters
const uint toscill_end = 1750;
const uint sfall_end = 1500; //Skin parameters
const uint soscill_end = 2250;

//duration before full background refresh
const int bg_refresh = soscill_end;

Main_Menu::~Main_Menu(){
  delete background;
  delete title;
  delete skin_left;
  delete skin_right;
  delete play;
  delete network;
  delete options;
  delete infos;
  delete quit;

  delete version_text;
  delete website_text;
}

void Main_Menu::button_click(){
  jukebox.Play("share", "menu/clic");
}

bool Main_Menu::sig_play(){
  button_click();
  choice = menuPLAY;
  return true;
}

bool Main_Menu::sig_network(){
  button_click();
  choice = menuNETWORK;
  return true;
}
  
bool Main_Menu::sig_options(){ 
  button_click(); 
  choice = menuOPTIONS;
  return true; 
}

bool Main_Menu::sig_infos(){ 
  button_click(); 
  menu_infos.Run();
  return true; 
}
  
bool Main_Menu::sig_quit(){ 
  choice=menuQUIT;return true; }

Main_Menu::Main_Menu(){
  int x_button;
  double y_scale;

  background = new Sprite(IMG_Load((config.data_dir+"menu/img/background.png").c_str()));
  background->EnableLastFrameCache();
  title = new Sprite(IMG_Load((config.data_dir+"menu/img/title.png").c_str()));
  title->EnableLastFrameCache();
  skin_left = new Sprite(IMG_Load((config.data_dir+"menu/img/skin01.png").c_str()));
  skin_left->EnableLastFrameCache();
  skin_right = new Sprite(IMG_Load((config.data_dir+"menu/img/skin02.png").c_str()));
  skin_right->EnableLastFrameCache();

  button_height = 64;
  button_width = 402;
  skin_offset = 50;
  title_offset = 50;
  skinl_y = 0;
  skinr_y = 0;
  title_y = 0;

  y_scale = (double)app.video.GetHeight() / DEFAULT_SCREEN_HEIGHT ;

  x_button = app.video.GetWidth()/2 - button_width/2;

  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml", false);

  int y = 300;
  const int y2 = 580;
#ifdef NETWORK_BUTTON  
  int dy = (y2-y)/4;
#else  
  int dy = (y2-y)/3;
#endif  

  play = new ButtonText(x_button,(uint)(y * y_scale),//Position
			res, "main_menu/button",
			_("Play"),
			&global().large_font());
  y += dy;

#ifdef NETWORK_BUTTON  
  network = new ButtonText(x_button,(int)(y * y_scale), //Position
			   res, "main_menu/button",
			   _("Network Game"),
			   &global().large_font() );
  y += dy;
#else
  network = NULL;
#endif
  
  options = new ButtonText(x_button,(int)(y * y_scale), //Position
			   res, "main_menu/button",
			   _("Options"),
			   &global().large_font());
  y += dy;

  infos =  new ButtonText(x_button,(int)(y * y_scale), //Position
			  res, "main_menu/button",
			  _("Credits"),
			  &global().large_font());
  y += dy;

  quit =  new ButtonText(x_button,(int)(y * y_scale), //Position
			 res, "main_menu/button",
			 _("Quit"),
			 &global().large_font());

  resource_manager.UnLoadXMLProfile( res);

  std::string s("Version "+VERSION);
  version_text = new Text(s, green_color, &global().normal_font(), false);

  std::string s2(WEB_SITE);
  website_text = new Text(s2, green_color, &global().normal_font(), false);
}

void Main_Menu::onClick ( int x, int y, int button)
{       
  if (play->MouseIsOver (x, y)) sig_play();
#ifdef NETWORK_BUTTON  
  else if (network->MouseIsOver (x, y)) sig_network();
#endif  
  else if (options->MouseIsOver (x, y)) sig_options();
  else if (infos->MouseIsOver (x, y)) sig_infos();
  else if (quit->MouseIsOver (x, y)) sig_quit();
}

menu_item Main_Menu::Run ()
{
  int x=0, y=0;

  background->ScaleSize(app.video.GetWidth(), app.video.GetHeight());
  EraseAll(); //Display background
  image_par_seconde.Reset();
  start_time = global_time.Read();
  last_refresh = start_time;
  anim_frame_count = 0;
  choice = menuNULL;
  while (choice == menuNULL){
    
  // Poll and treat events
  SDL_Event event;
     
  while( SDL_PollEvent( &event) )
  {
    if( event.type == SDL_MOUSEBUTTONDOWN )
    {
      onClick( event.button.x, event.button.y, event.button.button);
    }
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
   
  Draw(x, y);

  last_refresh = global_time.Read();
  image_par_seconde.Refresh();
  image_par_seconde.Draw();
  image_par_seconde.AjouteUneImage();
  app.video.Flip();
  }
  
  return choice;
}

void Main_Menu::Draw (int mx, int my) // mx,my = mouse location
{
  uint dt = global_time.Read() - start_time;
  if( last_refresh / bg_refresh != global_time.Read() / bg_refresh )
  {
    //Refresh all the screen
    EraseAll();
    DrawSkins(dt);
    DrawTitle(dt);
    DrawButtons(mx,my,dt);
  }
  else
  {
    //Refresh only modified parts
    EraseGfx(dt);
    DrawGfx(mx,my,dt);
  }
}

void Main_Menu::EraseAll()
{
    background->Blit( app.video.sdlwindow, 0, 0);
    version_text->DrawCenter( app.video.GetWidth()/2,
                              app.video.GetHeight() + VERSION_DY);
    website_text->DrawCenter( app.video.GetWidth()/2,
                              app.video.GetHeight() + VERSION_DY/2);
}

void Main_Menu::EraseGfx(uint dt)
{
  //Show background behind gfxs
  int x_button = app.video.GetWidth()/2 - button_width/2;
  if( dt < boscill_end )
  {
    //Clean buttons bg
    background->Blit(app.video.sdlwindow,   play->GetX(),   play->GetY(),   play->GetX(),   play->GetY(),   play->GetW(),   play->GetH());
#ifdef NETWORK_BUTTON  
    background->Blit(app.video.sdlwindow,network->GetX(),network->GetY(),network->GetX(),network->GetY(),network->GetW(),network->GetH());
#endif
    background->Blit(app.video.sdlwindow,options->GetX(),options->GetY(),options->GetX(),options->GetY(),options->GetW(),options->GetH());
    background->Blit(app.video.sdlwindow,  infos->GetX(),  infos->GetY(),  infos->GetX(),  infos->GetY(),  infos->GetW(),  infos->GetH());
    background->Blit(app.video.sdlwindow,   quit->GetX(),   quit->GetY(),   quit->GetX(),   quit->GetY(),   quit->GetW(),   quit->GetH());
  }
  if( dt <= toscill_end && dt >= bfall_end )
  {
    // Erase previous title
    uint title_x = app.video.GetWidth()/2 - title->GetWidth()/2;
    background->Blit(app.video.sdlwindow,title_x, title_y,
                                         title_x, title_y,
                                         title->GetWidth(), title->GetHeight());
  }

  if( dt <= soscill_end && dt >= tfall_end )
  {
    // Erase previous skin
    uint x_offset = x_button/2 - skin_left->GetWidth()/2;
    background->Blit(app.video.sdlwindow,x_offset, skinl_y,
                                         x_offset, skinl_y,
                                         skin_left->GetWidth(), skin_left->GetHeight());
    x_offset = app.video.GetWidth() - x_button/2 - skin_right->GetWidth()/2;
    background->Blit(app.video.sdlwindow,x_offset, skinr_y,
                                         x_offset, skinr_y,
                                         skin_right->GetWidth(), skin_right->GetHeight());
  }
}

void Main_Menu::DrawGfx(int mx, int my, uint dt)
{
  if( dt <= toscill_end && dt >= bfall_end )
    DrawTitle(dt);
  if( dt <= soscill_end && dt >= tfall_end )
    DrawSkins(dt);
  DrawButtons(mx, my, dt);
}

void Main_Menu::DrawSkins(uint dt)
{
  //Calculate inital position of the buttons depending on windows size.
  double y_scale = (double)app.video.GetHeight() / DEFAULT_SCREEN_HEIGHT ;
  int x_button = app.video.GetWidth()/2 - button_width/2;

  ////////////////////////////////  Skins XY, scale ///////////////////////////
  if(dt >= soscill_end)
  {
    //Skin final position
    skin_left->Scale(y_scale, y_scale);
    skinl_y = app.video.GetHeight() - skin_left->GetHeight() - skin_offset;
    skin_right->Scale(y_scale, y_scale);
    skinr_y = app.video.GetHeight() - skin_right->GetHeight() - skin_offset;
  }
  else
  if(dt >= sfall_end)
  {
    //Make the skins oscillate
    int skin_dh = 0;
    int skin_dy = 0;
    Gelatine(skin_dy, skin_dh, start_time + sfall_end, skin_left->GetHeight()/8, soscill_end - sfall_end, 2);

    skin_left->Scale(y_scale, y_scale);
    skinl_y = app.video.GetHeight() - skin_left->GetHeight() - skin_offset + skin_dy;
    skin_left->ScaleSize(skin_left->GetWidth(), skin_left->GetHeight()+skin_dh);

    skin_right->Scale(y_scale, y_scale);
    skinr_y = app.video.GetHeight() - skin_right->GetHeight() - skin_offset + skin_dy;
    skin_right->ScaleSize(skin_right->GetWidth(), skin_right->GetHeight()+skin_dh);
  }
  else
  if(dt >= tfall_end) //Skins begins to fall when title ends falling
  {
    uint dt2 = dt - tfall_end;
    uint fall_duration = sfall_end - tfall_end;
    skin_left->Scale(y_scale, y_scale);
    skinl_y = app.video.GetHeight() - skin_left->GetHeight() - skin_offset;
    skinl_y = (dt2*dt2*app.video.GetHeight()/fall_duration/fall_duration) - app.video.GetHeight() + skinl_y;

    skin_right->Scale(y_scale, y_scale);
    skinr_y = app.video.GetHeight() - skin_right->GetHeight() - skin_offset;
    skinr_y = (dt2*dt2*app.video.GetHeight()/fall_duration/fall_duration) - app.video.GetHeight() + skinr_y;
  }
  else
  {
    skinl_y = -skin_left->GetHeight();
    skinr_y = -skin_right->GetHeight();
  }

  //Draw the skins at final position/scale +- delta
  uint x_offset = x_button/2 - skin_left->GetWidth()/2;
  skin_left->Blit(app.video.sdlwindow, x_offset, skinl_y);
  x_offset = app.video.GetWidth() - x_button/2 - skin_right->GetWidth()/2;
  skin_right->Blit(app.video.sdlwindow, x_offset, skinr_y);
}

void Main_Menu::DrawTitle(uint dt)
{
  //Calculate inital position of the buttons depending on windows size.
  double y_scale = (double)app.video.GetHeight() / DEFAULT_SCREEN_HEIGHT ;

  ////////////////////////////////  Title XY, scale ///////////////////////////
  if(dt >= toscill_end)
  {
    //Title final position
    title->Scale(y_scale, y_scale);
    title_y = title_offset;
  }
  else
  if(dt >= tfall_end)
  {
    //Make the title oscillate
    int title_dh = 0;
    int title_dy = 0;
    Gelatine(title_dy, title_dh, start_time + tfall_end, title->GetHeight()/4, toscill_end - tfall_end, 2);

    title->Scale(y_scale, y_scale);
    title_y = title_offset + title_dy;
    title->ScaleSize(title->GetWidth(), title->GetHeight()+title_dh);
  }
  else
  if(dt >= bfall_end) //Skins begins to fall when title ends falling
  {
    uint dt2 = dt - bfall_end;
    uint fall_duration = bfall_end - tfall_end;
    title->Scale(y_scale, y_scale);
    title_y = title_offset;
    title_y = (int)((dt2*dt2*(title->GetHeight()+title_offset)/fall_duration/fall_duration) - (title->GetHeight()+title_offset) + title_y);
  }
  else
  {
    title_y = -title->GetHeight();
  }
  //Draw the title at final position/scale +- delta
  uint title_x = app.video.GetWidth()/2 - title->GetWidth()/2;
  title->Blit(app.video.sdlwindow, title_x, title_y);
}

void Main_Menu::DrawButtons(int mx, int my, uint dt)
{
  //Calculate inital position of the buttons depending on windows size.
  double y_scale = (double)app.video.GetHeight() / DEFAULT_SCREEN_HEIGHT ;
  int x_button = app.video.GetWidth()/2 - button_width/2;

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

  if ( dt >= boscill_end )
  {
    // Finish the animation for buttons
    play->SetXY(x_button, y_play);
    play->SetSize(button_width, button_height);
#ifdef NETWORK_BUTTON  
    network->SetXY(x_button, y_network);
    network->SetSize(button_width, button_height);
#endif
    options->SetXY(x_button, y_options);
    options->SetSize(button_width, button_height);
    infos->SetXY(x_button, y_infos);
    infos->SetSize(button_width, button_height);
    quit->SetXY(x_button, y_quit);
    quit->SetSize(button_width, button_height);
  }
  else
  if ( dt >= bfall_end && dt <= boscill_end)
  {
    //Buttons finished falling, make them oscillate
    int button_dy, button_dh;
    Gelatine(button_dy, button_dh, start_time + bfall_end, dy - button_height, boscill_end - bfall_end, 2);
    play->SetSize(button_width, button_height + button_dh);
    play->SetXY(x_button, y_play + button_dy);
#ifdef NETWORK_BUTTON
    network->SetSize(button_width, button_height + button_dh);
    network->SetXY(x_button, y_network + button_dy);
#endif
    options->SetSize(button_width, button_height + button_dh);
    options->SetXY(x_button, y_options + button_dy);
    infos->SetSize(button_width, button_height + button_dh);
    infos->SetXY(x_button, y_infos + button_dy);
    quit->SetSize(button_width, button_height + button_dh);
    quit->SetXY(x_button, y_quit + button_dy);
  }
  else
  {
    uint fall_duration = bfall_end;
    //Buttons are falling
    play   ->SetXY(x_button, (dt*dt*app.video.GetHeight()/fall_duration/fall_duration) - app.video.GetHeight() + y_play);
#ifdef NETWORK_BUTTON
    network->SetXY(x_button, (dt*dt*app.video.GetHeight()/fall_duration/fall_duration) - app.video.GetHeight() + y_network);
#endif
    options->SetXY(x_button, (dt*dt*app.video.GetHeight()/fall_duration/fall_duration) - app.video.GetHeight() + y_options);
    infos  ->SetXY(x_button, (dt*dt*app.video.GetHeight()/fall_duration/fall_duration) - app.video.GetHeight() + y_infos);
    quit   ->SetXY(x_button, (dt*dt*app.video.GetHeight()/fall_duration/fall_duration) - app.video.GetHeight() + y_quit);
  }

  play->Draw(mx,my);
#ifdef NETWORK_BUTTON  
  network->Draw(mx,my);
#endif   
  options->Draw(mx,my);
  infos->Draw(mx,my);
  quit->Draw(mx,my);
}
