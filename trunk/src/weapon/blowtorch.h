#ifndef __BLOWTORCH_H__
#define __BLOWTORCH_H__

#include "weapon.h"

class BlowtorchConfig : public WeaponConfig
{
  public:
    BlowtorchConfig();
    virtual void LoadXml(xmlpp::Element* elem);

    uint range;
};

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
    BlowtorchConfig& cfg();
    void HandleKeyEvent(int action, int event_type);
};

#endif  // __BLOWTORCH_H__
