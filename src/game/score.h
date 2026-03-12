/* score.h
 */
 
#ifndef SCORE_H
#define SCORE_H

struct score {
  int total; // 0 or 100000..999999
  int timems; // Unlimited.
  int deathc; // 0..99
  int killc; // 0..999. Can exceed sprite count because you can kill then die then kill again.
  int attackc; // 0..999
  int coinc; // 0..99
};

/* Read from storage and return >0.
 * If missing or invalid, set defaults and return 0.
 */
int score_load(struct score *score);

/* Encode and write to storage.
 */
int score_save(const struct score *score);

/* Mostly just copies fields from (g) into (score).
 * Also calculates the total, which formula only we are privy to.
 * This assumes that the game was completed, so its total will always be at least 100k.
 */
int score_from_globals(struct score *score);

#endif
