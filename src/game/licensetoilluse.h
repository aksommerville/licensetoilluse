#ifndef EGG_GAME_MAIN_H
#define EGG_GAME_MAIN_H

#include "egg/egg.h"
#include "util/stdlib/egg-stdlib.h"
#include "util/graf/graf.h"
#include "util/font/font.h"
#include "util/res/res.h"
#include "util/text/text.h"
#include "egg_res_toc.h"
#include "shared_symbols.h"
#include "sprite/sprite.h"

#define SOUND_BLACKOUT_LIMIT 16

extern struct g {
  void *rom;
  int romc;
  struct graf graf;
  uint8_t physics[256];
  int input,pvinput;
  int songid_playing;
  struct sound_blackout {
    int rid;
    double when;
  } sound_blackoutv[SOUND_BLACKOUT_LIMIT];
  int sound_blackoutc;
  
  /* There's one map loaded at a time and it's read-only.
   */
  int mapid;
  int mapw,maph;
  const uint8_t *map;
  const uint8_t *mapcmd;
  int mapcmdc;
  
  /* One flat list of sprites, don't overthink it.
   * The whole map's worth of sprites get instantiated at scene start.
   */
  struct sprite **spritev;
  int spritec,spritea;
  struct sprite *hero; // WEAK, captured at start of cycle. Can be null.
  
  /* Most recent camera position.
   * Updates at the start of egg_client_render, and also gets an initial guess at start_scene.
   */
  int camerax,cameray;
  
  double resetclock;
  double fadeout;
  
} g;

int start_scene(int mapid);
void reset_soon();

/* Check for villains in the direction (dx) from (x,y), scare them if we find any.
 * The illusions should call this repeatedly as they update.
 */
void lti_scare_foes(double x,double y,double dx);

void lti_sound(int rid);
void lti_song(int rid);

#endif
