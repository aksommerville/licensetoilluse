#include "licensetoilluse.h"

/* Map load commands.
 */
 
static void spawn_map_sprite(const uint8_t *arg) {
  double x=arg[0]+0.5;
  double y=arg[1]+0.5;
  int rid=(arg[2]<<8)|arg[3];
  const uint8_t *sarg=arg+4;
  struct sprite *sprite=sprite_spawn(x,y,rid,sarg,4);
  if (!sprite) return;
  if (sprite->type==&sprite_type_hero) {
    // Kind of a hack, take an initial guess at the camera position while loading.
    // Just get it close, don't bother clamping.
    g.camerax=(int)(x*NS_sys_tilesize)-(FBW>>1);
    g.cameray=(int)(y*NS_sys_tilesize)-(FBH>>1);
  }
}

/* Search goals.
 */
 
static int goalv_search(int x,int y) {
  int lo=0,hi=g.goalc;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
    const struct goal *q=g.goalv+ck;
         if (y<q->y) hi=ck;
    else if (y>q->y) lo=ck+1;
    else if (x<q->x) hi=ck;
    else if (x>q->x) lo=ck+1;
    else return ck;
  }
  return -lo-1;
}

/* Start scene.
 */
 
int start_scene(int mapid) {

  g.hero=0;
  while (g.spritec>0) {
    g.spritec--;
    sprite_del(g.spritev[g.spritec]);
  }

  const void *serial=0;
  int serialc=0;
  {
    struct rom_reader reader;
    if (rom_reader_init(&reader,g.rom,g.romc)<0) return -1;
    struct rom_entry res;
    while (rom_reader_next(&res,&reader)>0) {
      if ((res.tid==EGG_TID_map)&&(res.rid==mapid)) {
        serial=res.v;
        serialc=res.c;
        break;
      }
      if (res.tid>EGG_TID_map) break;
    }
  }
  struct map_res map_res;
  if (map_res_decode(&map_res,serial,serialc)<0) return -1;
  
  g.mapid=mapid;
  g.map=map_res.v;
  g.mapw=map_res.w;
  g.maph=map_res.h;
  g.mapcmd=map_res.cmd;
  g.mapcmdc=map_res.cmdc;
  g.goalc=0;
  g.heroqx=-1;
  g.heroqy=-1;
  g.goalclock=0.0;
  
  struct cmdlist_reader reader={.v=g.mapcmd,.c=g.mapcmdc};
  struct cmdlist_entry cmd;
  while (cmdlist_reader_next(&cmd,&reader)>0) {
    switch (cmd.opcode) {
      case CMD_map_goal: {
          int p=goalv_search(cmd.arg[0],cmd.arg[1]);
          if (p>=0) {
            fprintf(stderr,"map:%d duplicate goal at %d,%d\n",mapid,cmd.arg[0],cmd.arg[1]);
          } else if (g.goalc>=GOAL_LIMIT) {
            fprintf(stderr,"map:%d contains too many goals. Limit %d.\n",mapid,GOAL_LIMIT);
          } else {
            p=-p-1;
            struct goal *goal=g.goalv+p;
            memmove(goal+1,goal,sizeof(struct goal)*(g.goalc-p));
            g.goalc++;
            goal->x=cmd.arg[0];
            goal->y=cmd.arg[1];
          }
        } break;
      case CMD_map_sprite: spawn_map_sprite(cmd.arg); break;
    }
  }
  if (!g.goalc) {
    fprintf(stderr,"map:%d contains no goals, plan on staying a while.\n",mapid);
  }

  return 0;
}

/* Full reset.
 */
 
void reset_game() {
  g.have_ghost=0;
  g.have_rabbit=0;
  g.have_bird=0;
  g.rabbitc=0;
  g.birdc=0;
  memset(g.powerupv,0,sizeof(g.powerupv));
  g.playtime=0.0;
  g.deathc=0;
  g.killc=0;
  g.attackc=0;
  g.coinc=0;
  if (start_scene(1)<0) {
    fprintf(stderr,"Failed to start map:1!\n");
    egg_terminate(1);
  }
  egg_play_song(1,RID_song_double_oh_no,1,1.0,0.0);
}

/* Scare foes.
 */
 
void lti_scare_foes(double x,double y,double dx) {
  //XXX Foes scare themselves now.
}

/* Set reset clock.
 */
 
void reset_soon() {
  g.resetclock=1.000;
}

/* Check goals.
 */
 
#define GOAL_TIME 0.250

void scene_update_goal(double elapsed) {
  int nqx=-1,nqy=-1;
  if (g.hero&&sprite_hero_get_seated(g.hero)) {
    nqx=(int)g.hero->x;
    nqy=(int)g.hero->y;
  }
  if ((nqx!=g.heroqx)||(nqy!=g.heroqy)) {
    int ongoal=(goalv_search(nqx,nqy)>=0);
    if (!ongoal) {
      g.goalclock=0.0;
    } else if (g.goalclock<=0.0) {
      g.goalclock=0.001;
    }
    g.heroqx=nqx;
    g.heroqy=nqy;
  }
  if (g.goalclock>0.0) {
    if (((g.goalclock+=elapsed)>=GOAL_TIME)&&(g.resetclock<=0.0)) {
      g.resetclock=FADE_OUT_TIME;
    }
  }
}
