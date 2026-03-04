/* sprite_dummy.c
 */
 
#include "game/licensetoilluse.h"
 
struct sprite_dummy {
  struct sprite hdr;
};

#define SPRITE ((struct sprite_dummy*)sprite)

/*
void _dummy_del(struct sprite *sprite) {
}
*/

/*
int _dummy_init(struct sprite *sprite) {
  return 0;
}
*/

/*
void _dummy_update(struct sprite *sprite,double elapsed) {
}
*/

/*
void _dummy_render(struct sprite *sprite,int x,int y) {
  graf_tile(&g.graf,x,y,sprite->tileid,sprite->xform);
}
*/

const struct sprite_type sprite_type_dummy={
  .name="dummy",
  .objlen=sizeof(struct sprite_dummy),
  //.del=_dummy_del,
  //.init=_dummy_init,
  //.update=_dummy_update,
  //.render=_dummy_render,
};
