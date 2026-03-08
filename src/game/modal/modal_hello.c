#include "game/licensetoilluse.h"

#define LABEL_LIMIT 3

struct hello {
  double herox,heroy;
  double heroanimclock;
  int heroanimframe;
  int heroposed;
  double heroposeclock;
  struct label {
    int texid,x,y,w,h;
  } labelv[LABEL_LIMIT];
  int labelc;
};

/* Delete.
 */
 
void hello_del(struct hello *hello) {
  if (!hello) return;
  struct label *label=hello->labelv;
  int i=hello->labelc;
  for (;i-->0;label++) {
    egg_texture_del(label->texid);
  }
  free(hello);
}

/* Add label.
 */
 
static struct label *hello_add_label(struct hello *hello,int strix) {
  if (hello->labelc>=LABEL_LIMIT) return 0;
  struct label *label=hello->labelv+hello->labelc++;
  memset(label,0,sizeof(struct label));
  const char *src=0;
  int srcc=text_get_string(&src,1,strix);
  label->texid=font_render_to_texture(0,g.font,src,srcc,FBW,FBH,0xc0c0c0ff);
  egg_texture_get_size(&label->w,&label->h,label->texid);
  label->x=(FBW>>1)-(label->w>>1);
  return label;
}

/* New.
 */
 
struct hello *hello_new() {
  struct hello *hello=calloc(1,sizeof(struct hello));
  if (!hello) return 0;
  
  hello->herox=-16.0;
  hello->heroy=FBH>>1;
  
  hello_add_label(hello,6);
  hello_add_label(hello,7);
  hello_add_label(hello,8);
  
  // Line up labels against the bottom.
  int y=FBH;
  struct label *label=hello->labelv+hello->labelc-1;
  int i=hello->labelc;
  for (;i-->0;label--) {
    y-=label->h;
    label->y=y;
    y-=1;
  }
  
  egg_play_song(1,RID_song_illusion,1,1.0,0.0);
  return hello;
}

/* Update.
 */
 
int hello_update(struct hello *hello,double elapsed) {

  if (hello->heroposeclock>0.0) {
    hello->heroposeclock-=elapsed;
  } else {
    hello->herox+=20.0*elapsed;
    if (hello->herox>340.0) {
      hello->herox=-16.0;
      hello->heroposed=0;
      hello->heroposeclock=0.0;
    } else if (!hello->heroposed&&(hello->herox>=160.0)) {
      hello->heroposed=1;
      hello->heroposeclock=2.0;
    }
    if ((hello->heroanimclock-=elapsed)<=0.0) {
      hello->heroanimclock+=0.200;
      if (++(hello->heroanimframe)>=4) hello->heroanimframe=0;
    }
  }
  
  if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) {
    return 0;
  }

  return 1;
}

/* Render.
 */
 
void hello_render(struct hello *hello) {
  graf_fill_rect(&g.graf,0,0,FBW,FBH,0x000000ff);
  
  graf_set_image(&g.graf,RID_image_hello);
  int herox=(int)hello->herox;
  int heroy=(int)hello->heroy;
  graf_decal(&g.graf,herox-16,heroy-16,80,0,32,32);
  uint8_t tileid=0x10;
  if (hello->heroposeclock>0.0) {
    tileid+=3;
    graf_tile(&g.graf,herox+16,heroy+4,tileid+1,0);
    graf_tile(&g.graf,herox+16,heroy-12,tileid-0x10+1,0);
  } else {
    switch (hello->heroanimframe) {
      case 1: tileid+=1; break;
      case 3: tileid+=2; break;
    }
  }
  graf_tile(&g.graf,herox,heroy+4,tileid,0);
  graf_tile(&g.graf,herox,heroy-12,tileid-0x10,0);
  
  int titlex=0,titley=32,titlew=224,titleh=32;
  graf_decal(&g.graf,(FBW>>1)-(titlew>>1),20,titlex,titley,titlew,titleh);
  
  struct label *label=hello->labelv;
  int i=hello->labelc;
  for (;i-->0;label++) {
    graf_set_input(&g.graf,label->texid);
    graf_decal(&g.graf,label->x,label->y,0,0,label->w,label->h);
  }
}
