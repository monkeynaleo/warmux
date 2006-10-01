#ifndef __BLOWTORCH_H__
#define __BLOWTORCH_H__

#include "weapon.h"

class Blowtorch : public Weapon
{
	private:
		bool p_Shoot();
		uint new_timer, old_timer;
	public:
		Blowtorch();
		void DrawWeaponFire();
		void Refresh();
		void HandleKeyEvent(int action, int event_type);
};

#endif	// __BLOWTORCH_H__
