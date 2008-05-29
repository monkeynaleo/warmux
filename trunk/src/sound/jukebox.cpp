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
 * Sound engine
 *****************************************************************************/

#include "sound/jukebox.h"

#include <iostream>
#include <fstream>

#include "game/config.h"
#include "tool/debug.h"
#include "tool/i18n.h"
#include "tool/random.h"
#include "tool/file_tools.h"
#include "tool/xml_document.h"

JukeBox::JukeBox()
  : music(NULL)
  , m_init(false)
  , m_cache(0) // Defaults to unlimited cache
{
  m_config.music = true;
  m_config.effects = true;
  m_config.frequency = 44100; //MIX_DEFAULT_FREQUENCY;
  m_config.channels = 2; // stereo
}

void JukeBox::Pause() const
{
  if (m_config.music || m_config.effects)
    Mix_Pause(-1);
}

void JukeBox::Resume() const
{
  if (m_config.music || m_config.effects)
    Mix_Resume(-1);
}

void JukeBox::Init()
{
  JukeBox::GetInstance()->ActiveMusic(Config::GetInstance()->GetSoundMusic());
  JukeBox::GetInstance()->ActiveEffects(Config::GetInstance()->GetSoundEffects());
  JukeBox::GetInstance()->SetFrequency(Config::GetInstance()->GetSoundFrequency());

  if (!m_config.music && !m_config.effects) {
    End();
    return;
  }

  if (m_init) return;

  Uint16 audio_format = MIX_DEFAULT_FORMAT;

  /* Initialize the SDL library */
  if ( SDL_Init(SDL_INIT_AUDIO) < 0 ) {
    std::cerr << "* Couldn't initialize SDL: "<< SDL_GetError() << std::endl;
    return;
  }

  int audio_buffer = 1024;

  /* Open the audio device */
  if (Mix_OpenAudio(m_config.frequency, audio_format, m_config.channels, audio_buffer) < 0)
  {
    std::cerr << "* Couldn't open audio: " <<  SDL_GetError() << std::endl;
    return;
  }
  else
  {
    Mix_QuerySpec(&m_config.frequency, &audio_format, &m_config.channels);
    std::cout << Format(_("o Opened audio at %d Hz %d bit"),
                        m_config.frequency, (audio_format&0xFF)) << std::endl;
  }
  Mix_ChannelFinished(JukeBox::EndChunk);
  Mix_HookMusicFinished(JukeBox::EndMusic);

  m_init = true;

  LoadXML("share");
  LoadMusicXML();
}

void JukeBox::End()
{
  if (!m_init) return;

  StopAll();
  StopMusic();

  m_cache.Clear();

  m_soundsamples.clear();
  m_profiles_loaded.clear();
  playlist.clear();

  Mix_CloseAudio();

  m_init = false;
}

void JukeBox::SetFrequency (int frequency)
{
  if ((frequency != 11025)
      && (frequency != 22050)
      && (frequency != 44100)) frequency = 44100;

  if (m_config.frequency == frequency) return;

  m_config.frequency = frequency;

  // Close and reopen audio device
  End();
  Init();
}

// Code not used
#if 0
void JukeBox::SetNumbersOfChannel(int channels)
{
  if (m_config.channels == channels) return;

  m_config.channels = channels;

  // Close and reopen audio device
  End();
  Init();
}
#endif

void JukeBox::ActiveMusic (bool on)
{
   if(IsPlayingMusic() && !on)
      StopMusic();
   m_config.music = on;
}

void JukeBox::LoadMusicXML()
{
  if (!m_init) // the sound device has not be initialized (was busy?)
    return;

  // is xml_file already loaded ?
  std::set<std::string>::iterator it_profile = m_profiles_loaded.find("music") ;
  if (it_profile !=  m_profiles_loaded.end())
  {
    MSG_DEBUG("jukebox", "Music is already loaded !");
    return;
  }
  std::cout << _("o Loading music") << std::endl;

  XmlReader doc;

  // Load the XML
  std::string folder = Config::GetInstance()->GetDataDir() + "music" PATH_SEPARATOR;
  std::string xml_filename = folder + "profile.xml";
  if( !DoesFileExist(xml_filename) )
  {
    std::cerr << "[Music] Error : file " << xml_filename
              << " not found" << std::endl;
    return;
  }
  if(!doc.Load(xml_filename))
    return;

  xmlNodeArray nodes = XmlReader::GetNamedNeighbours(doc.GetRoot(), "music");
  xmlNodeArray::const_iterator
    it = nodes.begin(),
    end = nodes.end();

  for (; it != end; ++it)
  {
    // loading XML ...
    std::string sample="no_sample";
    std::string file="no_file";
    XmlReader::ReadStringAttr(*it, "type", sample);
    XmlReader::ReadStringAttr(*it, "playlist", file);

    MSG_DEBUG("jukebox", "Load music sample %s", sample.c_str());

    // Load sound
    std::string filename = folder + file;
    if( !DoesFileExist(filename) ){
      std::cerr << "Music error: File " << filename.c_str()
                << " does not exist !" << std::endl;
      continue;
    }

    std::ifstream fp(filename.c_str());
    std::string line;

    while(std::getline(fp, line))
    {
      if(line[0] == '#') continue;

      if(!DoesFileExist(line))
      {
        // This line comes from an XML file, thus path separator here is "/"
        line = filename.substr(0, filename.find_last_of("/")) + PATH_SEPARATOR + line;
        if(!DoesFileExist(line))
        {
          std::cerr << "[Music] Unable to find " << line << " music file." << std::endl;
          continue;
        }
      }
      playlist[sample].push_back(line);
    }
  }

  // The profile is loaded
  m_profiles_loaded.insert("music");
  playing_pl = playlist.end();
}

void JukeBox::EndMusic()
{
  JukeBox *jukebox = JukeBox::GetInstance();
  if(!jukebox->music)
    return;

  Mix_FreeMusic(jukebox->music);
  jukebox->music = 0;

  if(!jukebox->UseMusic() || !jukebox->IsPlayingMusic())
    return;

  if((jukebox->playing_music+1) == jukebox->playing_pl->second.end())
    jukebox->playing_music = jukebox->playing_pl->second.begin();
  else
    ++jukebox->playing_music;

  jukebox->PlayMusicSample(jukebox->playing_music);

  return;
}

void JukeBox::StopMusic()
{
  if(!IsPlayingMusicSample())
    return;
  playing_music = playing_pl->second.end();
  playing_pl = playlist.end();
  Mix_HaltMusic();
  EndMusic();
}

void JukeBox::NextMusic()
{
  if(!IsPlayingMusic())
    return;
  else if(!IsPlayingMusicSample())
    PlayMusic(playing_pl->first);
  else
    EndMusic(); // next music but before, we stop the current one.
}

bool JukeBox::PlayMusic(const std::string& type)
{
  if(m_init == false || !UseMusic()) return false;

  PlayListMap::iterator it = playlist.find(type);

  if(it == playlist.end())
  {
    std::cerr << "[Music] Unable to find " << type << " profile" << std::endl;
    return false;
  }

  MSG_DEBUG("jukebox", "Loading playlist %s", type.c_str());

  StopMusic();

  if(it->second.empty() || !UseMusic()) return false;

  playing_pl = it;

  int i, j = 0;
  do
  {
    i = rand()%it->second.size();
    j++;
  } while(!PlayMusicSample(it->second.begin()+i) && j < 10); // After 10 times, we think there is a problem.

  if(j >= 10)
    playing_pl = playlist.end();

  return (j < 10);
}

bool JukeBox::PlayMusicSample(const std::vector<std::string>::const_iterator& file_it)
{
  if(!UseMusic() || !m_init) return false;

  std::string file = *file_it;

  if(music)
    Mix_FreeMusic(music);

  music = Mix_LoadMUS(file.c_str());
  SetMusicVolume(Config::GetInstance()->GetVolumeMusic());
  MSG_DEBUG("jukebox", "We trying to load music %s", file.c_str());

  if(!music || Mix_PlayMusic(music, 0) < 0)
  {
    std::cerr << "[Music] Error : Unable to load music " << file << std::endl;
     playing_music = playing_pl->second.end();
     return false;
  }

  playing_music = file_it;
  return true;
}

void JukeBox::LoadXML(const std::string& profile)
{
  if (!m_init) // the sound device has not be initialized (was busy?)
    return;

  // is xml_file already loaded ?
  std::set<std::string>::iterator it_profile = m_profiles_loaded.find(profile);
  if (it_profile !=  m_profiles_loaded.end())
  {
    MSG_DEBUG("jukebox", "Profile %s is already loaded !", profile.c_str());
    return;
  }
  std::cout << Format(_("o Loading sound profile: %s"), profile.c_str()) << std::endl;

  XmlReader doc;

  // Load the XML
  std::string folder = Config::GetInstance()->GetDataDir()
                     + "sound" PATH_SEPARATOR + profile + PATH_SEPARATOR;
  std::string xml_filename = folder + "profile.xml";
  if( !DoesFileExist(xml_filename) )
  {
    std::cerr << "[Sound] Error : file " << xml_filename
              << " not found" << std::endl;
    return;
  }
  if(!doc.Load(xml_filename))
    return;

  xmlNodeArray nodes = XmlReader::GetNamedNeighbours(doc.GetRoot(), "sound");
  xmlNodeArray::const_iterator
    it = nodes.begin(),
    end = nodes.end();

  for (; it != end; ++it)
  {
    // reads XML
    std::string sample = "no_sample";
    std::string file   = "no_file";
    double      level  = 1.0;

    XmlReader::ReadStringAttr(*it, "sample", sample);
    XmlReader::ReadStringAttr(*it, "file", file);
    XmlReader::ReadDoubleAttr(*it, "level", level);

    MSG_DEBUG("jukebox", "Load sound sample %s/%s: %s (%.3f)",
              profile.c_str(), sample.c_str(), file.c_str(), level);

    // Load sound
    std::string sample_filename = folder + file;
    if( !DoesFileExist(sample_filename) )
    {
      std::cerr << "Sound warning: File " << sample_filename.c_str()
                << " does not exist! (probably not yet recorded)" << std::endl;
      continue;
    }

    // Inserting sound sample in list
    sample_info inf = { sample_filename, level };
    m_soundsamples.insert(sound_sample(profile+"/"+sample, inf));

    // Precache
    m_cache.Precache( sample_filename.c_str() );
  }

  // The profile is loaded
  m_profiles_loaded.insert(profile);
}

int JukeBox::Play (const std::string& category, const std::string& sample,
                   const int loop)
{
  if (!UseEffects() || !m_init) return -1;

  uint nb_sons= m_soundsamples.count(category+"/"+sample);
  if (nb_sons)
  {
    std::pair<sample_iterator, sample_iterator> p =
      m_soundsamples.equal_range(category+"/"+sample);
    sample_iterator it = p.first;

    // Choose a random sound sample
    if (nb_sons > 1)
    {
      uint selection = uint(randomObj.GetLong(0, nb_sons));
      if (selection == nb_sons) --selection ;

      it = p.first ;

      for ( uint i=0 ; i<selection && it!=p.second ; ++i ) it++ ;
    }

    // Play the sound
    Mix_Chunk * sampleChunk = m_cache.LoadSound( it->second.filename.c_str() );
    MSG_DEBUG("jukebox.play", "Playing sample %s/%s",
              category.c_str(), sample.c_str());

    return PlaySample(sampleChunk, it->second.level, loop);
  }
  else if (category != "default")
  {
    return Play("default", sample, loop) ; // try with default profile
  }

  std::cerr << "Sound error: No sound found for sample" << category.c_str()
            << "/" << sample.c_str() << std::endl;
  return -1;
}

int JukeBox::Stop (int channel) const
{
  if (!m_init) return 0;
  if (!m_config.music && !m_config.effects) return 0;
  if (channel == -1) return 0;
  return Mix_HaltChannel(channel);
}

int JukeBox::StopAll() const
{
  if (!m_init) return 0;
  if (!m_config.music && !m_config.effects) return 0;

  // halt playback on all channels
  return Mix_HaltChannel(-1);
}

int JukeBox::PlaySample (Mix_Chunk * sample, double level, int loop)
{
  if (loop != -1) loop--;

  Mix_VolumeChunk(sample, int(0.5+level*Config::GetInstance()->GetVolumeEffects()));
  int channel = Mix_PlayChannel(-1, sample, loop);

  if (channel == -1)
  {
    MSG_DEBUG("jukebox", "Error: Jukebox::PlaySample: %s", Mix_GetError());
    m_cache.FreeChunk( sample );
  }
  else
    chunks[channel] = sample;
  return channel;
}

void JukeBox::EndChunk(int channel)
{
  JukeBox *jukebox = JukeBox::GetInstance();
  Mix_Chunk* chk = jukebox->chunks[channel];

  if(!chk) return;

  //Mix_FreeChunk(chk);
  jukebox->m_cache.FreeChunk( chk );
  jukebox->chunks[channel] = 0;
}
