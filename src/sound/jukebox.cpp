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

#include "jukebox.h"
//-----------------------------------------------------------------------------
#include "../game/config.h"
#include "../tool/i18n.h"
#include "../tool/random.h"
#include "../tool/file_tools.h"
#include <ClanLib/core.h>
#include <ClanLib/sound.h>
using namespace std;
using namespace Wormux;
//-----------------------------------------------------------------------------

#ifdef DEBUG

// Débogue le jukebox ?
//#define DBG_SON

#define COUT_DBG cout << "[Son] "

#endif

//-----------------------------------------------------------------------------
JukeBox jukebox;
//-----------------------------------------------------------------------------

JukeBox::JukeBox()
{
  res = NULL ;
  m_init = false;
  m_init_res = false;
  
  m_config.use = true;
  m_config.music = true;
  m_config.effects = true;
  m_config.frequency = 22050;
}

//-----------------------------------------------------------------------------

void JukeBox::Init() 
{
  if (!UseMusic()) return;
  if (m_init) return;

  bool delete_res = false;
  try
    {
      soundinit = new CL_SetupSound();
      vorbisinit = new CL_SetupVorbis() ; 

      soundoutput = new CL_SoundOutput(m_config.frequency);

      if (!m_init_res) 
      {
	delete_res = true;
	res = new CL_ResourceManager(config.data_dir+"sound.xml", false);
	delete_res = false;
	m_init_res = true;
      } 

      m_init = true ;
      
#ifdef DBG_SON
      COUT_DBG << "Everything OK !" << endl ;
#endif
    }
  catch (CL_Error err)
    {
      cerr << "Error initialising sound: " << err.message.c_str() << std::endl;
      if (delete_res) {
	res = NULL;
	m_init_res = false;
      }
    }
}

//-----------------------------------------------------------------------------

void JukeBox::End() 
{

  if (!m_init) return;
  m_init = false;


  m_soundbuffers.clear();
  m_profiles_loaded.clear();

  delete soundoutput; soundoutput = NULL;
  delete soundinit; soundinit = NULL ;
  delete vorbisinit; vorbisinit = NULL;
}


//-----------------------------------------------------------------------------

void JukeBox::Play(const string& soundfile,
		   const bool loop,
		   CL_SoundBuffer_Session **session) 
{
  if (!UseEffects()) return;
  if (res == NULL) return;

  iterator it= m_soundbuffers.find(soundfile) ;
  sound_item_t sound;

  if (it !=  m_soundbuffers.end()) 
  {
#ifdef DBG_SON
    COUT_DBG << "Son " << soundfile << " trouvé, le joue." << endl;
#endif

    PlayItem (it->second, loop, session);
    return;
  }

#ifdef DBG_SON
  COUT_DBG << "Charge la ressource " << soundfile << endl;
#endif
  try 
  {
    sound.error_displayed = false;
    sound.session = NULL;
    sound.buffer = new CL_SoundBuffer(soundfile, res);
    PlayItem (sound, loop, session);
  }
  catch (const CL_Error &err)
  {
    cerr << endl
	 << _("Sound error :") << endl
	 << err.message.c_str() << endl
	 << endl;
    sound.buffer = NULL;
  }

  // Insert sound into our list
  m_soundbuffers.insert(paire_soundbuffer(soundfile, sound));
}

//-----------------------------------------------------------------------------

void JukeBox::PlayItem (sound_item_t &item, const bool loop, 
			CL_SoundBuffer_Session **session)
{
  if (item.buffer == NULL) return;

  CL_SoundBuffer_Session playback = item.buffer -> prepare();

  item.session = new CL_SoundBuffer_Session(playback);

  (item.session)->set_looping(loop);
  (item.session)->play() ;

  if (session != NULL) *session = new CL_SoundBuffer_Session(playback);
}

//-----------------------------------------------------------------------------

void JukeBox::PlayProfile(const string& profile, const string& action, const bool loop,
		   CL_SoundBuffer_Session **ptr_session)
{
  if (!UseEffects()) return;

  uint nb_sons= m_soundbuffers.count(profile+"/"+action);
  if (nb_sons) 
  {
    pair<iterator, iterator> p = m_soundbuffers.equal_range(profile+"/"+action);
    iterator it = p.first;

    // on choisit un son au hasard
    if (nb_sons > 1)
    {
      uint selection = uint(RandomLong(0, nb_sons));
      if (selection == nb_sons) --selection ;

#ifdef DBG_SON
      COUT_DBG << "Joue le son n° " << selection+1 << "/" << nb_sons << " pour " << profile << "/" << action  << endl;
#endif
      it = p.first ;
      
      for ( uint i=0 ; i<selection && it!=p.second ; ++i ) it++ ;
    }
#ifdef DBG_SON
    else
      COUT_DBG << "Un seul son pour " << profile << "/" << action << endl ;
#endif

    PlayItem(it->second, loop, ptr_session);
    return;
  }
  else if (profile != "default") {
    PlayProfile("default", action, loop, ptr_session) ; // on essaie avec le profil par défaut    
#ifdef DBG_SON
  } else {
    COUT_DBG << "Aucun son pour l'action " << action << endl ;
#endif
  }
}


//-----------------------------------------------------------------------------
void JukeBox::Load(const std::string& profile)
{

  if (!UseEffects()) return;

  // Profile already loaded
  std::set<std::string>::iterator it_profile = m_profiles_loaded.find(profile) ;
  if (it_profile !=  m_profiles_loaded.end()) {
#ifdef DBG_SON
    COUT_DBG << "Profile " << profile << "is already loaded !" << endl ;
    return ;
#endif
  } 
  LitDocXml doc;


  // Charge le XML
  std::string repertoire = config.data_dir + "sound/"+ profile + '/';
  std::string nomfich = repertoire + "profile.xml";
  if (!FichierExiste(nomfich)) return;
  if (!doc.Charge (nomfich)) return;

  xmlpp::Node::NodeList nodes = doc.racine() -> get_children("sound");
  xmlpp::Node::NodeList::iterator 
    it=nodes.begin(),
    fin=nodes.end();

  for (; it != fin; ++it)
    {
      // lit le XML
      xmlpp::Element *elem = dynamic_cast<xmlpp::Element*> (*it);
      std::string action="no_action";
      std::string filename="no_filename";
      LitDocXml::LitAttrString(elem, "action", action);
      LitDocXml::LitAttrString(elem, "filename", filename);

#ifdef DBG_SON
      COUT_DBG << "Charge le son " << profile << "/" << action << " : " << filename << endl;
#endif

      // Charge le son
      std::string nom_fichier_complet = repertoire + filename;
      if (!FichierExiste(nom_fichier_complet))
	{
	  cerr << endl
	       << _("Sound error :")  << endl
	       << Format(_("File \"%s\" does not exist !"),
			 nom_fichier_complet.c_str()) 
	       << endl << endl;
	  continue;
	}
    
      sound_item_t sound;

      try 
	{
	  sound.buffer = new CL_SoundBuffer(nom_fichier_complet, true);

	  /* */
	  sound.session = NULL;
	  /* */
	  // On insère le son dans notre liste
	  m_soundbuffers.insert(paire_soundbuffer(profile+"/"+action, sound)); 
	}
      catch (const CL_Error &err)
	{
	  cerr << endl
	       << _("Sound error :") << endl
	       << err.message.c_str() << endl
	       << endl;
	  sound.buffer = NULL;
	}
    }

  // The profile is loaded
  m_profiles_loaded.insert(profile);
}

//-----------------------------------------------------------------------------
void JukeBox::Stop(const string& soundfile) 
{
  if (!UseEffects()) return;
  iterator it= m_soundbuffers.find(soundfile) ;
  
  if ( it !=  m_soundbuffers.end() ) 
  {
    if (it->second.buffer == NULL) return;
#ifdef DBG_SON
    COUT_DBG << "Stoppe le son : " << soundfile << endl;
    COUT_DBG << "Still playing ? " 
	     << it->second.buffer -> is_playing() << endl; 
    // is_playing() return always false ! (see Clanlib code !!)
#endif
    if (it->second.session != NULL) it->second.session -> stop();
  }
}

//-----------------------------------------------------------------------------

void JukeBox::StopAll()
{
  if (!UseSound()) return;

  /*
  assert (soundoutput != NULL);
  soundoutput -> stop_all(); // method not implemented in ClanLib !!!
  */
  
  iterator it = m_soundbuffers.begin(),
    fin= m_soundbuffers.end();

  for (; it != fin; ++it)
    if (it->second.session != NULL) it->second.session -> stop();
}

//-----------------------------------------------------------------------------

void JukeBox::ActiveSound (bool on)
{
  m_config.use = on; 
  if (on) 
    Init();
  else
    StopAll();
}
void JukeBox::ActiveMusic (bool on) { m_config.music = on; }
void JukeBox::ActiveEffects (bool on) { m_config.effects = on; }
bool JukeBox::UseSound() const { return m_config.use; }
bool JukeBox::UseMusic() const { return m_config.use && m_config.music; }
bool JukeBox::UseEffects() const { return m_config.use && m_config.effects; }
bool JukeBox::GetMusicConfig() const { return m_config.music; }
bool JukeBox::GetEffectsConfig() const { return m_config.effects; }
uint JukeBox::GetFrequency() const { return m_config.frequency; }

//-----------------------------------------------------------------------------

void JukeBox::SetFrequency (uint frequency)
{
  if ((frequency != 11025) 
      && (frequency != 22050) 
      && (frequency != 44100)) frequency = 22050;

  if (m_config.frequency == frequency) return;

  delete soundoutput;
  soundoutput = new CL_SoundOutput(frequency);

  m_config.frequency = frequency;
}

//-----------------------------------------------------------------------------
