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
 ******************************************************************************
 * Options menu
 *****************************************************************************/
#include <sstream>
#include <string>
#include <iostream>
#include <WORMUX_download.h>
#include "menu/options_menu.h"
#include "include/app.h"
#include "include/constant.h"
#include "game/game_mode.h"
#include "game/game.h"
#include "game/config.h"
#include "graphic/video.h"
#include "graphic/font.h"
#include "graphic/sprite.h"
#include "gui/button.h"
#include "gui/label.h"
#include "gui/box.h"
#include "gui/big/button_pic.h"
#include "gui/list_box.h"
#include "gui/combo_box.h"
#include "gui/check_box.h"
#include "gui/picture_widget.h"
#include "gui/picture_text_cbox.h"
#include "gui/spin_button_picture.h"
#include "gui/tabs.h"
#include "gui/text_box.h"
#include "gui/question.h"
#include "map/maps_list.h"
#include "map/wind.h"
#include "sound/jukebox.h"
#include "team/teams_list.h"
#include "team/custom_team.h"
#include "team/custom_teams_list.h"
#include "tool/string_tools.h"
#include "tool/resource_manager.h"

OptionMenu::OptionMenu() :
  Menu("menu/bg_option")
{
  AppWormux * app = AppWormux::GetInstance();
  Config * config = Config::GetInstance();
  Profile *res = GetResourceManager().LoadXMLProfile("graphism.xml", false);
  Point2i option_size(140, 130);

  uint max_width = app->video->window.GetWidth()-50;

  /* Tabs */
  MultiTabs * tabs = new MultiTabs(Point2i(max_width,
					   app->video->window.GetHeight()-100));
  tabs->SetPosition(25, 25);

  /* Graphic options */
  Box * graphic_options = new GridBox(max_width, option_size, false);

  // Various options
  opt_display_wind_particles = new PictureTextCBox(_("Wind particles?"), "menu/display_wind_particles", option_size);
  graphic_options->AddWidget(opt_display_wind_particles);

  opt_display_multisky = new PictureTextCBox(_("Multi-layer sky?"), "menu/multisky", option_size);
  graphic_options->AddWidget(opt_display_multisky);

  opt_display_energy = new PictureTextCBox(_("Player energy?"), "menu/display_energy", option_size);
  graphic_options->AddWidget(opt_display_energy);

  opt_display_name = new PictureTextCBox(_("Player's name?"), "menu/display_name", option_size);
  graphic_options->AddWidget(opt_display_name);

#ifndef __APPLE__
  full_screen = new PictureTextCBox(_("Fullscreen?"), "menu/fullscreen", option_size);
  graphic_options->AddWidget(full_screen);
#endif

  opt_max_fps = new SpinButtonWithPicture(_("Maximum FPS"), "menu/fps",
					  option_size,
					  50, 5,
					  20, 50);
  graphic_options->AddWidget(opt_max_fps);

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
  graphic_options->AddWidget(cbox_video_mode);

  tabs->AddNewTab("unused", _("Graphics"), graphic_options);

  /* Language selection */
  lbox_languages = new ListBox(option_size);
  tabs->AddNewTab("unused", _("Language"), lbox_languages);

  /* Team editor */

  // bug #12193 : Missed assertion in game option (custom team editor) while playing
  if (!Game::IsRunning()) {
    Box * teams_editor = new HBox(option_size.x, false, true);
    Box * teams_editor_inf = new VBox(max_width - option_size.x - 10, true, false);

    lbox_teams = new ListBox(option_size, false);
    teams_editor->AddWidget(lbox_teams);

    Box * box_team_name = new HBox(30, false, true);

    team_name = new Label(Format("%s:", _("Head commander")), 150, Font::FONT_SMALL, Font::FONT_BOLD);
    box_team_name->AddWidget(team_name);

    tbox_team_name = new TextBox("", 160, Font::FONT_SMALL, Font::FONT_BOLD);
    box_team_name->AddWidget(tbox_team_name);

    teams_editor_inf->AddWidget(box_team_name);

    Label* label_ch_names = new Label(_("Character names:"), 0, Font::FONT_SMALL, Font::FONT_BOLD);
    teams_editor_inf->AddWidget(label_ch_names);

    Point2i names_size(190, 20);

    Box * teams_editor_names = new GridBox(max_width - option_size.x - 40, names_size, false);
    for (uint i=0; i < 10; i++) {
      std::ostringstream oss;
      oss << i+1 << ":";
      tbox_character_name_list.push_back(new TextBox("",160,Font::FONT_SMALL, Font::FONT_BOLD));
      Label * lab = new Label(oss.str(), 30, Font::FONT_SMALL, Font::FONT_BOLD);

      Box * name_box = new HBox(20, false, true);
      name_box->SetNoBorder();

      name_box->AddWidget(lab);
      name_box->AddWidget(tbox_character_name_list[i]);

      teams_editor_names->AddWidget(name_box);
    }

    teams_editor_inf->AddWidget(teams_editor_names);

    Box * team_action_box = new HBox(35, false, true);
    team_action_box->SetNoBorder();

    add_team = new Button(res, "menu/add_custom_team");
    team_action_box->AddWidget(add_team);

    delete_team = new Button(res, "menu/del_custom_team");
    team_action_box->AddWidget(delete_team);

    teams_editor_inf->AddWidget(team_action_box);

    teams_editor_inf->Pack();
    teams_editor->AddWidget(teams_editor_inf);
    tabs->AddNewTab("unused", _("Teams editor"), teams_editor);
    selected_team = NULL;
    ReloadTeamList();
  } else {
    lbox_teams = NULL;
    add_team = NULL;
    delete_team = NULL;
    selected_team = NULL;
    tbox_team_name = NULL;
    team_name = NULL;
  }

  /* Misc options */
  Box * misc_options = new GridBox(max_width, option_size, false);
  opt_updates = new PictureTextCBox(_("Check updates online?"),
                                    "menu/ico_update", option_size);
  misc_options->AddWidget(opt_updates);

  opt_lefthanded_mouse = new PictureTextCBox(_("Left-handed mouse?"),
					     "menu/ico_lefthanded_mouse", option_size);
  misc_options->AddWidget(opt_lefthanded_mouse);

  opt_scroll_on_border = new PictureTextCBox(_("Scroll on border"), "menu/scroll_on_border", option_size);
  misc_options->AddWidget(opt_scroll_on_border);

  opt_scroll_border_size = new SpinButtonWithPicture(_("Scroll border size"), "menu/scroll_on_border",
						     option_size,
						     50, 5, 5, 80);
  misc_options->AddWidget(opt_scroll_border_size);



  tabs->AddNewTab("unused", _("Misc"), misc_options);


  /* Sound options */
  Box * sound_options = new GridBox(max_width, option_size, false);

  music_cbox = new PictureTextCBox(_("Music?"), "menu/music_enable", option_size);
  sound_options->AddWidget(music_cbox);

  initial_vol_mus = config->GetVolumeMusic();
  volume_music = new SpinButtonWithPicture(_("Music volume"), "menu/music_enable",
					   option_size,
                                           fromVolume(initial_vol_mus), 5,
                                           0, 100);
  sound_options->AddWidget(volume_music);

  effects_cbox = new PictureTextCBox(_("Sound effects?"), "menu/sound_effects_enable", option_size);
  sound_options->AddWidget(effects_cbox);

  initial_vol_eff = config->GetVolumeEffects();
  volume_effects = new SpinButtonWithPicture(_("Effects volume"), "menu/sound_effects_enable",
					     option_size,
                                             fromVolume(initial_vol_eff), 5,
                                             0, 100);
  sound_options->AddWidget(volume_effects);

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
  sound_options->AddWidget(cbox_sound_freq);

  warn_cbox = new PictureTextCBox(_("New player warning?"), "menu/warn_on_new_player", option_size);
  sound_options->AddWidget(warn_cbox);

  tabs->AddNewTab("unused", _("Sound"), sound_options);

  // Values initialization
  opt_max_fps->SetValue(app->video->GetMaxFps());
  opt_display_wind_particles->SetValue(config->GetDisplayWindParticles());
  opt_display_multisky->SetValue(config->GetDisplayMultiLayerSky());
  opt_display_energy->SetValue(config->GetDisplayEnergyCharacter());
  opt_display_name->SetValue(config->GetDisplayNameCharacter());
#ifndef __APPLE__
  full_screen->SetValue(app->video->IsFullScreen());
#endif
  music_cbox->SetValue(config->GetSoundMusic());
  effects_cbox->SetValue(config->GetSoundEffects());
  warn_cbox->SetValue(config->GetWarnOnNewPlayer());

  // Setting language selection
  lbox_languages->AddItem(config->GetLanguage() == "",    _("(system language)"),  "");
  lbox_languages->AddItem(config->GetLanguage() == "bg",    "Български (bg)",      "bg");
  lbox_languages->AddItem(config->GetLanguage() == "bs",    "Bosanski",            "bs");
  lbox_languages->AddItem(config->GetLanguage() == "es",    "Castellano",          "es");
  lbox_languages->AddItem(config->GetLanguage() == "ca",    "Català",              "ca");
  lbox_languages->AddItem(config->GetLanguage() == "cs",    "čeština (Czech)",     "cs");
  lbox_languages->AddItem(config->GetLanguage() == "cpf",   "Créole",              "cpf");
  lbox_languages->AddItem(config->GetLanguage() == "da",    "Dansk",               "da");
  lbox_languages->AddItem(config->GetLanguage() == "de",    "Deutsch",             "de");
  lbox_languages->AddItem(config->GetLanguage() == "eo",    "Esperanto",           "eo");
  lbox_languages->AddItem(config->GetLanguage() == "en",    "English",             "en");
  lbox_languages->AddItem(config->GetLanguage() == "el",    "Ελληνικά",            "el");
  lbox_languages->AddItem(config->GetLanguage() == "et",    "Eesti keel",          "et");
  lbox_languages->AddItem(config->GetLanguage() == "fa",    "فارسی (Farsi)",       "fa");
  lbox_languages->AddItem(config->GetLanguage() == "fr",    "Français",            "fr");
  lbox_languages->AddItem(config->GetLanguage() == "gl",    "Galego",              "gl");
  lbox_languages->AddItem(config->GetLanguage() == "he",    "עברית (Hebrew)",      "he");
  lbox_languages->AddItem(config->GetLanguage() == "hu",    "Magyar",              "hu");
  lbox_languages->AddItem(config->GetLanguage() == "it",    "Italiano",            "it");
  lbox_languages->AddItem(config->GetLanguage() == "ja_JP", "日本語 (japanese)",   "ja_JP");
  lbox_languages->AddItem(config->GetLanguage() == "kw",    "Kernewek",            "kw");
  lbox_languages->AddItem(config->GetLanguage() == "lv",    "latviešu valoda",     "lv");
  lbox_languages->AddItem(config->GetLanguage() == "nb",    "Norsk (bokmål)",      "nb");
  lbox_languages->AddItem(config->GetLanguage() == "nn",    "Norsk (nynorsk)",     "nn");
  lbox_languages->AddItem(config->GetLanguage() == "nl",    "Nederlands",          "nl");
  lbox_languages->AddItem(config->GetLanguage() == "pl",    "Polski",              "pl");
  lbox_languages->AddItem(config->GetLanguage() == "pt",    "Português",           "pt");
  lbox_languages->AddItem(config->GetLanguage() == "pt_BR", "Português do Brasil", "pt_BR");
  lbox_languages->AddItem(config->GetLanguage() == "ro",    "Română",              "ro");
  lbox_languages->AddItem(config->GetLanguage() == "ru",    "Pусский язык (ru)",   "ru");
  lbox_languages->AddItem(config->GetLanguage() == "sk",    "Slovenčina",          "sk");
  lbox_languages->AddItem(config->GetLanguage() == "sl",    "Slovenščina",         "sl");
  lbox_languages->AddItem(config->GetLanguage() == "fi",    "Suomi",               "fi");
  lbox_languages->AddItem(config->GetLanguage() == "sv",    "Svenska",             "sv");
  lbox_languages->AddItem(config->GetLanguage() == "tr",    "Türkçe",              "tr");
  lbox_languages->AddItem(config->GetLanguage() == "ua",    "украї́нська мо́ва",     "ua");
  lbox_languages->AddItem(config->GetLanguage() == "zh_CN", "汉语 (hànyǔ)",        "zh_CN");
  lbox_languages->AddItem(config->GetLanguage() == "zh_TW", "闽语 (mǐnyǔ)",              "zh_TW");

  opt_updates->SetValue(config->GetCheckUpdates());
  opt_lefthanded_mouse->SetValue(config->GetLeftHandedMouse());
  opt_scroll_on_border->SetValue(config->GetScrollOnBorder());
  opt_scroll_border_size->SetValue(config->GetScrollBorderSize());

  GetResourceManager().UnLoadXMLProfile(res);

  widgets.AddWidget(tabs);
  widgets.Pack();

}

OptionMenu::~OptionMenu()
{

}

void OptionMenu::OnClickUp(const Point2i &mousePosition, int button)
{
  Widget* w = widgets.ClickUp(mousePosition, button);

  // Now that the click has been processed by the underlying widgets,
  // make use of their newer values in near-realtime!
  if (w == volume_music)
    Config::GetInstance()->SetVolumeMusic(toVolume(volume_music->GetValue()));
  else if (w == volume_effects) {
    Config::GetInstance()->SetVolumeEffects(toVolume(volume_effects->GetValue()));
    JukeBox::GetInstance()->Play("default", "menu/clic");
  }
  else if (w == music_cbox) {
    JukeBox::GetInstance()->ActiveMusic(music_cbox->GetValue());
  }
  else if (w == effects_cbox) {
    JukeBox::GetInstance()->ActiveEffects(effects_cbox->GetValue());
  }
  else if (w == lbox_teams) {
    SelectTeam();
  }
  else if (w == add_team) {
    AddTeam();
  }
  else if (w == delete_team) {
    DeleteTeam();
  }
}

void OptionMenu::OnClick(const Point2i &mousePosition, int button)
{
  widgets.Click(mousePosition, button);
}

void OptionMenu::SaveOptions()
{

  Config * config = Config::GetInstance();

  // Graphic options
  config->SetDisplayWindParticles(opt_display_wind_particles->GetValue());
  // bug #11826 : Segmentation fault while exiting the menu.
  if (Game::IsRunning())
    Wind::GetRef().Reset();

  config->SetDisplayMultiLayerSky(opt_display_multisky->GetValue());
  config->SetDisplayEnergyCharacter(opt_display_energy->GetValue());
  config->SetDisplayNameCharacter(opt_display_name->GetValue());
  config->SetScrollOnBorder(opt_scroll_on_border->GetValue());
  config->SetScrollBorderSize(opt_scroll_border_size->GetValue());

  // Misc options
  config->SetCheckUpdates(opt_updates->GetValue());
  config->SetLeftHandedMouse(opt_lefthanded_mouse->GetValue());

  // Sound settings - volume already saved
  config->SetSoundFrequency(cbox_sound_freq->GetIntValue());
  config->SetSoundMusic(music_cbox->GetValue());
  config->SetSoundEffects(effects_cbox->GetValue());

  AppWormux * app = AppWormux::GetInstance();
  app->video->SetMaxFps(opt_max_fps->GetValue());
  // Video mode
  std::string s_mode = cbox_video_mode->GetValue();

  int w, h;
  sscanf(s_mode.c_str(),"%dx%d", &w, &h);
#ifdef __APPLE__
  // The mac version of SDL does not support fullscreen properly
  app->video->SetConfig(w, h, false);
#else
  app->video->SetConfig(w, h, full_screen->GetValue());
#endif

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
  config->SetWarnOnNewPlayer(warn_cbox->GetValue());

  JukeBox::GetInstance()->Init(); // commit modification on sound options

  //Save options in XML
  config->Save();

  //Team editor
  if (!Game::IsRunning()) {
    if (!lbox_teams->IsSelectedItem()) {
      AddTeam();
    }
    SaveTeam();
  }
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
  if (!Config::GetInstance()->GetCheckUpdates()
      || Constants::WORMUX_VERSION.find("svn") != std::string::npos)
    return;

  try {
      std::string latest_version = Downloader::GetInstance()->GetLatestVersion();
      const char  *cur_version   = Constants::GetInstance()->WORMUX_VERSION.c_str();
      if (latest_version != cur_version) {
	Question new_version;
	std::string txt = Format(_("A new version %s is available, while your version is %s. "
				   "You may want to check whether an update is available for your OS!"),
				 latest_version.c_str(), cur_version);
	new_version.Set(txt, true, 0);
	new_version.Ask();
      }
  }
  catch (const std::string err) {
    AppWormux::DisplayError(Format(_("Version verification failed because: %s"), err.c_str()));
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


// Team editor function
bool OptionMenu::TeamInfoValid()
{
  if (tbox_team_name->GetText().empty())
    return false;

  for (uint i = 0; i < tbox_character_name_list.size(); i++) {
    if (tbox_character_name_list[i]->GetText().empty()) {
      return false;
    }
  }

  return true;
}

void OptionMenu::AddTeam()
{
  if (Game::IsRunning())
    return;

  if (!TeamInfoValid())
    return;

  CustomTeam *new_team = new CustomTeam(tbox_team_name->GetText());
  new_team->NewTeam();

  selected_team = new_team;
  SaveTeam();

  ReloadTeamList();
  lbox_teams->NeedRedrawing();
}

void OptionMenu::DeleteTeam()
{
  if (Game::IsRunning())
    return;

  if (selected_team) {
    selected_team->Delete();
    selected_team = NULL;
    if (lbox_teams->IsSelectedItem()) {
      lbox_teams->Deselect();
    }
    ReloadTeamList();
    LoadTeam();
    lbox_teams->NeedRedrawing();
  }
}

void OptionMenu::LoadTeam()
{
  if (Game::IsRunning())
    return;

  if (selected_team) {
    tbox_team_name->SetText(selected_team->GetName());
    std::vector<std::string> character_names = selected_team->GetCharactersNameList();

    for (uint i = 0; i < character_names.size() && i < tbox_character_name_list.size(); i++) {
      tbox_character_name_list[i]->SetText(character_names[i]);
    }

  } else {
    tbox_team_name->SetText("");

    for (uint i = 0; i < tbox_character_name_list.size(); i++) {
      tbox_character_name_list[i]->SetText("");
    }
  }
}

void OptionMenu::ReloadTeamList()
{
  if (Game::IsRunning())
    return;

  lbox_teams->ClearItems();
  std::string selected_team_name ="";
  if (selected_team) {
    selected_team_name = selected_team->GetName();
  }

  GetCustomTeamsList().LoadList();
  std::vector<CustomTeam *> custom_team_list = GetCustomTeamsList().GetList();

  for (uint i = 0; i < custom_team_list.size(); i++) {
    if (custom_team_list[i]->GetName() == selected_team_name) {
      selected_team = custom_team_list[i];
      LoadTeam();
    }

    lbox_teams->AddItem((selected_team == custom_team_list[i]),
			custom_team_list[i]->GetName(),
			custom_team_list[i]->GetName());

  }
}

bool OptionMenu::SaveTeam()
{
  if (Game::IsRunning())
    return false;

  if (!TeamInfoValid())
    return false;

  if (selected_team) {
    bool is_name_changed = (selected_team->GetName().compare(tbox_team_name->GetText()) != 0);
    selected_team->SetName(tbox_team_name->GetText());
    for (uint i = 0; i < tbox_character_name_list.size(); i++) {
      selected_team->SetCharacterName(i,tbox_character_name_list[i]->GetText());
    }
    selected_team->Save();
    return is_name_changed;
  }

  return false;
}

void OptionMenu::SelectTeam()
{
  if (Game::IsRunning())
    return;

  if (lbox_teams->IsSelectedItem()) {
    bool is_changed_name = SaveTeam();
    std::string s_selected_team = lbox_teams->ReadValue();
    selected_team = GetCustomTeamsList().GetByName(s_selected_team);
    LoadTeam();
    if (is_changed_name) {
      ReloadTeamList();
    }
  }
}
