#include "game/licensetoilluse.h"

#define SMIDGE 0.001

/* Move sprite with optional physics.
 */
 
int sprite_move(struct sprite *sprite,double dx,double dy) {
  if (!sprite) return 1;
  
  /* Non-solid sprites, just go.
   */
  if (!sprite->solid) {
    sprite->x+=dx;
    sprite->y+=dy;
    return 1;
  }
  
  /* Recur if both axes nonzero, or take the discrete direction.
   */
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
  
  /* Compute the new position optimistically.
   * When we get into testing, tests are free to adjust (nx,ny). Each must call REHITBOX after changing.
   * It's fine to go wild; we'll clean up at the end.
   */
  double nx=sprite->x+dx;
  double ny=sprite->y+dy;
  double nl,nr,nt,nb;
  #define REHITBOX { \
    nl=nx+sprite->hbl; \
    nr=nx+sprite->hbr; \
    nt=ny+sprite->hbt; \
    nb=ny+sprite->hbb; \
  }
  REHITBOX
  
  #define CHECKBOX(l,r,t,b) { \
    double _l=(l),_r=(r),_t=(t),_b=(b); \
    if (_l>=nr) ; \
    else if (_r<=nl) ; \
    else if (_t>=nb) ; \
    else if (_b<=nt) ; \
    else { \
      /* Collision! */ \
      switch (dir) { \
        case 0x40: ny=_b-sprite->hbt; break; \
        case 0x10: nx=_r-sprite->hbl; break; \
        case 0x08: nx=_l-sprite->hbr; break; \
        case 0x02: ny=_t-sprite->hbb; break; \
      } \
      REHITBOX \
    } \
  }
  
  /* Check grid.
   */
  {
    int cola=(int)nl; if (cola<0) cola=0;
    int rowa=(int)nt; if (rowa<0) rowa=0;
    int colz=(int)(nr-SMIDGE); if (colz>=g.mapw) colz=g.mapw-1;
    int rowz=(int)(nb-SMIDGE); if (rowz>=g.maph) rowz=g.maph-1;
    if ((cola<=colz)&&(rowa<=rowz)) {
      const uint8_t *maprow=g.map+rowa*g.mapw+cola;
      int row=rowa;
      for (;row<=rowz;row++,maprow+=g.mapw) {
        const uint8_t *mapp=maprow;
        int col=cola;
        for (;col<=colz;col++,mapp++) {
          uint8_t physics=g.physics[*mapp];
          switch (physics) {
            case NS_physics_solid: {
                double bl=col,bt=row;
                double br=bl+1.0,bb=bt+1.0;
                CHECKBOX(bl,br,bt,bb)
              } break;
            case NS_physics_oneway: {
                // Oneways only count as a collision if we're moving down and were previously clear of it.
                if (dir==0x02) {
                  double bt=row;
                  double pvb=sprite->y+sprite->hbb-SMIDGE;
                  if (pvb<=bt) {
                    ny=bt-sprite->hbb;
                    REHITBOX
                  }
                }
              } break;
          }
        }
      }
    }
  }
  
  /* Check other solids.
   */
  {
    struct sprite **otherp=g.spritev;
    int i=g.spritec;
    for (;i-->0;otherp++) {
      struct sprite *other=*otherp;
      if (other->defunct||!other->solid) continue;
      if (other==sprite) continue;
      double ol=other->x+other->hbl;
      double or=other->x+other->hbr;
      double ot=other->y+other->hbt;
      double ob=other->y+other->hbb;
      CHECKBOX(ol,or,ot,ob)
    }
  }
  
  /* If the motion ended up zero or negative, don't commit it, and return zero.
   */
  switch (dir) {
    case 0x40: if (ny>=sprite->y) return 0; break;
    case 0x10: if (nx>=sprite->x) return 0; break;
    case 0x08: if (nx<=sprite->x) return 0; break;
    case 0x02: if (ny<=sprite->y) return 0; break;
  }
  
  /* OK, commit it and return 1.
   */
  #undef REHITBOX
  #undef CHECKBOX
  sprite->x=nx;
  sprite->y=ny;
  return 1;
}
