#include "game/licensetoilluse.h"

#define LABEL_LIMIT 4

struct gameover {
  struct label {
    int texid,x,y,w,h;
  } labelv[LABEL_LIMIT];
  int labelc;
};

/* Delete.
 */
 
void gameover_del(struct gameover *gameover) {
  if (!gameover) return;
  struct label *label=gameover->labelv;
  int i=gameover->labelc;
  for (;i-->0;label++) {
    egg_texture_del(label->texid);
  }
  free(gameover);
}

/* Add label.
 */
 
static struct label *gameover_add_label(struct gameover *gameover,int strix) {
  if (gameover->labelc>=LABEL_LIMIT) return 0;
  struct label *label=gameover->labelv+gameover->labelc++;
  memset(label,0,sizeof(struct label));
  const char *src=0;
  int srcc=text_get_string(&src,1,strix);
  label->texid=font_render_to_texture(0,g.font,src,srcc,FBW,FBH,0xffffffff);
  egg_texture_get_size(&label->w,&label->h,label->texid);
  label->x=(FBW>>1)-(label->w>>1);
  return label;
}

/* New.
 */
 
struct gameover *gameover_new() {
  struct gameover *gameover=calloc(1,sizeof(struct gameover));
  if (!gameover) return 0;
  egg_play_song(1,0,1,1.0,0.0);
  
  gameover_add_label(gameover,9);
  
  // Center labels vertically. They are already centered horizontally.
  int totalh=0,i=gameover->labelc;
  struct label *label=gameover->labelv;
  for (;i-->0;label++) totalh+=label->h+1;
  int y=(FBH>>1)-(totalh>>1);
  for (i=gameover->labelc,label=gameover->labelv;i-->0;label++) {
    label->y=y;
    y+=label->h+1;
  }
  
  
  return gameover;
}

/* Update.
 */
 
int gameover_update(struct gameover *gameover,double elapsed) {

  if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) {
    return 0;
  }

  return 1;
}

/* Render.
 */
 
void gameover_render(struct gameover *gameover) {
  graf_fill_rect(&g.graf,0,0,FBW,FBH,0x000000ff);
  struct label *label=gameover->labelv;
  int i=gameover->labelc;
  for (;i-->0;label++) {
    graf_set_input(&g.graf,label->texid);
    graf_decal(&g.graf,label->x,label->y,0,0,label->w,label->h);
  }
}
