
#ifndef ARGS_H_
#define ARGS_H_

#include "cfg.h"
#include <argp.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

const char *argp_program_version = "0.1";
const char *argp_program_bug_address = "<rory.rudolph@outlook.com>";

static char doc[] = "This program creates a VHDL module of a sine and cosine LUT.";

static char args_doc[] = "";

static struct argp_option options[] =
{
	{ "verbose", 'v', 0, 0, "Print debug messages", 0 },
	{ "width", 'w', "NUM", 0, "The bit width of the LUT. "
		"Default=" STR(DEFAULT_WIDTH), 0 },
	{ "depth", 'd', "NUM", 0, "The depth of the LUT. "
		"Default=" STR(DEFAULT_DEPTH), 0 },
	{ 0 }
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{

	cfg_t *cfg = (cfg_t *) state->input;

	switch (key)
	{
		case 'd':
			cfg->depth = atoi(arg);
			break;
		case 'v':
			cfg->verbose = 1;
			break;
		case 'w':
			cfg->width = atoi(arg);
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp argp = 
{
	options,
	parse_opt,
	args_doc,
	doc,
	0, 
	0,
	0
};

#ifdef __cplusplus
}
#endif

#endif /* ARGS_H_ */
