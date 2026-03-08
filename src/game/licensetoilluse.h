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
#define GOAL_LIMIT 16
#define POWERUP_LIMIT 64 /* All powerup instances are identified; you can only get each once. */

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
   * We index all the CMD_map_goal, and sort by (y,x).
   */
  int mapid;
  int mapw,maph;
  const uint8_t *map;
  const uint8_t *mapcmd;
  int mapcmdc;
  struct goal { uint8_t x,y; } goalv[GOAL_LIMIT];
  int goalc;
  
  /* One flat list of sprites, don't overthink it.
   * The whole map's worth of sprites get instantiated at scene start.
   */
  struct sprite **spritev;
  int spritec,spritea;
  struct sprite *hero; // WEAK, captured at start of cycle. Can be null.
  int heroqx,heroqy; // Track hero's quantized position for goal detection.
  double goalclock;
  
  /* Most recent camera position.
   * Updates at the start of egg_client_render, and also gets an initial guess at start_scene.
   */
  int camerax,cameray;
  
  double resetclock;
  double fadeout;
  
  int have_ghost,have_rabbit,have_bird; // Have we ever had this powerup? For ghost, that's the whole story.
  int rabbitc,birdc;
  uint8_t powerupv[POWERUP_LIMIT];
  double playtime;
  
} g;

void reset_game(); // Full reset, not just level-to-level or death.
int start_scene(int mapid);
void reset_soon();
void scene_update_goal(double elapsed);

/* Check for villains in the direction (dx) from (x,y), scare them if we find any.
 * The illusions should call this repeatedly as they update.
 */
void lti_scare_foes(double x,double y,double dx);

void lti_sound(int rid);
void lti_song(int rid);

#endif
