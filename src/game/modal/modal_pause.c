#include "game/licensetoilluse.h"

#define LABEL_LIMIT 3

struct pause {
  int boxx,boxy,boxw,boxh;
  struct label {
    int texid,x,y,w,h;
    int strix;
  } labelv[LABEL_LIMIT];
  int labelc;
  int labelp;
};

/* Delete.
 */
 
void pause_del(struct pause *pause) {
  if (!pause) return;
  struct label *label=pause->labelv;
  int i=pause->labelc;
  for (;i-->0;label++) {
    egg_texture_del(label->texid);
  }
  free(pause);
}

/* Add label.
 */
 
static struct label *pause_add_label(struct pause *pause,int strix) {
  if (pause->labelc>=LABEL_LIMIT) return 0;
  struct label *label=pause->labelv+pause->labelc++;
  memset(label,0,sizeof(struct label));
  label->strix=strix;
  const char *src=0;
  int srcc=text_get_string(&src,1,strix);
  label->texid=font_render_to_texture(0,g.font,src,srcc,FBW,FBH,0xffffffff);
  egg_texture_get_size(&label->w,&label->h,label->texid);
  label->x=(FBW>>1)-(label->w>>1);
  return label;
}

/* New.
 */
 
struct pause *pause_new() {
  struct pause *pause=calloc(1,sizeof(struct pause));
  if (!pause) return 0;
  
  int y=0;
  struct label *label;
  if (label=pause_add_label(pause,20)) {
    if (label->w>pause->boxw) pause->boxw=label->w;
    label->y=y;
    y+=label->h;
  }
  if (label=pause_add_label(pause,21)) {
    if (label->w>pause->boxw) pause->boxw=label->w;
    label->y=y;
    y+=label->h;
  }
  if (label=pause_add_label(pause,22)) {
    if (label->w>pause->boxw) pause->boxw=label->w;
    label->y=y;
    y+=label->h;
  }
  pause->boxw+=4;
  pause->boxh=y+3;
  pause->boxx=(FBW>>1)-(pause->boxw>>1);
  pause->boxy=(FBH>>1)-(pause->boxh>>1);
  
  int i=pause->labelc;
  for (label=pause->labelv;i-->0;label++) {
    label->y+=pause->boxy+2;
  }
  
  return pause;
}

/* Move cursor.
 */
 
static void pause_move(struct pause *pause,int d) {
  if (pause->labelc<1) return;
  pause->labelp+=d;
  if (pause->labelp<0) pause->labelp=pause->labelc-1;
  else if (pause->labelp>=pause->labelc) pause->labelp=0;
}

/* Activate.
 */
 
static int pause_activate(struct pause *pause) {
  if ((pause->labelp>=0)&&(pause->labelp<pause->labelc)) {
    g.poison_input|=EGG_BTN_SOUTH;
    struct label *label=pause->labelv+pause->labelp;
    switch (label->strix) {
      case 20: return 0; // Resume
      case 21: { // Restart
          g.resetclock=FADE_OUT_TIME;
        } return 0;
      case 22: { // Main menu
          g.hello=hello_new();
        } return 0;
    }
  }
  return 1;
}

/* Update.
 */
 
int pause_update(struct pause *pause,double elapsed) {

  if ((g.input&EGG_BTN_AUX1)&&!(g.pvinput&EGG_BTN_AUX1)) {
    return 0;
  }
  
  if ((g.input&EGG_BTN_UP)&&!(g.pvinput&EGG_BTN_UP)) pause_move(pause,-1);
  else if ((g.input&EGG_BTN_DOWN)&&!(g.pvinput&EGG_BTN_DOWN)) pause_move(pause,1);
  else if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) return pause_activate(pause);
  
  return 1;
}

/* Render.
 */
 
void pause_render(struct pause *pause) {
  graf_fill_rect(&g.graf,pause->boxx,pause->boxy,pause->boxw,pause->boxh,0x000000ff);
  if ((pause->labelp>=0)&&(pause->labelp<pause->labelc)) {
    struct label *label=pause->labelv+pause->labelp;
    graf_fill_rect(&g.graf,pause->boxx,label->y-1,pause->boxw,label->h+1,0x000080ff);
  }
  struct label *label=pause->labelv;
  int i=pause->labelc;
  for (;i-->0;label++) {
    graf_set_input(&g.graf,label->texid);
    graf_decal(&g.graf,label->x,label->y,0,0,label->w,label->h);
  }
}
