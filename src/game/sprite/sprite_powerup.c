/* sprite_powerup.c
 */
 
#include "game/licensetoilluse.h"

struct sprite_powerup {
  struct sprite hdr;
  uint8_t powertype;
  uint8_t index;
};

#define SPRITE ((struct sprite_powerup*)sprite)

static int _powerup_init(struct sprite *sprite) {
  
  struct cmdlist_reader reader;
  if (sprite_reader_init(&reader,sprite->cmd,sprite->cmdc)>=0) {
    struct cmdlist_entry cmd;
    while (cmdlist_reader_next(&cmd,&reader)>0) {
      switch (cmd.opcode) {
        case CMD_sprite_powertype: SPRITE->powertype=cmd.arg[1]; break;
      }
    }
  }
  
  SPRITE->index=sprite->arg[0];
  if (SPRITE->index>=POWERUP_LIMIT) {
    fprintf(stderr,"Sprite requested powerup index %d, limit %d\n",SPRITE->index,POWERUP_LIMIT);
    return -1;
  }
  if (g.powerupv[SPRITE->index]) {
    // Already got it. No worries, quietly reject.
    return -1;
  }
  return 0;
}

static void powerup_collect(struct sprite *sprite) {
  lti_sound(RID_sound_powerup);
  switch (SPRITE->powertype) {
    case NS_powertype_ghost: {
        g.have_ghost=1;
      } break;
    case NS_powertype_rabbit: {
        g.have_rabbit=1;
        g.rabbitc++;
      } break;
    case NS_powertype_bird: {
        g.have_bird=1;
        g.birdc++;
      } break;
  }
  g.powerupv[SPRITE->index]=1;
  sprite->defunct=1;
}

static void _powerup_update(struct sprite *sprite,double elapsed) {
  if (g.hero) {
    double dx=g.hero->x-sprite->x;
    if (dx<0.0) dx=-dx;
    if (dx<0.750) {
      double dy=g.hero->y-sprite->y;
      if ((dy>-0.333)&&(dy<1.125)) { // Hero must be allowed exactly 1m below, at least.
        powerup_collect(sprite);
      }
    }
  }
}

const struct sprite_type sprite_type_powerup={
  .name="powerup",
  .objlen=sizeof(struct sprite_powerup),
  .init=_powerup_init,
  .update=_powerup_update,
};
