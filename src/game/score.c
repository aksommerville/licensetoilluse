#include "licensetoilluse.h"

/*
Use two store fields:
  "hiscore": 6 decimal digits.
  "besttime": "MM:SS.mmm"
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
 
static int score_decode_int(int *dst,const char *src,int srcc) {
  if (!src||(srcc<1)||(src[0]<'0')||(src[0]>'9')) return -1;
  *dst=0;
  int srcp=0;
  while (srcp<srcc) {
    char digit=src[srcp++];
    if ((digit<'0')||(digit>'9')) return -1;
    digit-='0';
    (*dst)*=10;
    (*dst)+=digit;
    if (*dst>999999) return -1;
  }
  return srcp;
}

static int score_decode_time(int *dst,const char *src,int srcc) {
  if (srcc!=9) return -1;
  if ((src[2]!=':')||(src[4]!='.')) return -1;
  int m10=src[0]-'0';
  int m01=src[1]-'0';
  int s10=src[3]-'0';
  int s01=src[4]-'0';
  int ms100=src[6]-'0';
  int ms010=src[7]-'0';
  int ms001=src[8]-'0';
  if ((m10<0)||(m10>9)) return -1;
  if ((m01<0)||(m01>9)) return -1;
  if ((s10<0)||(s10>9)) return -1;
  if ((s01<0)||(s01>9)) return -1;
  if ((ms100<0)||(ms100>9)) return -1;
  if ((ms010<0)||(ms010>9)) return -1;
  if ((ms001<0)||(ms001>9)) return -1;
  int min=m10*10+m01;
  int sec=s10*10+s01;
  int ms=ms100*100+ms010*10+ms001;
  *dst=min*60000+sec*1000+ms;
  return 0;
}

/* Encode unsigned integer.
 * Fails if <0 or if there's not enough room plus one after.
 */
 
static int score_encode_int6(char *dst,int dsta,int src) {
  if (dsta<6) return -1;
  if (src<0) src=0;
  else if (src>999999) src=999999;
  dst[0]='0'+(src/100000)%10;
  dst[1]='0'+(src/ 10000)%10;
  dst[2]='0'+(src/  1000)%10;
  dst[3]='0'+(src/   100)%10;
  dst[4]='0'+(src/    10)%10;
  dst[5]='0'+(src       )%10;
  return 6;
}

static int score_encode_time(char *dst,int dsta,int src) {
  if (dsta<9) return -1;
  if (src<0) src=0;
  int ms=src%1000;
  int sec=src/1000;
  int min=sec/60;
  sec%=60;
  if (min>99) {
    min=sec=99;
    ms=999;
  }
  dst[0]='0'+min/10;
  dst[1]='0'+min%10;
  dst[2]=':';
  dst[3]='0'+sec/10;
  dst[4]='0'+sec%10;
  dst[5]='.';
  dst[6]='0'+ms/100;
  dst[7]='0'+(ms/10)%10;
  dst[8]='0'+ms%10;
  return 9;
}

/* Load or default.
 */
 
int score_load(struct score *score) {
  memset(score,0,sizeof(struct score));
  char hiscore[6],besttime[9];
  if (egg_store_get(hiscore,sizeof(hiscore),"hiscore",7)==sizeof(hiscore)) {
    if (score_decode_int(&score->total,hiscore,sizeof(hiscore))<0) score->total=0;
  }
  if (egg_store_get(besttime,sizeof(besttime),"besttime",8)==sizeof(besttime)) {
    if (score_decode_time(&score->timems,besttime,sizeof(besttime))<0) score->timems=0;
  }
  return 1;
}

/* Encode and save.
 */

int score_save(const struct score *score) {
  char serial[32];
  int serialc;
  if ((serialc=score_encode_int6(serial,sizeof(serial),score->total))>=0) {
    egg_store_set("hiscore",7,serial,serialc);
  }
  if ((serialc=score_encode_time(serial,sizeof(serial),score->timems))>=0) {
    egg_store_set("besttime",8,serial,serialc);
  }
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
