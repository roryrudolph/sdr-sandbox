/*
 * This program produces a VHDL file for a sin and cosine wave lookup table.
 * Parameters for the program can be found with the --help flag at run-time.
 */

#include "args.h"
#include <stdio.h>
#include <math.h>

const char *libs =
	"library ieee;\n"
	"use ieee.std_logic_1164.all;\n"
	"use ieee.numeric_std.all;\n";

const char *entity =
	"entity wfmlut is\n"
	"	port (\n"
	"		addr : in std_logic_vector (%d downto 0);\n"
	"		sin  : out std_logic_vector (%d downto 0);\n"
	"		cos  : out std_logic_vector (%d downto 0)\n"
	"	);\n"
	"end entity;\n";

const char *arch_head =
	"architecture arch of wfmlut is\n"
	"begin\n";

const char *proc_head =
	"	process (addr)\n"
	"	begin\n"
	"		case addr is\n";

const char *proc_foot =
	"		end case;\n"
	"	end process;\n";

const char *arch_foot =
	"end architecture;\n";

/**
 * Main program entry point
 * @param argc Number of command line arguments
 * @param argv Command line arguments
 */
int main(int argc, char **argv)
{
	/* Program configuration */
	cfg_t cfg =
	{
		.verbose = DEFAULT_VERBOSE,
		.width = DEFAULT_WIDTH,
		.depth = DEFAULT_DEPTH
	};

	/* Begin arg parsing stuff */
	putenv("ARGP_HELP_FMT=no-dup-args-note");
	argp_parse(&argp, argc, argv, 0, 0, &cfg);
	/* End arg parsing stuff */

	if (cfg.verbose)
	{
		printf("Width : %d\n", cfg.width);
		printf("Depth : %d\n", cfg.depth);
	}

	double phi, s, c;
	char ss[cfg.width+4];
	char cs[cfg.width+4];

	for (int i = 0; i < cfg.depth; ++i)
	{
		phi = (double) 2.0f * M_PI * i / cfg.depth;
		s = sin(phi);
		c = cos(phi);

		snprintf(ss, sizeof(ss), "%0.16f", s);
		snprintf(cs, sizeof(cs), "%0.16f", c);
	} 

	return 0;
}
