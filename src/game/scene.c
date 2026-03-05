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

/* Start scene.
 */
 
int start_scene(int mapid) {

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
  
  struct cmdlist_reader reader={.v=g.mapcmd,.c=g.mapcmdc};
  struct cmdlist_entry cmd;
  while (cmdlist_reader_next(&cmd,&reader)>0) {
    switch (cmd.opcode) {
      case CMD_map_sprite: spawn_map_sprite(cmd.arg); break;
    }
  }

  return 0;
}

/* Scare foes.
 */
 
void lti_scare_foes(double x,double y,double dx) {
  //TODO
}
