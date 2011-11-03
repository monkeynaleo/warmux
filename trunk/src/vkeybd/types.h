/*
 * types.h
 *
 *  Created on: Oct 17, 2011
 *      Author: mikkov
 */

#ifndef SCUMMVMTOWARMUX_TYPES_H_
#define SCUMMVMTOWARMUX_TYPES_H_

#ifdef ENABLE_VKEYBD

#include <SDL/SDL.h>

#define String  std::string
#define List    std::list

typedef SDL_keysym KeyState;
typedef Uint8 KeyCode; //SDL_keysym.scancode
typedef char byte;
typedef short int int16;
typedef unsigned int uint32;

#endif
#endif /* TYPES_H_ */
