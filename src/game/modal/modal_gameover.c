#include "game/licensetoilluse.h"

#define LABEL_LIMIT 32

struct gameover {
  struct label {
    int texid,x,y,w,h;
    int strix;
  } labelv[LABEL_LIMIT];
  int labelc;
  int new_hiscore; // boolean
  int prev_hiscore; // total
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
  label->strix=strix;
  return label;
}

/* Add label as value for some existing label.
 */
 
static void gameover_add_value(struct gameover *gameover,int strix,const char *src,int srcc) {
  if (gameover->labelc>=LABEL_LIMIT) return;
  struct label *k=0;
  struct label *label=gameover->labelv;
  int i=gameover->labelc;
  for (;i-->0;label++) {
    if (label->strix==strix) {
      k=label;
      break;
    }
  }
  if (!k) return;
  label=gameover->labelv+gameover->labelc++;
  memset(label,0,sizeof(struct label));
  label->texid=font_render_to_texture(0,g.font,src,srcc,FBW,FBH,0xffffffff);
  egg_texture_get_size(&label->w,&label->h,label->texid);
  label->x=k->x+k->w+5;
  label->y=k->y;
}

/* Add value label for model inputs.
 */
 
static void gameover_add_value_int(struct gameover *gameover,int strix,int v) {
  char tmp[16];
  if (v<0) v=0;
  int tmpc=0;
  if (v>=1000000000) tmp[tmpc++]='0'+(v/1000000000)%10;
  if (v>= 100000000) tmp[tmpc++]='0'+(v/ 100000000)%10;
  if (v>=  10000000) tmp[tmpc++]='0'+(v/  10000000)%10;
  if (v>=   1000000) tmp[tmpc++]='0'+(v/   1000000)%10;
  if (v>=    100000) tmp[tmpc++]='0'+(v/    100000)%10;
  if (v>=     10000) tmp[tmpc++]='0'+(v/     10000)%10;
  if (v>=      1000) tmp[tmpc++]='0'+(v/      1000)%10;
  if (v>=       100) tmp[tmpc++]='0'+(v/       100)%10;
  if (v>=        10) tmp[tmpc++]='0'+(v/        10)%10;
  tmp[tmpc++]='0'+v%10;
  gameover_add_value(gameover,strix,tmp,tmpc);
}

static void gameover_add_value_ratio(struct gameover *gameover,int strix,int n,int d) {
  char tmp[16];
  if (n<0) n=0; else if (n>999) n=999;
  if (d<0) d=0; else if (d>999) d=999;
  int tmpc=0;
  if (n>=100) tmp[tmpc++]='0'+n/100;
  if (n>=10) tmp[tmpc++]='0'+(n/10)%10;
  tmp[tmpc++]='0'+n%10;
  tmp[tmpc++]='/';
  if (d>=100) tmp[tmpc++]='0'+d/100;
  if (d>=10) tmp[tmpc++]='0'+(d/10)%10;
  tmp[tmpc++]='0'+d%10;
  gameover_add_value(gameover,strix,tmp,tmpc);
}

static void gameover_add_value_time(struct gameover *gameover,int strix,int ms) {
  if (ms<0) ms=0;
  int sec=ms/1000; ms%=1000;
  int min=sec/60; sec%=60;
  if (min>99) {
    min=99;
    sec=99;
    ms=999;
  }
  char tmp[]={
    '0'+min/10,
    '0'+min%10,
    ':',
    '0'+sec/10,
    '0'+sec%10,
    '.',
    '0'+ms/100,
    '0'+(ms/10)%10,
    '0'+ms%10,
  };
  gameover_add_value(gameover,strix,tmp,sizeof(tmp));
}

/* New.
 */
 
struct gameover *gameover_new() {
  struct gameover *gameover=calloc(1,sizeof(struct gameover));
  if (!gameover) return 0;
  egg_play_song(1,0,1,1.0,0.0);
  
  struct score score;
  score_from_globals(&score);
  gameover->prev_hiscore=g.hiscore.total;
  if (score.total>g.hiscore.total) {
    gameover->new_hiscore=1;
    g.hiscore=score;
    score_save(&score);
  }
  fprintf(stderr,
    "%s:%d: Score: total=%d timems=%d deathc=%d killc=%d attackc=%d coinc=%d new_hiscore=%d prev_hiscore=%d\n",
    __FILE__,__LINE__,
    score.total,score.timems,score.deathc,score.killc,score.attackc,score.coinc,
    gameover->new_hiscore,gameover->prev_hiscore
  );
  
  struct label *label;
  // "You win!" centered near the top.
  if (label=gameover_add_label(gameover,9)) {
    label->y=10;
  }
  // Table keys align their right edges.
  int right=150,y=50;
  if (label=gameover_add_label(gameover,10)) {
    label->x=right-label->w;
    label->y=y;
    y+=label->h;
  }
  if (label=gameover_add_label(gameover,11)) {
    label->x=right-label->w;
    label->y=y;
    y+=label->h;
  }
  if (label=gameover_add_label(gameover,12)) {
    label->x=right-label->w;
    label->y=y;
    y+=label->h;
  }
  if (label=gameover_add_label(gameover,13)) {
    label->x=right-label->w;
    label->y=y;
    y+=label->h;
  }
  if (label=gameover_add_label(gameover,14)) {
    label->x=right-label->w;
    label->y=y;
    y+=label->h;
  }
  // Then a vertical gap, "New high score!" if applicable, and the stored best time.
  y+=10;
  if (gameover->new_hiscore) {
    if (label=gameover_add_label(gameover,15)) {
      label->y=y;
      y+=label->h;
    }
  } else {
    if (label=gameover_add_label(gameover,16)) {
      label->x=right-label->w;
      label->y=y;
      y+=label->h;
    }
  }
  if (label=gameover_add_label(gameover,17)) {
    label->x=right-label->w;
    label->y=y;
    y+=label->h;
  }
  // Then "Thanks for playing!" near the bottom.
  if (label=gameover_add_label(gameover,18)) {
    label->y=FBH-label->h-10;
  }
  
  /* Then create labels for the table values.
   * Was going to not use labels for these, and do a counting-up animation, but meh.
   */
  gameover_add_value_int(gameover,10,score.total);
  gameover_add_value_time(gameover,11,score.timems);
  gameover_add_value_int(gameover,12,score.deathc);
  gameover_add_value_ratio(gameover,13,score.killc,score.attackc);
  gameover_add_value_ratio(gameover,14,score.coinc,20);
  gameover_add_value_int(gameover,16,g.hiscore.total);
  gameover_add_value_time(gameover,17,g.hiscore.timems);
  
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
