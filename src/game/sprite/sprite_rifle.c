/* sprite_rifle.c
 * Decoration that flies out of a soldier's hands when he gets spooked.
 * Soldier gives us the final position.
 */

#include "game/licensetoilluse.h"

#define DY_INITIAL -6.0
#define GRAVITY 20.0 /* m/s**2 */

struct sprite_rifle {
  struct sprite hdr;
  double xz,yz;
  double dy;
};

#define SPRITE ((struct sprite_rifle*)sprite)

static int _rifle_init(struct sprite *sprite) {
  SPRITE->xz=sprite->x;
  SPRITE->yz=sprite->y;
  SPRITE->dy=DY_INITIAL;
  return 0;
}

static void _rifle_update(struct sprite *sprite,double elapsed) {
  SPRITE->dy+=GRAVITY*elapsed;
  sprite->y+=SPRITE->dy*elapsed;
  if ((sprite->y>=SPRITE->yz)&&(SPRITE->dy>0.0)) {
    // In general, reassigning (type) would be wildly unsafe. Pretend you don't see this:
    sprite->type=&sprite_type_dummy;
  }
}

const struct sprite_type sprite_type_rifle={
  .name="rifle",
  .objlen=sizeof(struct sprite_rifle),
  .init=_rifle_init,
  .update=_rifle_update,
};
