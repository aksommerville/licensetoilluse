#include "game/licensetoilluse.h"

/* Move sprite with optional physics.
 */
 
int sprite_move(struct sprite *sprite,double dx,double dy) {
  if (!sprite) return 1;
  
  if (!sprite->solid) {
    sprite->x+=dx;
    sprite->y+=dy;
    return 1;
  }
  
  if (((dx<-0.0)||(dx>0.0))&&((dy<-0.0)||(dy>0.0))) {
    int xok=sprite_move(sprite,dx,0.0);
    int yok=sprite_move(sprite,0.0,dy);
    return xok||yok;
  }
  uint8_t dir;
  if (dy<0.0) dir=0x40;
  else if (dx<0.0) dir=0x10;
  else if (dx>0.0) dir=0x08;
  else if (dy>0.0) dir=0x02;
  else return 0;
  
  double nx=sprite->x+dx;
  double ny=sprite->y+dy;
  //TODO hitbox
  
  //TODO check grid
  if (ny>11.0) return 0;
  
  //TODO check other solids
  
  sprite->x+=dx;
  sprite->y+=dy;
  return 1;
}
