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
 * Moteur de son
 *****************************************************************************/

#ifndef JUKEBOX_H
#define JUKEBOX_H
//-----------------------------------------------------------------------------
#include <ClanLib/vorbis.h>
#include <ClanLib/sound.h>
#include <ClanLib/core.h>
#include <map>
#include "../include/base.h"
//-----------------------------------------------------------------------------

class JukeBox 
{
private:
  // Define types
  typedef struct s_error_displayed{
    CL_SoundBuffer* buffer;
    CL_SoundBuffer_Session* session;
    bool error_displayed;
  } sound_item_t;
  typedef std::multimap<std::string, sound_item_t>::value_type 
    paire_soundbuffer;
  typedef std::multimap<std::string, sound_item_t>::iterator 
    iterator;

  std::multimap<std::string, sound_item_t> m_soundbuffers;
  struct s_m_config{
    bool use;
    bool music;
    bool effects;
    uint frequency;
  } m_config;
  bool m_init;
  bool m_init_res;
  CL_ResourceManager * res; 
  
  CL_SoundOutput * soundoutput;
  CL_SetupCore * coreinit;
  CL_SetupSound * soundinit;
  CL_SetupVorbis * vorbisinit;

  std::set<std::string> m_profiles_loaded;

public:
  JukeBox();

  void ActiveSound (bool on);
  void ActiveMusic (bool on);
  void ActiveEffects (bool on);
  void SetFrequency (uint frequency);
  bool UseSound() const;
  bool UseMusic() const;
  bool UseEffects() const;
  bool GetMusicConfig() const;
  bool GetEffectsConfig() const;
  uint GetFrequency() const;

  void Init();
  void End(); 
  void StopAll();
  void Play(const std::string& soundfile, 
	    const bool loop = false,
	    CL_SoundBuffer_Session **session = NULL);
  void Stop(const std::string& soundfile);

  void Load(const std::string& profile);
  void PlayProfile(const std::string& profile, 
	    const std::string& action, 
	    const bool loop = false,
	    CL_SoundBuffer_Session **session = NULL);
  
private:
  void PlayItem (sound_item_t &item, const bool loop, 
		 CL_SoundBuffer_Session **session);
};

extern JukeBox jukebox;
//-----------------------------------------------------------------------------
#endif
