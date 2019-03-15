#ifndef CFG_H_
#define CFG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define _STR(x)  #x
#define STR(x)   _STR(x)

#define DEFAULT_VERBOSE      0
#define DEFAULT_WIDTH        16
#define DEFAULT_DEPTH        256
#define DEFAULT_OUTPUT_FILE  "wfmlut.vhd"

/**
 * TODO Document
 */
typedef struct cfg
{
	int verbose;
	int width;
	int depth;
	char output_file[32];
} cfg_t;

#ifdef __cplusplus
}
#endif

#endif /* CFG_H_ */
