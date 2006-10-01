#include "../weapon/blowtorch.h"
#include "../tool/i18n.h"
#include "../map/map.h"
#include "../team/teams_list.h"
#include "../game/game_loop.h"
#include "../game/time.h"
#include "../character/move.h"
#include "explosion.h"

static const uint pause_time = 200;	// milliseconds

Blowtorch::Blowtorch() : Weapon(WEAPON_BLOWTORCH, "blowtorch", new WeaponConfig())
{
	m_name = _("Blowtorch");
	override_keys = true;

	new_timer = 0;
	old_timer = 0;

	m_weapon_fire = new Sprite(resource_manager.LoadImage(weapons_res_profile, "blowtorch_fire"));
}

void Blowtorch::DrawWeaponFire()
{
	//Point2i pos = ActiveCharacter().GetHandPosition();
	double angle = ActiveTeam().crosshair.GetAngle();
	Point2i size = m_weapon_fire->GetSize();
	m_weapon_fire->SetRotation_deg(angle);
	size.x = (ActiveCharacter().GetDirection() == 1 ? -20 : size.x + 20);
	size.y /= 2;
	m_weapon_fire->Draw(GetGunHolePosition() - size);
}

void Blowtorch::Refresh()
{

}

bool Blowtorch::p_Shoot()
{
	ActiveCharacter().SetRebounding(false);
	ActiveCharacter().body->StartWalk();

	//int direction = ActiveCharacter().GetDirection();	// -1 left & +1 right
	uint posX = ActiveCharacter().GetX() + ActiveCharacter().GetWidth()/2;
	uint posY = ActiveCharacter().GetY() + ActiveCharacter().GetHeight()/2;

	double angle = ActiveTeam().crosshair.GetAngleRad();
	double dx = cos(angle) * 2.0;
	double dy = sin(angle) * 2.0;

	Point2i pos = Point2i(posX+(int)dx, posY+(int)dy);
	world.Dig(pos, ActiveCharacter().GetHeight()/2);

	MoveCharacter(ActiveCharacter());
	ActiveCharacter().SetXY(ActiveCharacter().GetPosition());

	return true;
}

void Blowtorch::HandleKeyEvent(int action, int event_type)
{
	switch(action)
	{
		case ACTION_SHOOT:
			if(event_type == KEY_RELEASED)
			{
				ActiveCharacter().body->ResetWalk();
				ActiveCharacter().body->StopWalk();
				m_is_active = false;
				ActiveTeam().AccessNbUnits() = 0;
				GameLoop::GetInstance()->SetState(GameLoop::HAS_PLAYED);
			}
			else if(event_type == KEY_REFRESH)
			{
				if(!EnoughAmmoUnit())
				{
					ActiveCharacter().body->ResetWalk();
					ActiveCharacter().body->StopWalk();
				}
				new_timer = Time::GetInstance()->Read();
				if(new_timer - old_timer >= pause_time)
				{
					NewActionShoot();
					old_timer = new_timer;
				}
			}

			break;
		default:
			break;
	}
}
