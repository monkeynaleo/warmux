/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2010 Wormux Team.
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

#include <vector>
#include <sstream>

#include "gui/combo_box.h"
#include "graphic/text.h"
#include "graphic/video.h"
#include "gui/button.h"
#include "include/app.h"
#include "tool/math_tools.h"
#include "tool/resource_manager.h"
#include "graphic/polygon_generator.h"
#include "tool/affine_transform.h"

ComboBox::ComboBox (const std::string &label,
                    const std::string &resource_id,
                    const Point2i &_size,
                    const std::vector<std::pair<std::string, std::string> > &choices,
                    const std::string choice)
  : m_choices(choices)
  , m_index(0)
{
  position = Point2i(-1, -1);
  size = _size;

  Profile *res = GetResourceManager().LoadXMLProfile("graphism.xml", false);
  m_image = GetResourceManager().LoadImage(res, resource_id);
  m_annulus_background = GetResourceManager().LoadImage(res, "menu/annulus_background", true);
  m_annulus_foreground = GetResourceManager().LoadImage(res, "menu/annulus_foreground", true);
  m_progress_color = GetResourceManager().LoadColor(res, "menu/annulus_progress_color");
  GetResourceManager().UnLoadXMLProfile(res);

  txt_label = new Text(label, dark_gray_color, Font::FONT_SMALL, Font::FONT_BOLD, false);
  txt_label->SetMaxWidth(GetSizeX());

  txt_value_black = new Text("", black_color, Font::FONT_MEDIUM, Font::FONT_BOLD, false);
  txt_value_white = new Text("", white_color, Font::FONT_MEDIUM, Font::FONT_BOLD, false);

  std::vector<std::string>::size_type index = 0;
  for (std::vector<std::pair<std::string, std::string> >::const_iterator iter
       = choices.begin ();
       iter != choices.end ();
       iter++) {

    if (iter->first == choice)
      m_index = index;
    index++;
  }

  SetChoice(m_index);
}

ComboBox::~ComboBox ()
{
  delete txt_label;
  delete txt_value_black;
  delete txt_value_white;
}

void ComboBox::Pack()
{
  txt_label->SetMaxWidth(size.x);
}

void ComboBox::Draw(const Point2i &/*mousePosition*/) const
{
  Surface& video_window = GetMainWindow();

  //  the computed positions are to center on the image part of the widget

  // 1. first draw the annulus background
  Point2i tmp(GetPositionX() + (GetSizeX() - m_annulus_background.GetWidth())/2,
              GetPositionY());
  video_window.Blit(m_annulus_background, tmp);

  // 2. then draw the progress annulus
  #define SMALL_R 25
  #define BIG_R   35
  #define OPEN_ANGLE 0.96f // 55
  Point2i center = tmp + m_annulus_background.GetSize()/2;
  float angle;
  if (m_choices.size () > 1)
    angle = m_index*(2*M_PI - OPEN_ANGLE) / (m_choices.size () - 1);
  else
    angle = 0;
  Polygon *ptmp = PolygonGenerator::GeneratePartialTorus(BIG_R*2, SMALL_R*2, 100, angle, OPEN_ANGLE/2);
  ptmp->SetPlaneColor(m_progress_color);
  ptmp->ApplyTransformation(AffineTransform2D::Translate(center.x, center.y));
  ptmp->Draw(&video_window);
  delete(ptmp);

  // 3. then draw the annulus foreground
  video_window.Blit(m_annulus_foreground, tmp);

  // 4. then draw the image
  video_window.Blit(m_image, center - m_image.GetSize()/2);

  // 5. add in the value image
  uint tmp_x = center.x;
  uint tmp_y = center.y + SMALL_R - 3;
  uint value_h = Font::GetInstance(Font::FONT_MEDIUM)->GetHeight();

  txt_value_black->DrawCenterTop(Point2i(tmp_x + 1, tmp_y + 1 - value_h/2));
  txt_value_white->DrawCenterTop(Point2i(tmp_x, tmp_y - value_h/2));

  // 6. and finally the label image
  txt_label->DrawCenterTop(Point2i(GetPositionX() + GetSizeX()/2,
                            GetPositionY() + GetSizeY() - txt_label->GetHeight()));
}

Widget* ComboBox::ClickUp(const Point2i &mousePosition, uint button)
{
  NeedRedrawing();

  bool is_click = Mouse::IS_CLICK_BUTTON(button);
  if ( (is_click && mousePosition.x+5 > (GetPositionX() + GetSizeX()/2))
       || button == SDL_BUTTON_WHEELUP ) {
    SetChoice(m_index + 1);
    return this;
  } else if ( (is_click && mousePosition.x+5 <= (GetPositionX() + GetSizeX()/2))
              || button == SDL_BUTTON_WHEELDOWN ) {
    SetChoice(m_index - 1);
    return this;
  }

  return NULL;
}

void ComboBox::SetChoice (std::vector<std::string>::size_type index)
{
  std::string text;

  if (index >= m_choices.size ())
    return; /* index = 0; // loop back */

  m_index = index;

  txt_value_black->SetText(m_choices[m_index].second);
  txt_value_white->SetText(m_choices[m_index].second);

  NeedRedrawing();
}

int ComboBox::GetIntValue() const
{
  int tmp = 0;
  sscanf(GetValue().c_str(),"%d", &tmp);
  return tmp;
}
