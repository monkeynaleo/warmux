#ifndef __BLOWTORCH_H__
#define __BLOWTORCH_H__

#include "weapon.h"

class Blowtorch : public Weapon
{
  private:
    uint new_timer, old_timer;
  protected:
    bool p_Shoot();
    void Refresh();
    void EndTurn();
  public:
    Blowtorch();
    void HandleKeyEvent(int action, int event_type);
};

#endif  // __BLOWTORCH_H__
