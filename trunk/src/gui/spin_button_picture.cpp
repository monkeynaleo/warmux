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
 *****************************************************************************/

#include "spin_button_picture.h"
#include "graphic/text.h"
#include "graphic/video.h"
#include "button.h"
#include <sstream>
#include "include/app.h"
#include "tool/math_tools.h"
#include "tool/resource_manager.h"

SpinButtonWithPicture::SpinButtonWithPicture (const std::string &label, const std::string &resource_id,
					      const Rectanglei &rect,
					      int value, int step, int min_value, int max_value)
{
  position =  rect.GetPosition();
  size = rect.GetSize();

  Profile *res = resource_manager.LoadXMLProfile( "graphism.xml", false); 
  m_image = resource_manager.LoadImage(res, resource_id);
  m_annulus_background = resource_manager.LoadImage(res, "menu/annulus_background");
  m_annulus_foreground = resource_manager.LoadImage(res, "menu/annulus_foreground");
  m_progress_color = resource_manager.LoadColor(res, "menu/annulus_progress_color");
  resource_manager.UnLoadXMLProfile( res); 

  txt_label = new Text(label, dark_gray_color, Font::FONT_MEDIUM, Font::FONT_BOLD, false);
  txt_label->SetMaxWidth(GetSizeX());

  if ( min_value != -1 && min_value <= value)
    m_min_value = min_value;
  else m_min_value = value/2;

  if ( max_value != -1 && max_value >= value)
    m_max_value = max_value;
  else m_max_value = value*2;

  txt_value = new Text("", white_color, Font::FONT_MEDIUM, Font::FONT_BOLD, false);
  SetValue(value);

  m_step = step;
}

SpinButtonWithPicture::~SpinButtonWithPicture ()
{
  delete txt_label;
  delete txt_value;
}

void SpinButtonWithPicture::SetSizePosition(const Rectanglei &rect)
{
  StdSetSizePosition(rect);
  txt_label->SetMaxWidth(GetSizeX());
}

void SpinButtonWithPicture::Draw(const Point2i &/*mousePosition*/, Surface& /*surf*/) const
{
  Surface video_window = AppWormux::GetInstance()->video->window;

  //  the computed positions are to center on the image part of the widget

  // 1. first draw the annulus background
  uint tmp_back_x = GetPositionX() + (GetSizeX() - m_annulus_background.GetWidth())/4 ;
  uint tmp_back_y = GetPositionY() + (GetSizeY() - m_annulus_background.GetHeight() - txt_label->GetHeight() - 5) /2;
  video_window.Blit(m_annulus_background, Point2i(tmp_back_x, tmp_back_y));

  // 2. then draw the progress annulus
  static uint small_r = 25;
  static uint big_r = 35;
  static double min_angle = -2.65;
  static double max_angle = 2.6;
  static double delta_angle = M_PI/100; // magic... ajust if not good
  double angle = (max_angle - min_angle) * (m_value - m_min_value) / (m_max_value - m_min_value);
  uint center_x = tmp_back_x + m_annulus_background.GetWidth() / 2;
  uint center_y = tmp_back_y + m_annulus_background.GetHeight() / 2;
  long num = static_cast<long> (angle / delta_angle) + 1;
  std::list<Point2i> points;

  for (long ii = 0; ii <= num ; ii++)
    points.push_back (Point2i (static_cast<int>(center_x + big_r * sin (min_angle + ii * delta_angle)),
			       static_cast<int>(center_y - big_r * cos (min_angle + ii * delta_angle))));

  for (long ii = num; ii >= 0; ii--)
    points.push_back (Point2i (static_cast<int>(center_x + small_r * sin (min_angle + ii * delta_angle)),
			       static_cast<int>(center_y - small_r * cos (min_angle + ii * delta_angle))));

  video_window.FilledPolygon (points, m_progress_color);

  // 3. then draw the annulus foreground
  uint tmp_fore_x = GetPositionX() + (GetSizeX() - m_annulus_foreground.GetWidth())/4 ;
  uint tmp_fore_y = GetPositionY() + (GetSizeY() - m_annulus_foreground.GetHeight() - txt_label->GetHeight() - 5) /2;
  video_window.Blit(m_annulus_foreground, Point2i(tmp_fore_x, tmp_fore_y));

  // 4. then draw the image
  uint tmp_x = center_x - m_image.GetWidth() / 2;
  uint tmp_y = center_y - m_image.GetHeight() / 2;

  video_window.Blit(m_image, Point2i(tmp_x, tmp_y));

  // 5. add in the value image
  tmp_x = center_x;
  tmp_y = center_y + small_r - 3;
  uint value_h = Font::GetInstance(Font::FONT_MEDIUM)->GetHeight();

  txt_value->DrawCenterTop(tmp_x, tmp_y - value_h/2);

  // 6. and finally the label image
  txt_label->DrawCenterTop( GetPositionX() + GetSizeX()/2, 
			    GetPositionY() + GetSizeY() - txt_label->GetHeight() );
}

Widget* SpinButtonWithPicture::ClickUp(const Point2i &mousePosition, uint button)
{
  need_redrawing = true;

  if (button == SDL_BUTTON_LEFT && Contains(mousePosition)) {  
    
    m_value += m_step;
    if (m_value > m_max_value) SetValue(m_min_value);
    else SetValue(m_value);

  } else if (button == SDL_BUTTON_RIGHT && Contains(mousePosition)) {  
    
    m_value -= m_step;
    if (m_value < m_min_value) SetValue(m_max_value);
    else SetValue(m_value);

  } else if( button == SDL_BUTTON_WHEELDOWN && Contains(mousePosition) ) {
    
    SetValue(m_value - m_step);
    return this;
    
  } else if( button == SDL_BUTTON_WHEELUP && Contains(mousePosition) ) {
    
    SetValue(m_value + m_step);
    return this;
  }
  return NULL;
}

Widget* SpinButtonWithPicture::Click(const Point2i &/*mousePosition*/, uint /*button*/)
{
  return NULL;
}

int SpinButtonWithPicture::GetValue() const
{
  return m_value;
}

void SpinButtonWithPicture::SetValue(int value)  
{
  m_value = BorneLong(value, m_min_value, m_max_value);  

  std::ostringstream value_s;
  value_s << m_value ;

  std::string s(value_s.str());
  txt_value->Set(s);

  ForceRedraw();
}
