/* sprite_hero.c
 */
 
#include "game/licensetoilluse.h"

#define GRAVITY_INITIAL 0.0
#define GRAVITY_INCREASE_RATE 20.0
#define GRAVITY_LIMIT 20.0
#define GRAVITY_HEAVY_SOUND 20.0
#define GRAVITY_LIGHT_SOUND 5.0
#define JUMPPOWER_INITIAL 20.0
#define JUMPPOWER_LOSS_RATE 80.0
#define COYOTE_LIMIT 0.100
 
struct sprite_hero {
  struct sprite hdr;
  
  int wanimframe;
  double wanimclock;
  int walking;
  
  int seated;
  int jump_blackout;
  double jumppower;
  double gravity;
  double coyoteclock; // Counts up from loss of seat.
};

#define SPRITE ((struct sprite_hero*)sprite)

void _hero_del(struct sprite *sprite) {
}

/* Init.
 */

int _hero_init(struct sprite *sprite) {
  SPRITE->seated=1; // All start positions should be pre-seated. If not, we'll figure it out soon enough.
  SPRITE->gravity=GRAVITY_INITIAL;
  return 0;
}

/* Update walking.
 */
 
static void hero_update_walk(struct sprite *sprite,double elapsed) {
  int dx=0;
  switch (g.input&(EGG_BTN_LEFT|EGG_BTN_RIGHT)) {
    case EGG_BTN_LEFT: dx=-1; break;
    case EGG_BTN_RIGHT: dx=1; break;
  }
  if (dx) {
    if (dx<0) sprite->xform=EGG_XFORM_XREV;
    else sprite->xform=0;
    SPRITE->walking=1;
    sprite_move(sprite,dx*6.0*elapsed,0.0);
    if ((SPRITE->wanimclock-=elapsed)<=0.0) {
      SPRITE->wanimclock+=0.200;
      if (++(SPRITE->wanimframe)>=4) SPRITE->wanimframe=0;
    }
  } else {
    SPRITE->walking=0;
    SPRITE->wanimclock=0.0;
    SPRITE->wanimframe=0;
  }
}

/* Attempt downjump.
 */
 
static void hero_attempt_downjump(struct sprite *sprite) {
  #define NOPE { lti_sound(RID_sound_reject); return; }
  int row=(int)(sprite->y+sprite->hbb+0.001);
  if ((row<0)||(row>=g.maph)) NOPE
  int cola=(int)(sprite->x+sprite->hbl);
  int colz=(int)(sprite->x+sprite->hbr-0.001);
  if (cola<0) cola=0;
  if (colz>=g.mapw) colz=g.mapw-1;
  if (cola>colz) NOPE
  int oneway=0;
  const uint8_t *mapp=g.map+row*g.mapw+cola;
  for (;cola<=colz;cola++,mapp++) {
    uint8_t physics=g.physics[*mapp];
    if (physics==NS_physics_solid) NOPE;
    if (physics==NS_physics_oneway) oneway=1;
  }
  if (!oneway) NOPE
  #undef NOPE
  lti_sound(RID_sound_downjump);
  sprite->y+=0.005; // Cheat down a smidge, then let nature take its course.
}

/* Start regular jump.
 */
 
static void hero_start_jump(struct sprite *sprite) {
  SPRITE->jump_blackout=1;
  SPRITE->jumppower=JUMPPOWER_INITIAL;
  lti_sound(RID_sound_jump);
}

/* Update jumping and gravity.
 */
 
static void hero_update_jump(struct sprite *sprite,double elapsed) {
  
  // Holding down and pressed south, while seated? Attempt the oneway pass-thru jump.
  if (SPRITE->seated&&(g.input&EGG_BTN_DOWN)&&(g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) {
    hero_attempt_downjump(sprite);
    return;
  }
  
  // New stroke of south when seated? Start a jump.
  if (SPRITE->seated&&!(g.input&EGG_BTN_DOWN)&&(g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)&&!SPRITE->jump_blackout) {
    hero_start_jump(sprite);
    return;
  }
  
  // New stroke of south but not seated? Check the coyote clock.
  if (!SPRITE->seated&&!(g.input&EGG_BTN_DOWN)&&(g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)&&!SPRITE->jump_blackout) {
    if (SPRITE->coyoteclock<=COYOTE_LIMIT) {
      hero_start_jump(sprite);
      return;
    }
  }
  
  // Can we drop the jump blackout?
  if (!(g.input&EGG_BTN_SOUTH)) {
    SPRITE->jump_blackout=0;
  }
  
  // Jump ongoing? Decay and apply, or decay to zero if south is released.
  if (SPRITE->jumppower>0.0) {
    if (!(g.input&EGG_BTN_SOUTH)) {
      // Jump terminated early by user.
      SPRITE->jumppower=0.0;
    } else if ((SPRITE->jumppower-=JUMPPOWER_LOSS_RATE*elapsed)<=0.0) {
      // Jump worn out.
    } else {
      // Jump ongoing.
      sprite_move(sprite,0.0,-SPRITE->jumppower*elapsed);
    }
    
  // No jump? Apply gravity.  
  } else {
    if ((SPRITE->gravity+=GRAVITY_INCREASE_RATE*elapsed)>GRAVITY_LIMIT) SPRITE->gravity=GRAVITY_LIMIT;
    if (sprite_move(sprite,0.0,SPRITE->gravity*elapsed)) {
      if (SPRITE->seated) {
        SPRITE->seated=0;
        SPRITE->coyoteclock=0.0;
      } else {
        SPRITE->coyoteclock+=elapsed;
      }
    } else {
      if (!SPRITE->seated) {
        if (SPRITE->gravity>=GRAVITY_HEAVY_SOUND) lti_sound(RID_sound_land_heavy);
        else if (SPRITE->gravity>=GRAVITY_LIGHT_SOUND) lti_sound(RID_sound_land_light);
      }
      SPRITE->seated=1;
      SPRITE->gravity=GRAVITY_INITIAL;
      if (!SPRITE->jump_blackout&&!(g.input&EGG_BTN_DOWN)&&(g.input&EGG_BTN_SOUTH)) { // Let them bounce-jump, if it's a fresh stroke.
        hero_start_jump(sprite);
      }
    }
  }
}

/* Update.
 */

void _hero_update(struct sprite *sprite,double elapsed) {
  hero_update_walk(sprite,elapsed);
  hero_update_jump(sprite,elapsed);
}

/* Render.
 */

void _hero_render(struct sprite *sprite,int x,int y) {
  uint8_t tileid=sprite->tileid; // Nominal tile is the lower one.
  uint8_t xform=sprite->xform;
  
  if (SPRITE->jumppower>0.0) {
    tileid+=3;
  } else if (!SPRITE->seated&&(SPRITE->gravity>3.0)) { // Don't do the falling face immediately; it can get flickery.
    tileid+=4;
  } else if (SPRITE->walking) {
    switch (SPRITE->wanimframe) {
      case 1: tileid+=1; break;
      case 3: tileid+=2; break;
    }
  }
  
  graf_tile(&g.graf,x,y,tileid,xform);
  graf_tile(&g.graf,x,y-NS_sys_tilesize,tileid-0x10,xform);
}

/* Type definition.
 */

const struct sprite_type sprite_type_hero={
  .name="hero",
  .objlen=sizeof(struct sprite_hero),
  .del=_hero_del,
  .init=_hero_init,
  .update=_hero_update,
  .render=_hero_render,
};
