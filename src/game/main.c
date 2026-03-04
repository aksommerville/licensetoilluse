#include "licensetoilluse.h"

struct g g={0};

void egg_client_quit(int status) {
}

static void load_tilesheet(const void *src,int srcc) {
  struct tilesheet_reader reader;
  if (tilesheet_reader_init(&reader,src,srcc)<0) return;
  struct tilesheet_entry entry;
  while (tilesheet_reader_next(&entry,&reader)>0) {
    if (entry.tableid!=NS_tilesheet_physics) continue;
    memcpy(g.physics,entry.v+entry.tileid,entry.c);
  }
}

/* Init.
 */

int egg_client_init() {

  int fbw=0,fbh=0;
  egg_texture_get_size(&fbw,&fbh,1);
  if ((fbw!=FBW)||(fbh!=FBH)) {
    fprintf(stderr,"Framebuffer size mismatch! metadata=%dx%d header=%dx%d\n",fbw,fbh,FBW,FBH);
    return -1;
  }

  g.romc=egg_rom_get(0,0);
  if (!(g.rom=malloc(g.romc))) return -1;
  egg_rom_get(g.rom,g.romc);
  text_set_rom(g.rom,g.romc);
  
  struct rom_reader reader;
  if (rom_reader_init(&reader,g.rom,g.romc)<0) return -1;
  struct rom_entry res;
  while (rom_reader_next(&res,&reader)>0) {
    switch (res.tid) {
      case EGG_TID_tilesheet: if (res.rid==RID_tilesheet_graphics) load_tilesheet(res.v,res.c); break;
      case EGG_TID_sprite: if (sprite_register(res.rid,res.v,res.c)<0) return -1; break;
    }
  }

  srand_auto();

  if (start_scene(1)<0) return -1;

  return 0;
}

void egg_client_notify(int k,int v) {
}

/* Update.
 */

void egg_client_update(double elapsed) {

  g.pvinput=g.input;
  g.input=egg_input_get_one(0);

  sprites_update(elapsed);
  //TODO Check scene-end conditions.
}

/* Render.
 */

void egg_client_render() {
  graf_reset(&g.graf);
  
  // Going to try to keep all our graphics in this one tilesheet, and not use any non-texture ops.
  graf_set_image(&g.graf,RID_image_graphics);
  
  /* Determine camera position.
   * Maps must be at least the framebuffer size, so minimum 20x11 meters.
   * Nothing fancy, just center the hero and clamp to world edges.
   */
  struct sprite *hero=any_sprite_of_type(&sprite_type_hero);
  if (hero) {
    g.camerax=(int)(hero->x*NS_sys_tilesize)-(FBW>>1);
    g.cameray=(int)(hero->y*NS_sys_tilesize)-(FBH>>1);
    int xlimit=g.mapw*NS_sys_tilesize-FBW;
    int ylimit=g.maph*NS_sys_tilesize-FBH;
    if (g.camerax<0) g.camerax=0; else if (g.camerax>xlimit) g.camerax=xlimit;
    if (g.cameray<0) g.cameray=0; else if (g.cameray>ylimit) g.cameray=ylimit;
  }
  
  /* Render map.
   * Maps are expected to provide opaque tiles everywhere.
   * (note that the one tilesheet also contains sprites).
   */
  int cola=g.camerax/NS_sys_tilesize; if (cola<0) cola=0;
  int rowa=g.cameray/NS_sys_tilesize; if (rowa<0) rowa=0;
  int colz=(g.camerax+FBW-1)/NS_sys_tilesize; if (colz>=g.mapw) colz=g.mapw-1;
  int rowz=(g.cameray+FBH-1)/NS_sys_tilesize; if (rowz>=g.maph) rowz=g.maph-1;
  if ((cola<=colz)&&(rowa<=rowz)) {
    int x0=cola*NS_sys_tilesize+(NS_sys_tilesize>>1)-g.camerax;
    int y=rowa*NS_sys_tilesize+(NS_sys_tilesize>>1)-g.cameray;
    const uint8_t *mrow=g.map+rowa*g.mapw+cola;
    int row=rowa;
    for (;row<=rowz;row++,mrow+=g.mapw,y+=NS_sys_tilesize) {
      int x=x0;
      const uint8_t *mp=mrow;
      int col=cola;
      for (;col<=colz;col++,mp++,x+=NS_sys_tilesize) {
        graf_tile(&g.graf,x,y,*mp,0);
      }
    }
  }
  
  /* Render sprites.
   * There shouldn't be anything defunct at this point, updating clears them at the end, but check anyway.
   */
  struct sprite **spritep=g.spritev;
  int i=g.spritec;
  for (;i-->0;spritep++) {
    struct sprite *sprite=*spritep;
    if (sprite->defunct) continue;
    int x=(int)(sprite->x*NS_sys_tilesize)-g.camerax;
    int y=(int)(sprite->y*NS_sys_tilesize)-g.cameray;
    if (sprite->type->render) {
      sprite->type->render(sprite,x,y);
    } else {
      graf_tile(&g.graf,x,y,sprite->tileid,sprite->xform);
    }
  }
  
  graf_flush(&g.graf);
}

/* Audio helpers.
 */
 
#define BLACKOUT_TIME 0.050
 
void lti_sound(int rid) {
  
  double now=egg_time_real();
  struct sound_blackout *got=0;
  struct sound_blackout *blackout=g.sound_blackoutv;
  struct sound_blackout *oldest=blackout;
  int i=g.sound_blackoutc;
  for (;i-->0;blackout++) {
    if (blackout->when<oldest->when) oldest=blackout;
    if (blackout->rid!=rid) continue;
    double elapsed=now-blackout->when;
    if (elapsed<BLACKOUT_TIME) return;
    got=blackout;
  }
  if (!got) {
    if (g.sound_blackoutc<SOUND_BLACKOUT_LIMIT) got=g.sound_blackoutv+g.sound_blackoutc++;
    else got=oldest;
  }
  
  got->rid=rid;
  got->when=now;
  egg_play_sound(rid,1.0,0.0);
}
  
void lti_song(int rid) {
  if (rid==g.songid_playing) return;
  g.songid_playing=rid;
  egg_play_song(1,rid,1,1.0,0.0);
}
