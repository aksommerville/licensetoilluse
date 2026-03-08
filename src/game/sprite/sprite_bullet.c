/* sprite_bullet.c
 */
 
#include "game/licensetoilluse.h"

#define SPEED 10.0
#define DISAPPEAR_MARGIN 100

struct sprite_bullet {
  struct sprite hdr;
  double dx; // Get SPEED baked in.
};

#define SPRITE ((struct sprite_bullet*)sprite)

static int _bullet_init(struct sprite *sprite) {
  sprite->xform=sprite->arg[0];
  if (sprite->xform&EGG_XFORM_XREV) {
    SPRITE->dx=-SPEED;
  } else {
    SPRITE->dx=SPEED;
  }
  return 0;
}

static void _bullet_update(struct sprite *sprite,double elapsed) {
  sprite->x+=SPRITE->dx*elapsed;
  int xp=(int)(sprite->x*NS_sys_tilesize);
  if ((xp<g.camerax-DISAPPEAR_MARGIN)||(xp>g.camerax+FBW+DISAPPEAR_MARGIN)) {
    sprite->defunct=1;
  } else {
    if (g.hero&&!g.hero->defunct) {
      double l=g.hero->x+g.hero->hbl;
      double r=g.hero->x+g.hero->hbr;
      double t=g.hero->y+g.hero->hbt;
      double b=g.hero->y+g.hero->hbb;
      if ((sprite->x>l)&&(sprite->x<r)&&(sprite->y>t)&&(sprite->y>t)&&(sprite->y<b)) {
        sprite_hero_get_dead(g.hero,sprite);
      }
    }
  }
}

const struct sprite_type sprite_type_bullet={
  .name="bullet",
  .objlen=sizeof(struct sprite_bullet),
  .init=_bullet_init,
  .update=_bullet_update,
};
