#include "game/licensetoilluse.h"

static const uint8_t default_sprite_arg[4]={0};

/* Private resource registry.
 * Since we know the tid, we put the sprite type id there instead.
 */
 
static struct rom_entry *sprresv=0;
static int sprresc=0,sprresa=0;
  
int sprite_register(int rid,const void *src,int srcc) {
  if ((rid<1)||(rid>0xffff)) return -1;
  if ((sprresc>0)&&(rid<=sprresv[sprresc-1].rid)) return -1;
  if (sprresc>=sprresa) {
    int na=sprresa+16;
    if (na>INT_MAX/sizeof(struct rom_entry)) return -1;
    void *nv=realloc(sprresv,sizeof(struct rom_entry)*na);
    if (!nv) return -1;
    sprresv=nv;
    sprresa=na;
  }
  
  int tid=0;
  struct cmdlist_reader reader;
  if (sprite_reader_init(&reader,src,srcc)<0) return -1;
  struct cmdlist_entry cmd;
  while (cmdlist_reader_next(&cmd,&reader)>0) {
    if (cmd.opcode==CMD_sprite_type) {
      tid=(cmd.arg[0]<<8)|cmd.arg[1];
      break;
    }
  }
  if (!sprite_type_by_id(tid)) {
    fprintf(stderr,"sprite:%d does not declare a type.\n",rid);
    return -1;
  }
  
  struct rom_entry *sprres=sprresv+sprresc++;
  sprres->tid=tid;
  sprres->rid=rid;
  sprres->v=src;
  sprres->c=srcc;
  return 0;
}

static struct rom_entry *sprite_get_resource(int rid) {
  int lo=0,hi=sprresc;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
    const struct rom_entry *q=sprresv+ck;
         if (rid<q->rid) hi=ck;
    else if (rid>q->rid) lo=ck+1;
    else return sprresv+ck;
  }
  return 0;
}

/* Delete.
 */
 
void sprite_del(struct sprite *sprite) {
  if (!sprite) return;
  if (sprite->type->del) sprite->type->del(sprite);
  free(sprite);
}

/* Spawn.
 */

struct sprite *sprite_spawn(double x,double y,int rid,const void *arg,int argc) {
  struct rom_entry *res=sprite_get_resource(rid);
  if (!res) {
    fprintf(stderr,"sprite:%d not found\n",rid);
    return 0;
  }
  const struct sprite_type *type=sprite_type_by_id(res->tid);
  if (!type) {
    fprintf(stderr,"sprtype %d not defined (sprite:%d)\n",res->tid,rid);
    return 0;
  }
  struct cmdlist_reader reader;
  if (sprite_reader_init(&reader,res->v,res->c)<0) return 0;
  
  struct sprite *sprite=calloc(1,type->objlen);
  if (!sprite) return 0;
  sprite->type=type;
  sprite->rid=rid;
  if (arg) {
    sprite->arg=arg;
    sprite->argc=argc;
  } else {
    sprite->arg=default_sprite_arg;
    sprite->argc=4;
  }
  sprite->cmd=res->v;
  sprite->cmdc=res->c;
  sprite->x=x;
  sprite->y=y;
  
  struct cmdlist_entry cmd;
  while (cmdlist_reader_next(&cmd,&reader)>0) {
    switch (cmd.opcode) {
      case CMD_sprite_tile: {
          sprite->tileid=cmd.arg[0];
          sprite->xform=cmd.arg[1];
        } break;
      case CMD_sprite_solid: {
          sprite->solid=1;
          sprite->hbl=((int8_t)cmd.arg[0])/(double)NS_sys_tilesize;
          sprite->hbr=((int8_t)cmd.arg[1])/(double)NS_sys_tilesize;
          sprite->hbt=((int8_t)cmd.arg[2])/(double)NS_sys_tilesize;
          sprite->hbb=((int8_t)cmd.arg[3])/(double)NS_sys_tilesize;
        } break;
    }
  }
  
  if (type->init&&(type->init(sprite)<0)) {
    sprite_del(sprite);
    return 0;
  }
  
  if (g.spritec>=g.spritea) {
    int na=g.spritea+32;
    if (na>INT_MAX/sizeof(void*)) { sprite_del(sprite); return 0; }
    void *nv=realloc(g.spritev,sizeof(void*)*na);
    if (!nv) { sprite_del(sprite); return 0; }
    g.spritev=nv;
    g.spritea=na;
  }
  g.spritev[g.spritec++]=sprite;
  
  return sprite;
}

/* Update all.
 */
 
void sprites_update(double elapsed) {
  int i;
  struct sprite **p;
  struct sprite *sprite;

  /* Capture the hero; lots of other sprites are interested in him.
   */
  g.hero=0;
  for (i=g.spritec,p=g.spritev;i-->0;p++) {
    sprite=*p;
    if (sprite->defunct) continue;
    if (sprite->type==&sprite_type_hero) {
      g.hero=sprite;
      break;
    }
  }
  
  /* Iterate backward, do not retain a pointer, and skip defunct.
   * Should be safe to add sprites during iteration.
   * And of course "removing" is safe; that just means setting the defunct flag.
   */
  for (i=g.spritec;i-->0;) {
    sprite=g.spritev[i];
    if (sprite->defunct) continue;
    if (sprite->type->update) sprite->type->update(sprite,elapsed);
  }
  
  /* Drop any defunct sprites.
   * This time it's safe to retain a pointer.
   */
  for (i=g.spritec,p=g.spritev+g.spritec-1;i-->0;p--) {
    sprite=*p;
    if (!sprite->defunct) continue;
    g.spritec--;
    memmove(p,p+1,sizeof(void*)*(g.spritec-i));
    sprite_del(sprite);
  }
  g.hero=0;
}

/* Search sprites.
 */

struct sprite *any_sprite_of_type(const struct sprite_type *type) {
  struct sprite **p=g.spritev;
  int i=g.spritec;
  for (;i-->0;p++) {
    struct sprite *sprite=*p;
    if (sprite->defunct) continue;
    if (sprite->type!=type) continue;
    return sprite;
  }
  return 0;
}

/* Type registry.
 */

const struct sprite_type *sprite_type_by_id(int id) {
  switch (id) {
    #define _(tag) case NS_sprtype_##tag: return &sprite_type_##tag;
    FOR_EACH_SPRTYPE
    #undef _
  }
  return 0;
}
