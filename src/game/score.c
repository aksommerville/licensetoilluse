#include "licensetoilluse.h"

/*
Store under key "hiscore".
Store the five constituent fields and also the total.
Total could be calculated from the constituents but we'll also record it explicitly for validation.
And if our save file is ever read by external tools, that would be more convenient.

Field content is text.
Unsigned decimal integers separated by commas.
  TOTAL,TIMEMS,DEATHC,KILLC,ATTACKC,COINC
TOTAL must be 6 digits.
*/

/* Calculate total.
 */
 
static int score_calculate_total(const struct score *score) {

  // Start with 500k.
  int total=500000;
  
  // Up to 200k bonus based on clock.
  const int time_slowest=180000; // A generous three minutes for the time bonus.
  const int time_fastest= 90000; // Under 1:30 for a perfect score. This is very doable.
  const int time_bonus=200000;
  if (score->timems<=time_fastest) total+=time_bonus;
  else if (score->timems<time_slowest) total+=(int)(((double)time_bonus*(double)(time_slowest-score->timems))/(double)(time_slowest-time_fastest));
  
  // 100k bonus if deathless, otherwise -50k per death.
  if (!score->deathc) total+=100000;
  else total-=score->deathc*50000;
  
  // Modest bonus for each kill and penalty for each attack.
  // It's good to scare soldiers when you can. But spamming attacks, or dying to scare the soldiers again, you'll be penalized.
  // A perfect score is just barely possible with no attacks (tho you can't actually complete the game like that).
  total+=5000*score->killc-3500*score->attackc;
  
  // Up to 200k for collecting coins. Assume there are 20.
  total+=10000*score->coinc;
  
  // Clamp and we're done. Valid non-default scores are always 6 digits.
  if (total<100000) return 100000;
  if (total>999999) return 999999;
  return total;
}

/* Decode unsigned integer.
 * Consumes one trailing comma if present.
 * Fails if it breaches the upper limit.
 */
 
static int score_decode_int(int *dst,const char *src,int srcc,int limit) {
  if (!src||(srcc<1)||(src[0]<'0')||(src[0]>'9')) return -1;
  *dst=0;
  int srcp=0;
  while (srcp<srcc) {
    char digit=src[srcp++];
    if (digit==',') break;
    if ((digit<'0')||(digit>'9')) return -1;
    digit-='0';
    (*dst)*=10;
    (*dst)+=digit;
    if ((*dst)>limit) return -1;
  }
  return srcp;
}

/* Encode unsigned integer.
 * Fails if <0 or if there's not enough room plus one after.
 */
 
static int score_encode_int(char *dst,int dsta,int src) {
  if (src<0) return -1;
  int digitc=1,limit=10;
  while (limit<=src) { digitc++; if (limit>INT_MAX/10) break; limit*=10; }
  if (digitc>dsta-1) return -1;
  int i=digitc;
  for (;i-->0;src/=10) dst[i]='0'+src%10;
  return digitc;
}

/* Set default.
 */
 
static int score_default(struct score *score) {
  memset(score,0,sizeof(struct score));
  return 0;
}

/* Load or default.
 */
 
int score_load(struct score *score) {
  char src[32];
  int srcc=egg_store_get(src,sizeof(src),"hiscore",7);
  if ((srcc<1)||(srcc>sizeof(src))) return score_default(score);
  int srcp=0,err;
  if ((err=score_decode_int(&score->total,src+srcp,srcc-srcp,999999))<0) return score_default(score); srcp+=err;
  if ((err=score_decode_int(&score->timems,src+srcp,srcc-srcp,INT_MAX/10))<0) return score_default(score); srcp+=err;
  if ((err=score_decode_int(&score->deathc,src+srcp,srcc-srcp,99))<0) return score_default(score); srcp+=err;
  if ((err=score_decode_int(&score->killc,src+srcp,srcc-srcp,999))<0) return score_default(score); srcp+=err;
  if ((err=score_decode_int(&score->attackc,src+srcp,srcc-srcp,999))<0) return score_default(score); srcp+=err;
  if ((err=score_decode_int(&score->coinc,src+srcp,srcc-srcp,99))<0) return score_default(score); srcp+=err;
  if (score->total!=score_calculate_total(score)) return score_default(score);
  return 1;
}

/* Encode and save.
 */

int score_save(const struct score *score) {
  char serial[32];
  int serialc=0,err;
  if ((err=score_encode_int(serial+serialc,sizeof(serial)-serialc,score->total))<0) return -1; serialc+=err;
  serial[serialc++]=',';
  if ((err=score_encode_int(serial+serialc,sizeof(serial)-serialc,score->timems))<0) return -1; serialc+=err;
  serial[serialc++]=',';
  if ((err=score_encode_int(serial+serialc,sizeof(serial)-serialc,score->deathc))<0) return -1; serialc+=err;
  serial[serialc++]=',';
  if ((err=score_encode_int(serial+serialc,sizeof(serial)-serialc,score->killc))<0) return -1; serialc+=err;
  serial[serialc++]=',';
  if ((err=score_encode_int(serial+serialc,sizeof(serial)-serialc,score->attackc))<0) return -1; serialc+=err;
  serial[serialc++]=',';
  if ((err=score_encode_int(serial+serialc,sizeof(serial)-serialc,score->coinc))<0) return -1; serialc+=err;
  if (egg_store_set("hiscore",7,serial,serialc)<0) return -1;
  return 0;
}

/* Copy, sanitize, and compute total.
 */
 
static int score_bound(int v,int lo,int hi) {
  if (v<lo) return lo;
  if (v>hi) return hi;
  return v;
}

int score_from_globals(struct score *score) {
  score->timems=score_bound((int)(g.playtime*1000.0),0,INT_MAX/10);
  score->deathc=score_bound(g.deathc,0,99);
  score->killc=score_bound(g.killc,0,999);
  score->attackc=score_bound(g.attackc,0,999);
  score->coinc=score_bound(g.coinc,0,99);
  score->total=score_calculate_total(score);
  return 0;
}
