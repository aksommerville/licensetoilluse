/* sprite_coin.c
 */
 
#include "game/licensetoilluse.h"

struct sprite_coin {
  struct sprite hdr;
  uint8_t index;
  double animclock;
  int animframe; // 0=none, lasts long. 1..7=shimmer
  uint8_t tileid0;
};

#define SPRITE ((struct sprite_coin*)sprite)

static void coin_prepare_animation(struct sprite *sprite) {
  SPRITE->animframe=0;
  SPRITE->animclock=1.000+((rand()&0xffff)*2.000)/65535.0;
}

static int _coin_init(struct sprite *sprite) {
  SPRITE->tileid0=sprite->tileid;
  
  SPRITE->index=sprite->arg[0];
  if (SPRITE->index>=POWERUP_LIMIT) {
    fprintf(stderr,"Sprite requested coin index %d, limit %d\n",SPRITE->index,POWERUP_LIMIT);
    return -1;
  }
  if (g.powerupv[SPRITE->index]) {
    // Already got it. No worries, quietly reject.
    return -1;
  }
  
  coin_prepare_animation(sprite);
  
  return 0;
}

static void coin_collect(struct sprite *sprite) {
  lti_sound(RID_sound_powerup);
  g.coinc++;
  g.powerupv[SPRITE->index]=1;
  sprite->defunct=1;
}

static void _coin_update(struct sprite *sprite,double elapsed) {
  if (g.hero) {
    double dx=g.hero->x-sprite->x;
    if (dx<0.0) dx=-dx;
    if (dx<0.750) {
      double dy=g.hero->y-sprite->y;
      if ((dy>-0.333)&&(dy<1.125)) { // Hero must be allowed exactly 1m below, at least.
        coin_collect(sprite);
      }
    }
  }
  if ((SPRITE->animclock-=elapsed)<=0.0) {
    if (SPRITE->animframe>=7) {
      coin_prepare_animation(sprite);
    } else {
      SPRITE->animclock+=0.080;
      SPRITE->animframe++;
    }
    switch (SPRITE->animframe) {
      case 0: sprite->tileid=SPRITE->tileid0+0; sprite->xform=0; break;
      case 1: sprite->tileid=SPRITE->tileid0+1; sprite->xform=0; break;
      case 2: sprite->tileid=SPRITE->tileid0+2; sprite->xform=0; break;
      case 3: sprite->tileid=SPRITE->tileid0+3; sprite->xform=0; break;
      case 4: sprite->tileid=SPRITE->tileid0+4; sprite->xform=0; break;
      case 5: sprite->tileid=SPRITE->tileid0+3; sprite->xform=EGG_XFORM_XREV|EGG_XFORM_YREV; break;
      case 6: sprite->tileid=SPRITE->tileid0+2; sprite->xform=EGG_XFORM_XREV|EGG_XFORM_YREV; break;
      case 7: sprite->tileid=SPRITE->tileid0+1; sprite->xform=EGG_XFORM_XREV|EGG_XFORM_YREV; break;
    }
  }
}

const struct sprite_type sprite_type_coin={
  .name="coin",
  .objlen=sizeof(struct sprite_coin),
  .init=_coin_init,
  .update=_coin_update,
};
