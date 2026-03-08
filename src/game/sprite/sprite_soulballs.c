/* sprite_soulballs.c
 * In order to keep consistent with Dot Vine lore, there must be no more than 6 circles.
 * Magicians are emphatically not witches.
 */
 
#include "game/licensetoilluse.h"

#define BALLC 6
#define VELOCITY 60.0 /* px/s */
#define FRAME_PERIOD 0.150
#define FRAMEC 8 /* 5 tiles, pingpongging */
#define TOTAL_TIME 2.0
#define FADE_TIME 1.0

struct sprite_soulballs {
  struct sprite hdr;
  struct ball {
    double x,y; // Pixels, relative to the sprite.
    double dx,dy; // px/sec, speed baked in.
    double animclock;
    int animframe;
  } ballv[BALLC];
  double ttl;
};

#define SPRITE ((struct sprite_soulballs*)sprite)

static int _soulballs_init(struct sprite *sprite) {
  // Each ball gets a random frame and clock, and a symmetric delta with the velocity baked in.
  struct ball *ball=SPRITE->ballv;
  int i=BALLC;
  for (;i-->0;ball++) {
    double t=(i*M_PI*2.0)/BALLC;
    ball->dx=VELOCITY*sin(t);
    ball->dy=VELOCITY*cos(t);
    ball->animclock=((rand()&0xffff)*FRAME_PERIOD)/65535.0;
    ball->animframe=rand()%FRAMEC;
  }
  SPRITE->ttl=TOTAL_TIME;
  return 0;
}

static void _soulballs_update(struct sprite *sprite,double elapsed) {
  if ((SPRITE->ttl-=elapsed)<=0.0) {
    sprite->defunct=1;
    return;
  }
  struct ball *ball=SPRITE->ballv;
  int i=BALLC;
  for (;i-->0;ball++) {
    ball->x+=ball->dx*elapsed;
    ball->y+=ball->dy*elapsed;
    if ((ball->animclock-=elapsed)<=0.0) {
      ball->animclock+=FRAME_PERIOD;
      if (++(ball->animframe)>FRAMEC) ball->animframe=0;
    }
  }
}

static void _soulballs_render(struct sprite *sprite,int x,int y) {
  if (SPRITE->ttl<FADE_TIME) {
    int alpha=(int)((SPRITE->ttl*255.0)/FADE_TIME);
    if (alpha<=0) return;
    if (alpha<0xff) graf_set_alpha(&g.graf,alpha);
  }
  struct ball *ball=SPRITE->ballv;
  int i=BALLC;
  for (;i-->0;ball++) {
    int bx=x+(int)ball->x;
    int by=y+(int)ball->y;
    uint8_t tileid=sprite->tileid;
    switch (ball->animframe) {
      case 0: break;
      case 1: tileid+=1; break;
      case 2: tileid+=2; break;
      case 3: tileid+=3; break;
      case 4: tileid+=4; break;
      case 5: tileid+=3; break;
      case 6: tileid+=2; break;
      case 7: tileid+=1; break;
    }
    graf_tile(&g.graf,bx,by,tileid,0);
  }
  graf_set_alpha(&g.graf,0xff);
}

const struct sprite_type sprite_type_soulballs={
  .name="soulballs",
  .objlen=sizeof(struct sprite_soulballs),
  .init=_soulballs_init,
  .update=_soulballs_update,
  .render=_soulballs_render,
};
