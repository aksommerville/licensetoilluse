/* sprite_rabbit.c
 * Summoned by hero; walks forward and scares off villains.
 */
 
#include "game/licensetoilluse.h"

#define GRAVITY 7.0 /* m/s. Trying without a curve. */
#define WALK_SPEED 4.0
#define STUCK_TIME 1.000

struct sprite_rabbit {
  struct sprite hdr;
  double animclock;
  int animframe;
  uint8_t tileid0;
  double stuckclock;
};

#define SPRITE ((struct sprite_rabbit*)sprite)

static int _rabbit_init(struct sprite *sprite) {
  sprite->xform=sprite->arg[0];
  SPRITE->tileid0=sprite->tileid;
  return 0;
}

static void rabbit_animate(struct sprite *sprite,double elapsed) {
  if ((SPRITE->animclock-=elapsed)<=0.0) {
    SPRITE->animclock+=0.200;
    if (++(SPRITE->animframe)>=4) SPRITE->animframe=0;
    switch (SPRITE->animframe) {
      case 0: sprite->tileid=SPRITE->tileid0+0; break;
      case 1: sprite->tileid=SPRITE->tileid0+1; break;
      case 2: sprite->tileid=SPRITE->tileid0+0; break;
      case 3: sprite->tileid=SPRITE->tileid0+2; break;
    }
  }
}

static void _rabbit_update(struct sprite *sprite,double elapsed) {
  if (sprite_move(sprite,0.0,GRAVITY*elapsed)) {
    // Gravity moved us. Unset animation and don't move horizontally.
    sprite->tileid=SPRITE->tileid0;
    SPRITE->animclock=0.0;
    SPRITE->animframe=0;
    SPRITE->stuckclock=0.0;
  } else {
    // Seated. Animate, move horizontally, and scare foes.
    if (!sprite_move(sprite,((sprite->xform&EGG_XFORM_XREV)?-1.0:1.0)*WALK_SPEED*elapsed,0.0)) {
      // Blocked horizontally.
      if ((SPRITE->stuckclock+=elapsed)>=STUCK_TIME) {
        sprite->defunct=1;
        return;
      }
    } else {
      SPRITE->stuckclock=0.0;
    }
    rabbit_animate(sprite,elapsed);
    lti_scare_foes(sprite->x,sprite->y,(sprite->xform&EGG_XFORM_XREV)?-1.0:1.0);
  }
}

const struct sprite_type sprite_type_rabbit={
  .name="rabbit",
  .objlen=sizeof(struct sprite_rabbit),
  .init=_rabbit_init,
  .update=_rabbit_update,
};
