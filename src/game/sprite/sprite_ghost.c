/* sprite_ghost.c
 * Summoned by hero; scares off villains.
 */
 
#include "game/licensetoilluse.h"

struct sprite_ghost {
  struct sprite hdr;
  uint8_t tileid0;
  double animclock;
  int animframe;
};

#define SPRITE ((struct sprite_ghost*)sprite)

static int _ghost_init(struct sprite *sprite) {
  sprite->spooky=1;
  sprite->xform=sprite->arg[0];
  SPRITE->tileid0=sprite->tileid;
  SPRITE->animclock=0.250;
  return 0;
}

static void _ghost_update(struct sprite *sprite,double elapsed) {
  if ((SPRITE->animclock-=elapsed)<=0.0) {
    SPRITE->animclock+=0.250;
    if (++(SPRITE->animframe)>=2) SPRITE->animframe=0;
    sprite->tileid=SPRITE->tileid0+SPRITE->animframe;
  }
  lti_scare_foes(sprite->x,sprite->y,(sprite->xform&EGG_XFORM_XREV)?-1.0:1.0);
}

const struct sprite_type sprite_type_ghost={
  .name="ghost",
  .objlen=sizeof(struct sprite_ghost),
  .init=_ghost_init,
  .update=_ghost_update,
};
