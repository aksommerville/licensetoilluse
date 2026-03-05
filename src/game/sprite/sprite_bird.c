/* sprite_bird.c
 * Summoned by hero; flies around and scares off villains.
 */
 
#include "game/licensetoilluse.h"

#define VERTICAL_SPEED 4.000 /* rad/sec */
#define VERTICAL_RANGE -2.0 /* +-m */
#define HORIZONTAL_SPEED 5.0 /* m/sec */
#define KILL_MARGIN 100 /* px; how far offscreen before we kill it */

struct sprite_bird {
  struct sprite hdr;
  double animclock;
  int animframe;
  uint8_t tileid0;
  double vphase;
  double y0;
};

#define SPRITE ((struct sprite_bird*)sprite)

static int _bird_init(struct sprite *sprite) {
  sprite->xform=sprite->arg[0];
  SPRITE->tileid0=sprite->tileid;
  SPRITE->y0=sprite->y;
  return 0;
}

static void _bird_update(struct sprite *sprite,double elapsed) {

  if ((SPRITE->animclock-=elapsed)<=0.0) {
    SPRITE->animclock+=0.150;
    if (++(SPRITE->animframe)>=4) SPRITE->animframe=0;
    switch (SPRITE->animframe) {
      case 0: sprite->tileid=SPRITE->tileid0+0; break;
      case 1: sprite->tileid=SPRITE->tileid0+1; break;
      case 2: sprite->tileid=SPRITE->tileid0+0; break;
      case 3: sprite->tileid=SPRITE->tileid0+2; break;
    }
  }
  
  SPRITE->vphase+=elapsed*VERTICAL_SPEED;
  if (SPRITE->vphase>M_PI) SPRITE->vphase-=M_PI*2.0;
  sprite->y=SPRITE->y0+sin(SPRITE->vphase)*VERTICAL_RANGE;
  
  if (sprite->xform&EGG_XFORM_XREV) sprite->x-=HORIZONTAL_SPEED*elapsed;
  else sprite->x+=HORIZONTAL_SPEED*elapsed;
  
  lti_scare_foes(sprite->x,sprite->y,(sprite->xform&EGG_XFORM_XREV)?-1.0:1.0);
  
  int xi=(int)(sprite->x*NS_sys_tilesize);
  if (
    (xi<g.camerax-KILL_MARGIN)||
    (xi>g.camerax+FBW+KILL_MARGIN)
  ) {
    sprite->defunct=1;
  }
}

const struct sprite_type sprite_type_bird={
  .name="bird",
  .objlen=sizeof(struct sprite_bird),
  .init=_bird_init,
  .update=_bird_update,
};
