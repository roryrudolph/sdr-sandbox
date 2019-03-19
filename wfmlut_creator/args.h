
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

/**
 * TODO Document
 */
static char doc[] = "This program creates a VHDL module of a sine and cosine LUT.";

/**
 * TODO Document
 */
static char args_doc[] = "";

/**
 * TODO Document
 */
static struct argp_option options[] =
{
	{ "verbose", 'v', 0, 0, "Print debug messages", 0 },
	{ "ibits", 'i', "NUM", 0, "Number of integer bits in the LUT values. "
		"Default=" STR(DEFAULT_IBITS), 0 },
	{ "fbits", 'f', "NUM", 0, "Number of fractional bits in the LUT values. "
		"Default=" STR(DEFAULT_FBITS), 0 },
	{ "depth", 'd', "NUM", 0, "The depth of the LUT. "
		"Default=" STR(DEFAULT_DEPTH), 0 },
	{ "output", 'o', "FILE", 0, "The VHDL output file. If it already exists "
		"it will be overwritten. Default=" DEFAULT_OUTPUT_FILE, 0 },
	{ 0 }
};

/**
 * TODO Document
 */
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{

	cfg_t *cfg = (cfg_t *) state->input;

	switch (key)
	{
		case 'd':
			cfg->depth = atoi(arg);
			break;
		case 'f':
			cfg->fbits = atoi(arg);
			break;
		case 'i':
			cfg->ibits = atoi(arg);
			break;
		case 'o':
			snprintf(cfg->output_file, sizeof(cfg->output_file), "%s", arg);
			break;
		case 'v':
			cfg->verbose = 1;
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

/**
 * TODO Document
 */
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
