#ifndef CFG_H_
#define CFG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define _STR(x)  #x
#define STR(x)   _STR(x)

#define DEFAULT_VERBOSE  0
#define DEFAULT_WIDTH    16
#define DEFAULT_DEPTH    1024

typedef struct cfg
{
	int verbose;
	int width;
	int depth;
} cfg_t;

#ifdef __cplusplus
}
#endif

#endif /* CFG_H_ */
