/* sprite.h
 */
 
#ifndef SPRITE_H
#define SPRITE_H

struct sprite;
struct sprite_type;

struct sprite {
  const struct sprite_type *type;
  int rid;
  const uint8_t *arg,*cmd;
  int argc,cmdc;
  double x,y;
  int defunct;
  uint8_t tileid,xform;
  int solid; // sprite_move stops us at walls and each other
  double hbl,hbr,hbt,hbb;
  int spooky; // Nonzero if we scare enemy soldiers.
};

struct sprite_type {
  const char *name;
  int objlen;
  void (*del)(struct sprite *sprite);
  int (*init)(struct sprite *sprite);
  void (*update)(struct sprite *sprite,double elapsed);
  void (*render)(struct sprite *sprite,int x,int y);
};

// Call at init for each resource.
int sprite_register(int rid,const void *src,int srcc);

// Prefer setting (sprite->defunct).
void sprite_del(struct sprite *sprite);

struct sprite *sprite_spawn(double x,double y,int rid,const void *arg,int argc);

void sprites_update(double elapsed);

struct sprite *any_sprite_of_type(const struct sprite_type *type);

const struct sprite_type *sprite_type_by_id(int id);

#define _(tag) extern const struct sprite_type sprite_type_##tag;
FOR_EACH_SPRTYPE
#undef _

void sprite_hero_get_dead(struct sprite *sprite,struct sprite *assailant);

/* Move with instant collision correction.
 * Returns nonzero if fully stopped due to collision.
 */
int sprite_move(struct sprite *sprite,double dx,double dy);

#endif
