/* sprite_soldier.c
 * Your basic bad guy with a gun.
 */
 
#include "game/licensetoilluse.h"

#define GRAVITY 7.0
#define DISAPPEAR_MARGIN 100 /* pixels */
#define SPOOK_SPEED 8.0 /* m/s */
#define SPOOK_VISIBILITY 4.0 /* Horizontal meters. */
#define FIRE_VISIBILITY 7.0 /* Horizontal meters. */
#define STUCK_DISAPPEAR_TIME 2.0
 
struct sprite_soldier {
  struct sprite hdr;
  double shocked; // Counts down during the initial surprise.
  int spooked; // Direction of travel, if spooked.
  double animclock;
  int animframe;
  double recoil; // Counts down after firing rifle.
  double fireclock; // Counts down while hero in the fire box, before firing rifle.
  double stuck; // Counts up while spooked and blocked horizontally.
  double walkclock; // Counts down while idly walking.
  double walkdx;
  double chillclock; // Counts down when not idly walking.
  
  // Constants from sprite resource:
  double walk_time_min,walk_time_max;
  double chill_time_min,chill_time_max;
  double walk_speed;
  double fire_time;
};

#define SPRITE ((struct sprite_soldier*)sprite)

/* Init.
 */

int _soldier_init(struct sprite *sprite) {

  // Defaults may be overridden by resource:
  SPRITE->walk_time_min= 1.000;
  SPRITE->walk_time_max= 2.000;
  SPRITE->chill_time_min=1.000;
  SPRITE->chill_time_max=2.000;
  SPRITE->walk_speed=    3.000;
  SPRITE->fire_time=     0.750;

  struct cmdlist_reader reader;
  if (sprite_reader_init(&reader,sprite->cmd,sprite->cmdc)>=0) {
    struct cmdlist_entry cmd;
    while (cmdlist_reader_next(&cmd,&reader)>0) {
      switch (cmd.opcode) {
        case CMD_sprite_soldier: {
            SPRITE->walk_time_max=cmd.arg[0]/16.0;
            SPRITE->walk_time_min=SPRITE->walk_time_max*0.5;
            SPRITE->chill_time_max=cmd.arg[1]/16.0;
            SPRITE->chill_time_min=SPRITE->chill_time_max*0.5;
            SPRITE->walk_speed=cmd.arg[2]/16.0;
            SPRITE->fire_time=cmd.arg[3]/16.0;
          } break;
      }
    }
  }
  
  return 0;
}

/* Nonzero if there's something in our face direction that should scare us.
 * Face direction is based on xform: 0=right, XREV=left.
 */
 
static int soldier_sees_something_spooky(struct sprite *sprite) {

  // Compute the visibility box.
  double vist=sprite->y-1.0;
  double visb=sprite->y+1.0;
  double visl,visr;
  if (sprite->xform&EGG_XFORM_XREV) {
    visr=sprite->x+0.5;
    visl=visr-SPOOK_VISIBILITY;
  } else {
    visl=sprite->x-0.5;
    visr=visl+SPOOK_VISIBILITY;
  }

  // Anything spooky in there?
  struct sprite **otherp=g.spritev;
  int i=g.spritec;
  for (;i-->0;otherp++) {
    struct sprite *other=*otherp;
    if (other->defunct) continue;
    if (!other->spooky) continue;
    if (other->x<visl) continue;
    if (other->x>visr) continue;
    if (other->y<vist) continue;
    if (other->y>visb) continue;
    return 1;
  }
  return 0;
}

/* Nonzero if the hero is in our fire range.
 */
 
static int solider_sees_something_shooty(struct sprite *sprite) {
  if (!g.hero) return 0;
  double dy=g.hero->y-sprite->y;
  if ((dy<-1.0)||(dy>1.0)) return 0;
  double dx=g.hero->x-sprite->x;
  if (sprite->xform&EGG_XFORM_XREV) {
    if (dx<-FIRE_VISIBILITY) return 0;
    if (dx>0.0) return 0;
  } else {
    if (dx<0.0) return 0;
    if (dx>FIRE_VISIBILITY) return 0;
  }
  return 1;
}

/* Fire my rifle.
 */
 
static void soldier_fire(struct sprite *sprite) {
  SPRITE->recoil=0.500;
  SPRITE->fireclock=0.0;
  SPRITE->walkclock=0.0;
  lti_sound(RID_sound_rifle);
  double bulletx=sprite->x;
  double bullety=sprite->y;
  if (sprite->xform&EGG_XFORM_XREV) bulletx-=0.5;
  else bulletx+=0.5;
  uint8_t arg[4]={sprite->xform,0,0,0};
  struct sprite *bullet=sprite_spawn(bulletx,bullety,RID_sprite_bullet,arg,sizeof(arg));
  if (!bullet) return;
}

/* How far can I walk in each direction?
 */
 
static void soldier_measure_freedom(double *freel,double *freer,struct sprite *sprite) {
  *freel=*freer=0.0;
  
  /* First walk outward gridwise.
   * Require solid below and two rows vacant above.
   * Oneways are permitted anywhere, since we're only moving horizontally.
   * Zero freedom if OOB, and we can terminate when detected.
   * This will assume that the sprite is centered in his cell horizontally, it's fine, doesn't need to be exact.
   */
  int x=(int)sprite->x;
  if ((x<0)||(x>=g.mapw)) return;
  int y=(int)sprite->y+1; // (y) is the solid row, must have two vacants above it.
  if ((y<2)||(y>=g.maph)) return;
  const uint8_t *solidrow=g.map+y*g.mapw;
  const uint8_t *footrow=solidrow-g.mapw;
  const uint8_t *headrow=footrow-g.mapw;
  double ledge=sprite->x+sprite->hbl;
  double redge=sprite->x+sprite->hbr;
  int qx=x;
  for (;qx-->0;) {
    if ((g.physics[solidrow[qx]]!=NS_physics_solid)&&(g.physics[solidrow[qx]]!=NS_physics_oneway)) break;
    if ((g.physics[footrow[qx]]!=NS_physics_vacant)&&(g.physics[footrow[qx]]!=NS_physics_oneway)) break;
    if ((g.physics[headrow[qx]]!=NS_physics_vacant)&&(g.physics[headrow[qx]]!=NS_physics_oneway)) break;
    ledge=qx;
  }
  for (qx=x+1;qx<g.mapw;qx++) {
    if ((g.physics[solidrow[qx]]!=NS_physics_solid)&&(g.physics[solidrow[qx]]!=NS_physics_oneway)) break;
    if ((g.physics[footrow[qx]]!=NS_physics_vacant)&&(g.physics[footrow[qx]]!=NS_physics_oneway)) break;
    if ((g.physics[headrow[qx]]!=NS_physics_vacant)&&(g.physics[headrow[qx]]!=NS_physics_oneway)) break;
    redge=qx+1.0;
  }
  *freel=sprite->x+sprite->hbl-ledge;
  *freer=redge-(sprite->x+sprite->hbr);
  if ((*freel<1.0)&&(*freer<1.0)) return;
  
  /* Reduce both freedoms if any solid sprite intersects.
   */
  double t=sprite->y+sprite->hbt;
  double b=sprite->y+sprite->hbb;
  struct sprite **otherp=g.spritev;
  int i=g.spritec;
  for (;i-->0;otherp++) {
    struct sprite *other=*otherp;
    if (other->defunct) continue;
    if (!other->solid) continue;
    if (other==sprite) continue;
    if (other->y+other->hbb<=t) continue;
    if (other->y+other->hbt>=b) continue;
    double ol=other->x+other->hbl;
    double or=other->x+other->hbr;
    if ((or<=sprite->x)&&(or>sprite->x+sprite->hbl-(*freel))) *freel=sprite->x+sprite->hbl-or;
    if ((ol>=sprite->x)&&(ol<sprite->x+sprite->hbr+(*freer))) *freer=ol-(sprite->x+sprite->hbr);
  }
}

/* Nothing much happening. Set up to either walk a little, or stand still.
 */
 
static void soldier_choose_idle_activity(struct sprite *sprite) {
  SPRITE->walkclock=0.0;
  SPRITE->chillclock=0.0;
  
  char candidatev[3]; // [lrc]
  int candidatec=0;
  double freel,freer;
  soldier_measure_freedom(&freel,&freer,sprite);
  if (freel>1.0) candidatev[candidatec++]='l';
  if (freer>1.0) candidatev[candidatec++]='r';
  if (!candidatec||(SPRITE->chill_time_max>=0.250)) candidatev[candidatec++]='c';
  char action=candidatev[rand()%candidatec];
  
  double clocklimit=0.0;
  switch (action) {
    case 'l': {
        SPRITE->walkdx=-SPRITE->walk_speed;
        SPRITE->walkclock=(rand()&0xffff)/65535.0;
        SPRITE->walkclock=SPRITE->walk_time_min*(1.0-SPRITE->walkclock)+SPRITE->walk_time_max*SPRITE->walkclock;
        sprite->xform=EGG_XFORM_XREV;
        clocklimit=freel/SPRITE->walk_speed;
      } break;
    case 'r': {
        SPRITE->walkdx=SPRITE->walk_speed;
        SPRITE->walkclock=(rand()&0xffff)/65535.0;
        SPRITE->walkclock=SPRITE->walk_time_min*(1.0-SPRITE->walkclock)+SPRITE->walk_time_max*SPRITE->walkclock;
        sprite->xform=0;
        clocklimit=freer/SPRITE->walk_speed;
      } break;
    case 'c': {
        SPRITE->chillclock=(rand()&0xffff)/65535.0;
        SPRITE->chillclock=SPRITE->chill_time_min*(1.0-SPRITE->chillclock)+SPRITE->chill_time_max*SPRITE->chillclock;
        SPRITE->animclock=0.0;
        SPRITE->animframe=0;
      } break;
  }
  if (SPRITE->walkclock>clocklimit) SPRITE->walkclock=clocklimit;
}

/* Update.
 */

void _soldier_update(struct sprite *sprite,double elapsed) {

  /* Gravity always applies.
   */
  int falling=sprite_move(sprite,0.0,GRAVITY*elapsed);
  
  /* During shock, just tick the timer down.
   */
  if (SPRITE->shocked>0.0) {
    if ((SPRITE->shocked-=elapsed)>=0.0) return;
    sprite->xform^=EGG_XFORM_XREV;
  }

  /* If we're spooked, it's permanent.
   * Run until we're sufficiently far offscreen, and if we get far enough, disappear.
   */
  if (SPRITE->spooked) {
    if ((SPRITE->animclock-=elapsed)<=0.0) {
      SPRITE->animclock+=0.200;
      if (++(SPRITE->animframe)>=2) SPRITE->animframe=0;
    }
    if (falling) {
      SPRITE->stuck=0.0;
    } else {
      if (sprite_move(sprite,SPOOK_SPEED*SPRITE->spooked*elapsed,0.0)) {
        SPRITE->stuck=0.0;
      } else {
        if ((SPRITE->stuck+=elapsed)>STUCK_DISAPPEAR_TIME) {
          sprite->defunct=1;
        }
      }
    }
    int xp=sprite->x*NS_sys_tilesize;
    if ((xp<g.camerax-DISAPPEAR_MARGIN)||(xp>g.camerax+FBW+DISAPPEAR_MARGIN)) {
      sprite->defunct=1;
    }
    return;
  }

  /* Face the hero, if not walking.
   */
  if (g.hero&&(SPRITE->walkclock<=0.0)) {
    if (g.hero->x<sprite->x) sprite->xform=EGG_XFORM_XREV;
    else sprite->xform=0;
  }
  
  /* If we see a ghost, rabbit, or bird, get spooked.
   */
  if (soldier_sees_something_spooky(sprite)) {
    SPRITE->shocked=0.500;
    SPRITE->animframe=0;
    if (sprite->xform&EGG_XFORM_XREV) {
      SPRITE->spooked=1;
    } else {
      SPRITE->spooked=-1;
    }
    sprite_spawn(sprite->x,sprite->y,RID_sprite_rifle,0,0);
    return;
  }
  
  /* If the rifle was just fired, tick it down and do nothing else.
   */
  if (SPRITE->recoil>0.0) {
    if ((SPRITE->recoil-=elapsed)>0.0) return;
  }
  
  /* If the hero is in the fire box, tick down fireclock, and fire the rifle when it expires.
   * Don't stop walking, if we're doing that.
   */
  if (solider_sees_something_shooty(sprite)) {
    if (SPRITE->fireclock<=0.0) SPRITE->fireclock=SPRITE->fire_time;
    if ((SPRITE->fireclock-=elapsed)<=0.0) {
      soldier_fire(sprite);
      return;
    }
  } else {
    SPRITE->fireclock=0.0;
  }
  
  /* If we're idle-walking, carry on with that until time expires.
   */
  if (SPRITE->walkclock>0.0) {
    SPRITE->walkclock-=elapsed;
    sprite_move(sprite,SPRITE->walkdx*elapsed,0.0);
    if ((SPRITE->animclock-=elapsed)<=0.0) {
      SPRITE->animclock+=0.200;
      if (++(SPRITE->animframe)>=4) SPRITE->animframe=0;
    }
    return;
  }
  
  /* If we've decided to chill, chill.
   */
  if (SPRITE->chillclock>0.0) {
    SPRITE->chillclock-=elapsed;
    return;
  }
  
  /* Nothing else going on, either chill a bit or walk a bit.
   */
  soldier_choose_idle_activity(sprite);
}

/* Render.
 */

void _soldier_render(struct sprite *sprite,int x,int y) {
  uint8_t tileid=sprite->tileid;
  uint8_t xform=sprite->xform;
  
  if (SPRITE->shocked>0.0) {
    tileid+=4;
    
  } else if (SPRITE->spooked) {
    tileid+=5+SPRITE->animframe;
  
  } else if (SPRITE->recoil>0.0) {
    tileid+=3;
    
  } else if (SPRITE->walkclock>0.0) {
    switch (SPRITE->animframe) {
      case 1: tileid+=1; break;
      case 3: tileid+=2; break;
    }
  }
  
  graf_tile(&g.graf,x,y,tileid,xform);
  graf_tile(&g.graf,x,y-NS_sys_tilesize,tileid-0x10,xform);
}

/* Type definition.
 */

const struct sprite_type sprite_type_soldier={
  .name="soldier",
  .objlen=sizeof(struct sprite_soldier),
  .init=_soldier_init,
  .update=_soldier_update,
  .render=_soldier_render,
};
