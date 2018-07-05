/*
 * macros.h - macro definitions for Phantasia
 */

#define RND()		((double)Do_random() / 2147483647.0)
#define ROLL(BASE,INTERVAL)    floor(BASE + ((double)INTERVAL * RND()))
#define SGN(X)          ((X) < 0 ? -1 : 1)
/*
#define CIRCLE(X, Y)    floor(Do_distance(X, 0.0, Y, 0.0) / D_CIRCLE + 1)
*/
#define ANY(X)	((X) > 0 ? 1 : 0)
#define CRACKS()        ((c->player.circle > 25) && (c->player.circle < 30))
#define MAX(A, B)	((A) > (B) ? (A) : (B))
#define MIN(A, B)       ((A) < (B) ? (A) : (B))
#define CALCLEVEL(XP)   floor(sqrt(XP / 1800.0))
