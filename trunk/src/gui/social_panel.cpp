/******************************************************************************
 *  Warmux is a convivial mass murder game.
 *  Copyright (C) 2001-2011 Warmux Team.
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
 * Panel of widgets to enter credentials for several websites
 *****************************************************************************/

#include <string>

#include "game/config.h"
#include "gui/label.h"
#include "gui/check_box.h"
#include "gui/horizontal_box.h"
#include "gui/social_panel.h"
#include "gui/text_box.h"
#include "gui/vertical_box.h"

class SocialWidget : public VBox
{
  TextBox*     UserBox;
  CheckBox*    SaveBox;
  HBox*        HideBox;
  PasswordBox* PwdBox;
  bool         use_save;

public:
  SocialWidget(const std::string& name, int ssize, float factor, bool use_save,
               const std::string& user, const std::string& pwd, bool saved,
               Font::font_size_t fbig, Font::font_size_t fmedium)
    : VBox(ssize, false, false)
  {
    // Default look
    SetBorder(5*factor);
    SetBackgroundColor(transparent_color);

    // Name
    AddWidget(new Label(name, 20, fbig, Font::FONT_BOLD, c_red));

    // Email
    HBox *hbox = new HBox(30*factor, false); hbox->SetNoBorder(); hbox->SetBackgroundColor(transparent_color);
    hbox->AddWidget(new Label(_("User"), ssize/3-5*factor, fmedium));
    UserBox = new TextBox(user, (2*ssize)/3-5*factor, fmedium);
    hbox->AddWidget(UserBox);
    AddWidget(hbox);

    // Password
    SaveBox = new CheckBox(_("Save password"), ssize - 10*factor, saved, fmedium);
    SaveBox->SetBackgroundColor(transparent_color);
    SaveBox->SetValue(saved);
    AddWidget(SaveBox);
    HideBox = new HBox(30*factor, false);
    HideBox->SetNoBorder(); HideBox->SetBackgroundColor(transparent_color);
    HideBox->AddWidget(new Label(_("Password"), ssize/3-5*factor, fmedium));
    PwdBox = new PasswordBox(pwd, (2*ssize)/3-5*factor, fmedium);
    HideBox->AddWidget(PwdBox);
    if (use_save) HideBox->SetVisible(saved);
    AddWidget(HideBox);
  }

  virtual Widget * ClickUp(const Point2i & mousePosition, uint button)
  {
    Widget *w = VBox::ClickUp(mousePosition, button);
    if (use_save && w == SaveBox) {
      HideBox->SetVisible(SaveBox->GetValue());
    }
    return w;
  }

  bool IsSaved() const { return SaveBox->GetValue(); }
  const std::string& GetUser() const { return UserBox->GetText(); }
  const std::string& GetPassword() const { return PwdBox->GetPassword(); }
};

SocialPanel::SocialPanel(int width, float factor, bool s)
  : VBox(width, false)
  , save(s)
{
  std::string user, pwd;
  Font::font_size_t fbig = Font::GetFixedSize(Font::FONT_BIG*factor+0.5f);
  Font::font_size_t fmed = Font::GetFixedSize(Font::FONT_MEDIUM*factor+0.5f);
  SetBackgroundColor(transparent_color);
  width -= 10*factor;
#ifdef HAVE_FACEBOOK
  Config::GetInstance()->GetFaceBookCreds(user, pwd);
  facebook = new SocialWidget("Facebook", width, factor, s,
                              user, pwd, Config::GetInstance()->GetFaceBookSave(),
                              fbig, fmed);
  AddWidget(facebook);
#endif
#ifdef HAVE_TWITTER
  Config::GetInstance()->GetTwitterCreds(user, pwd);
  twitter = new SocialWidget("Twitter", width, factor, s,
                             user, pwd, Config::GetInstance()->GetTwitterSave(),
                             fbig, fmed);
  AddWidget(twitter);
#endif
}

void SocialPanel::Close()
{
  if (save) {
    Config* config = Config::GetInstance();
#ifdef HAVE_FACEBOOK
    config->SetFaceBookSave(facebook->IsSaved());
    config->SetFaceBookCreds(facebook->GetUser(), facebook->GetPassword());
#endif
#ifdef HAVE_TWITTER
    config->SetTwitterSave(twitter->IsSaved());
    config->SetTwitterCreds(twitter->GetUser(), twitter->GetPassword());
#endif
  }
}
