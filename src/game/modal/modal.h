/* modal.h
 * Not generalizing anything.
 * We have a "hello" and a "gameover" modal. Main manages them.
 */
 
#ifndef MODAL_H
#define MODAL_H

struct hello;
struct gameover;

void hello_del(struct hello *hello);
struct hello *hello_new();
int hello_update(struct hello *hello,double elapsed); // zero to terminate
void hello_render(struct hello *hello);

void gameover_del(struct gameover *gameover);
struct gameover *gameover_new();
int gameover_update(struct gameover *gameover,double elapsed); // zero to terminate
void gameover_render(struct gameover *gameover);

#endif
