/******************************************************************************
 *  Wormux is a convivial mass murder game.
 *  Copyright (C) 2001-2008 Wormux Team.
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
 * Options menu
 *****************************************************************************/

#include "menu/options_menu.h"

#include <iostream>
#include "include/app.h"
#include "include/constant.h"
#include "game/game_mode.h"
#include "game/config.h"
#include "graphic/video.h"
#include "graphic/font.h"
#include "graphic/sprite.h"
#include "gui/button.h"
#include "gui/label.h"
#include "gui/box.h"
#include "gui/list_box.h"
#include "gui/combo_box.h"
#include "gui/check_box.h"
#include "gui/picture_widget.h"
#include "gui/picture_text_cbox.h"
#include "gui/spin_button_picture.h"
#include "gui/list_box_w_label.h"
#include "gui/question.h"

#include "map/maps_list.h"
#include "network/download.h"
#include "sound/jukebox.h"
#include "team/teams_list.h"
#include "tool/i18n.h"
#include "tool/string_tools.h"
#include "tool/resource_manager.h"
#include <sstream>

const uint SOUND_Y = 10;
const uint SOUND_W = 530;
const uint SOUND_H = 150;

const uint GRAPHIC_W = 530;
const uint GRAPHIC_H = 330;

OptionMenu::OptionMenu() :
  Menu("menu/bg_option")
{
  AppWormux * app = AppWormux::GetInstance();
  Config * config = Config::GetInstance();
  Profile *res = resource_manager.LoadXMLProfile("graphism.xml", false);
  Point2i stdSize(140, -1);
  Point2i option_size(140, 130);

  /* Graphic options */
  Box * graphic_options = new HBox(GRAPHIC_H);
  graphic_options->AddWidget(new PictureWidget(Point2i(40, 136), "menu/video_label"));

  Box * graphic_options_box = new GridBox(GRAPHIC_W - 40, option_size, false);

  // Various options
  opt_display_wind_particles = new PictureTextCBox(_("Wind particles?"), "menu/display_wind_particles", option_size);
  graphic_options_box->AddWidget(opt_display_wind_particles);

  opt_display_energy = new PictureTextCBox(_("Player energy?"), "menu/display_energy", option_size);
  graphic_options_box->AddWidget(opt_display_energy);

  opt_display_name = new PictureTextCBox(_("Player's name?"), "menu/display_name", option_size);
  graphic_options_box->AddWidget(opt_display_name);

  full_screen = new PictureTextCBox(_("Fullscreen?"), "menu/fullscreen", option_size);
  graphic_options_box->AddWidget(full_screen);

  opt_max_fps = new SpinButtonWithPicture(_("Maximum FPS"), "menu/fps",
					  option_size,
					  50, 5,
					  20, 50);

  graphic_options_box->AddWidget(opt_max_fps);


  // Get available video resolution
  const std::list<Point2i>& video_res = app->video->GetAvailableConfigs();
  std::list<Point2i>::const_iterator mode;
  std::vector<std::pair<std::string, std::string> > video_resolutions;
  std::string current_resolution;

  for (mode = video_res.begin(); mode != video_res.end(); ++mode) {
    std::ostringstream ss;
    std::string text;
    ss << mode->GetX() << "x" << mode->GetY() ;
    text = ss.str();
    if (app->video->window.GetWidth() == mode->GetX() && app->video->window.GetHeight() == mode->GetY())
      current_resolution = text;

    video_resolutions.push_back (std::pair<std::string, std::string>(text, text));
  }
  cbox_video_mode = new ComboBox(_("Resolution"), "menu/resolution", option_size,
                                 video_resolutions, current_resolution);
  graphic_options_box->AddWidget(cbox_video_mode);

  graphic_options->AddWidget(graphic_options_box);

  widgets.AddWidget(graphic_options);

  /* Language selection */
  Box * language_options = new HBox(GRAPHIC_H);
  language_options->AddWidget(new PictureWidget(Point2i(40, 136), "menu/config_label"));
  widgets.AddWidget(language_options);
  lbox_languages = new ListBoxWithLabel(_("Language"), stdSize);
  language_options->AddWidget(lbox_languages);

  /* Misc options */
  Box * misc_options = new HBox(SOUND_H);
  opt_updates = new PictureTextCBox(_("Check updates online?"),
                                    "menu/ico_update", option_size);
  misc_options->AddWidget(opt_updates);
  widgets.AddWidget(misc_options);

  /* Sound options */
  Box * sound_options = new HBox(SOUND_H);
  sound_options->AddWidget(new PictureWidget(Point2i(40, 138), "menu/audio_label"));

  Box * all_sound_options = new GridBox(SOUND_W, option_size, false);

  initial_vol_mus = config->GetVolumeMusic();
  volume_music = new SpinButtonWithPicture(_("Music volume"), "menu/music_enable",
					   option_size,
                                           fromVolume(initial_vol_mus), 5,
                                           0, 100);
  all_sound_options->AddWidget(volume_music);

  initial_vol_eff = config->GetVolumeEffects();
  volume_effects = new SpinButtonWithPicture(_("Effects volume"), "menu/sound_effects_enable",
					     option_size,
                                             fromVolume(initial_vol_eff), 5,
                                             0, 100);
  all_sound_options->AddWidget(volume_effects);

  // Generate sound mode list
  uint current_freq = JukeBox::GetInstance()->GetFrequency();
  std::vector<std::pair<std::string, std::string> > sound_freqs;
  std::string current_sound_freq;
  sound_freqs.push_back (std::pair<std::string, std::string> ("11025", "11 kHz"));
  sound_freqs.push_back (std::pair<std::string, std::string> ("22050", "22 kHz"));
  sound_freqs.push_back (std::pair<std::string, std::string> ("44100", "44 kHz"));

  if (current_freq == 44100)
    current_sound_freq = "44100";
  else if (current_freq == 22050)
    current_sound_freq = "22050";
  else
    current_sound_freq = "11025";

  cbox_sound_freq = new ComboBox(_("Sound frequency"), "menu/sound_frequency",
				 option_size, sound_freqs, current_sound_freq);
  all_sound_options->AddWidget(cbox_sound_freq);

  sound_options->AddWidget(all_sound_options);
  widgets.AddWidget(sound_options);

  /* Center the widgets */
  uint center_x = app->video->window.GetWidth()/2;

  sound_options->SetXY(center_x - (sound_options->GetSizeX() + misc_options->GetSizeX() + 20)/2, SOUND_Y);

  misc_options->SetXY(sound_options->GetPositionX() + sound_options->GetSizeX() + 10, SOUND_Y);

  language_options->SetXY(center_x - (graphic_options->GetSizeX() + language_options->GetSizeX() + 20)/2,
			  sound_options->GetPositionY() + sound_options->GetSizeY() + 10);

  graphic_options->SetXY(language_options->GetPositionX() + language_options->GetSizeX() + 10,
			 language_options->GetPositionY());

  // Values initialization
  opt_max_fps->SetValue(app->video->GetMaxFps());
  opt_display_wind_particles->SetValue(config->GetDisplayWindParticles());
  opt_display_energy->SetValue(config->GetDisplayEnergyCharacter());
  opt_display_name->SetValue(config->GetDisplayNameCharacter());
  full_screen->SetValue(app->video->IsFullScreen());

  // Setting language selection
  lbox_languages->AddItem(config->GetLanguage() == "",    _("(system language)"),  "");
  lbox_languages->AddItem(config->GetLanguage() == "bg",    "Български (bg)",      "bg");
  lbox_languages->AddItem(config->GetLanguage() == "bs",    "Bosanski",            "bs");
  lbox_languages->AddItem(config->GetLanguage() == "ca",    "Català",              "ca");
  lbox_languages->AddItem(config->GetLanguage() == "cpf",   "Créole",              "cpf");
  lbox_languages->AddItem(config->GetLanguage() == "cs",    "čeština (Czech)",
   "cs");
  lbox_languages->AddItem(config->GetLanguage() == "da",    "Dansk",               "da");
  lbox_languages->AddItem(config->GetLanguage() == "de",    "Deutsch",             "de");
  lbox_languages->AddItem(config->GetLanguage() == "eo",    "Esperanto",           "eo");
  lbox_languages->AddItem(config->GetLanguage() == "en",    "English",             "en");
  lbox_languages->AddItem(config->GetLanguage() == "es",    "Castellano",          "es");
  lbox_languages->AddItem(config->GetLanguage() == "fa",    "فارسی (Farsi)",       "fa");
  lbox_languages->AddItem(config->GetLanguage() == "fi",    "Suomi",               "fi");
  lbox_languages->AddItem(config->GetLanguage() == "fr",    "Français",            "fr");
  lbox_languages->AddItem(config->GetLanguage() == "gl",    "Galego",              "gl");
  lbox_languages->AddItem(config->GetLanguage() == "he",    "עברית (Hebrew)",      "he");
  lbox_languages->AddItem(config->GetLanguage() == "hu",    "Magyar",              "hu");
  lbox_languages->AddItem(config->GetLanguage() == "it",    "Italiano",            "it");
  lbox_languages->AddItem(config->GetLanguage() == "lv",    "latviešu valoda",     "lv");
  lbox_languages->AddItem(config->GetLanguage() == "nb",    "Norsk",               "nb");
  lbox_languages->AddItem(config->GetLanguage() == "nl",    "Nederlands",          "nl");
  lbox_languages->AddItem(config->GetLanguage() == "pl",    "Polski",              "pl");
  lbox_languages->AddItem(config->GetLanguage() == "pt",    "Português",           "pt");
  lbox_languages->AddItem(config->GetLanguage() == "pt_BR", "Português do Brasil", "pt_BR");
  lbox_languages->AddItem(config->GetLanguage() == "ro",    "Română",              "ro");
  lbox_languages->AddItem(config->GetLanguage() == "ru",    "Pусский язык (ru)",   "ru");
  lbox_languages->AddItem(config->GetLanguage() == "sk",    "Slovenčina",          "sk");
  lbox_languages->AddItem(config->GetLanguage() == "sl",    "Slovenščina",         "sl");
  lbox_languages->AddItem(config->GetLanguage() == "sv",    "Svenska",             "sv");
  lbox_languages->AddItem(config->GetLanguage() == "tr",    "Türkçe",              "tr");
  lbox_languages->AddItem(config->GetLanguage() == "zh_CN", "汉语 (hànyǔ)",        "zh_CN");
  lbox_languages->AddItem(config->GetLanguage() == "zh_TW", "闽语 (mǐnyǔ)",        "zh_TW");

  opt_updates->SetValue(config->GetCheckUpdates());

  resource_manager.UnLoadXMLProfile(res);
}

OptionMenu::~OptionMenu()
{
}

void OptionMenu::OnClickUp(const Point2i &mousePosition, int button)
{
  widgets.ClickUp(mousePosition, button);

  // Now that the click has been processed by the underlying widgets,
  // make use of their newer values in near-realtime!
  if (volume_music->Contains(mousePosition))
    Config::GetInstance()->SetVolumeMusic(toVolume(volume_music->GetValue()));
  else if (volume_effects->Contains(mousePosition))
    Config::GetInstance()->SetVolumeEffects(toVolume(volume_effects->GetValue()));
}

void OptionMenu::OnClick(const Point2i &/*mousePosition*/, int /*button*/)
{

}

void OptionMenu::SaveOptions()
{
  // Save values
  Config * config = Config::GetInstance();
  config->SetDisplayWindParticles(opt_display_wind_particles->GetValue());
  config->SetDisplayEnergyCharacter(opt_display_energy->GetValue());
  config->SetDisplayNameCharacter(opt_display_name->GetValue());

  // Misc options
  config->SetCheckUpdates(opt_updates->GetValue());

  // Sound settings - volume already saved
  config->SetSoundFrequency(cbox_sound_freq->GetIntValue());

  AppWormux * app = AppWormux::GetInstance();
  app->video->SetMaxFps(opt_max_fps->GetValue());
  // Video mode
  std::string s_mode = cbox_video_mode->GetValue();

  int w, h;
  sscanf(s_mode.c_str(),"%dx%d", &w, &h);
  app->video->SetConfig(w, h, full_screen->GetValue());

  uint x = app->video->window.GetWidth() / 2;
  uint y = app->video->window.GetHeight() - 50;

  SetActionButtonsXY(x, y);

  // Language
  std::string s_language = lbox_languages->ReadValue();
  config->SetLanguage(s_language);

  // Sound
  std::string sfreq = cbox_sound_freq->GetValue();
  long freq;
  if (str2long(sfreq,freq)) JukeBox::GetInstance()->SetFrequency(freq);

  JukeBox::GetInstance()->Init(); // commit modification on sound options

  //Save options in XML
  config->Save();
}

bool OptionMenu::signal_ok()
{
  SaveOptions();
  CheckUpdates();
  return true;
}

bool OptionMenu::signal_cancel()
{
  Config::GetInstance()->SetVolumeMusic(initial_vol_mus);
  Config::GetInstance()->SetVolumeEffects(initial_vol_eff);
  return true;
}

void OptionMenu::Draw(const Point2i &/*mousePosition*/)
{
}

void OptionMenu::CheckUpdates()
{
  if (!Config::GetInstance()->GetCheckUpdates() || Constants::WORMUX_VERSION.find("svn") != std::string::npos)
    return;

  try
  {
    std::string latest_version = Downloader::GetInstance()->GetLatestVersion();
    const char  *cur_version   = Constants::GetInstance()->WORMUX_VERSION.c_str();
    if (latest_version != cur_version)
    {
      Question new_version;
      std::string txt = Format(_("A new version %s is available, while your version is %s."
                                 "You may want to check whether an update is available for your OS!"),
                               latest_version.c_str(), cur_version);
      new_version.Set(txt, true, 0);
      new_version.Ask();
    }
  }
  catch (const char* err)
  {
    std::cerr << Format(_("Version verification failed because: %s\n"), err);
  }
}

uint OptionMenu::toVolume(uint level)
{
  return (level * Config::GetMaxVolume() + 50) / 100;
}

uint OptionMenu::fromVolume(uint vol)
{
  uint max = Config::GetMaxVolume();
  return (vol*100 + max/2) / max;
}
