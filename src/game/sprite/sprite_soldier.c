/* sprite_soldier.c
 * Your basic bad guy with a gun.
 */
 
#include "game/licensetoilluse.h"

#define GRAVITY 7.0
#define DISAPPEAR_MARGIN 100 /* pixels */
#define SPOOK_SPEED 8.0 /* m/s */
#define SPOOK_VISIBILITY 4.0 /* Horizontal meters. */
#define FIRE_TIME 1.000
#define FIRE_VISIBILITY 7.0 /* Horizontal meters. */
 
struct sprite_soldier {
  struct sprite hdr;
  double shocked; // Counts down during the initial surprise.
  int spooked; // Direction of travel, if spooked.
  double animclock;
  int animframe;
  double recoil; // Counts down after firing rifle.
  double fireclock; // Counts down while hero in the fire box, before firing rifle.
};

#define SPRITE ((struct sprite_soldier*)sprite)

int _soldier_init(struct sprite *sprite) {
  return 0;
}

/* Nonzero if there's something in our face direction that should scare us.
 * Face direction is based on xform: 0=right, XREV=left.
 */
 
static int soldier_sees_something_spooky(struct sprite *sprite) {

  // Compute the visibility box.
  double vist=sprite->y-2.0;
  double visb=sprite->y+1.0;
  double visl,visr;
  if (sprite->xform&EGG_XFORM_XREV) {
    visr=sprite->x;
    visl=visr-SPOOK_VISIBILITY;
  } else {
    visl=sprite->x;
    visr=visl+SPOOK_VISIBILITY;
  }

  // Anything spooky in there?
  struct sprite **otherp=g.spritev;
  int i=g.spritec;
  for (;i-->0;otherp++) {
    struct sprite *other=*otherp;
    if (other->defunct) continue;
    if (!other->spooky) continue;
    if (other->x<visl) continue;
    if (other->x>visr) continue;
    if (other->y<vist) continue;
    if (other->y>visb) continue;
    return 1;
  }
  return 0;
}

/* Nonzero if the hero is in our fire range.
 */
 
static int solider_sees_something_shooty(struct sprite *sprite) {
  if (!g.hero) return 0;
  double dy=g.hero->y-sprite->y;
  if ((dy<-2.0)||(dy>1.0)) return 0;
  double dx=g.hero->x-sprite->x;
  if (sprite->xform&EGG_XFORM_XREV) {
    if (dx<-FIRE_VISIBILITY) return 0;
    if (dx>0.0) return 0;
  } else {
    if (dx<0.0) return 0;
    if (dx>FIRE_VISIBILITY) return 0;
  }
  return 1;
}

/* Fire my rifle.
 */
 
static void soldier_fire(struct sprite *sprite) {
  SPRITE->recoil=0.500;
  SPRITE->fireclock=0.0;
  lti_sound(RID_sound_rifle);
  double bulletx=sprite->x;
  double bullety=sprite->y;
  if (sprite->xform&EGG_XFORM_XREV) bulletx-=0.5;
  else bulletx+=0.5;
  uint8_t arg[4]={sprite->xform,0,0,0};
  struct sprite *bullet=sprite_spawn(bulletx,bullety,RID_sprite_bullet,arg,sizeof(arg));
  if (!bullet) return;
}

/* Update.
 */

void _soldier_update(struct sprite *sprite,double elapsed) {

  /* Gravity always applies.
   */
  int falling=sprite_move(sprite,0.0,GRAVITY*elapsed);
  
  /* During shock, just tick the timer down.
   */
  if (SPRITE->shocked>0.0) {
    if ((SPRITE->shocked-=elapsed)>=0.0) return;
    sprite->xform^=EGG_XFORM_XREV;
  }

  /* If we're spooked, it's permanent.
   * Run until we're sufficiently far offscreen, and if we get far enough, disappear.
   */
  if (SPRITE->spooked) {
    if ((SPRITE->animclock-=elapsed)<=0.0) {
      SPRITE->animclock+=0.200;
      if (++(SPRITE->animframe)>=2) SPRITE->animframe=0;
    }
    if (!falling) {
      sprite_move(sprite,SPOOK_SPEED*SPRITE->spooked*elapsed,0.0);
    }
    int xp=sprite->x*NS_sys_tilesize;
    if ((xp<g.camerax-DISAPPEAR_MARGIN)||(xp>g.camerax+FBW+DISAPPEAR_MARGIN)) {
      sprite->defunct=1;
    }
    return;
  }

  /* Face the hero.
   */
  if (g.hero) {
    if (g.hero->x<sprite->x) sprite->xform=EGG_XFORM_XREV;
    else sprite->xform=0;
  }
  
  /* If we see a ghost, rabbit, or bird, get spooked.
   */
  if (soldier_sees_something_spooky(sprite)) {
    SPRITE->shocked=0.500;
    if (sprite->xform&EGG_XFORM_XREV) {
      SPRITE->spooked=1;
    } else {
      SPRITE->spooked=-1;
    }
    sprite_spawn(sprite->x,sprite->y,RID_sprite_rifle,0,0);
    return;
  }
  
  /* If the rifle was just fired, tick it down and do nothing else.
   */
  if (SPRITE->recoil>0.0) {
    if ((SPRITE->recoil-=elapsed)>0.0) return;
  }
  
  /* If the hero is in the fire box, tick down fireclock, and fire the rifle when it expires.
   */
  if (solider_sees_something_shooty(sprite)) {
    if (SPRITE->fireclock<=0.0) SPRITE->fireclock=FIRE_TIME;
    if ((SPRITE->fireclock-=elapsed)<=0.0) {
      soldier_fire(sprite);
    }
  } else {
    SPRITE->fireclock=0.0;
  }
  
  //TODO Walk sometimes.
}

/* Render.
 */

void _soldier_render(struct sprite *sprite,int x,int y) {
  uint8_t tileid=sprite->tileid;
  uint8_t xform=sprite->xform;
  
  if (SPRITE->shocked>0.0) {
    tileid+=4;
    
  } else if (SPRITE->spooked) {
    tileid+=5+SPRITE->animframe;
  
  } else if (SPRITE->recoil>0.0) {
    tileid+=3;
    
  //TODO walking. Standing still is the default.
  }
  
  graf_tile(&g.graf,x,y,tileid,xform);
  graf_tile(&g.graf,x,y-NS_sys_tilesize,tileid-0x10,xform);
}

/* Type definition.
 */

const struct sprite_type sprite_type_soldier={
  .name="soldier",
  .objlen=sizeof(struct sprite_soldier),
  .init=_soldier_init,
  .update=_soldier_update,
  .render=_soldier_render,
};
