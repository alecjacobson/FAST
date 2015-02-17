#ifndef BUILD_ANTTWEAKBAR_MODIFIER_H
#define BUILD_ANTTWEAKBAR_MODIFIER_H

#ifdef STATIC_ANTTWEAKBAR
#  include "AntTweakBar.h"
#else
#  include <AntTweakBar.h>
#endif

// Builds anttweakbar modifier key from list of booleans telling whether each modifier key is up or down
// Inputs:
//   shift_down  shift key is down
//   control_down  control key is down
//   option_down  option key is down
// Returns TwKeyModifier ORed with corresponded modifier keys
inline int build_anttweakbar_modifier(
  bool shift_down,
  bool control_down,
  bool meta_down);
 

// Implementation
inline int build_anttweakbar_modifier(
  bool shift_down,
  bool control_down,
  bool meta_down)
{
  int twMod = TW_KMOD_NONE;
  if (control_down)
    twMod |= TW_KMOD_CTRL;
  if (shift_down)
    twMod |= TW_KMOD_SHIFT;
  if (meta_down)
    twMod |= TW_KMOD_ALT;
  return twMod;
}
#endif
