/*
 * This program produces a VHDL file for a sine and cosine lookup table.
 * Parameters for the program can be found with the --help flag at run-time.
 */

#include "args.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>

static const char *vhdl_fmt = "\
library ieee;\n\
use ieee.std_logic_1164.all;\n\
use ieee.numeric_std.all;\n\
\n\
entity wfmlut is\n\
	port (\n\
		addr : in std_logic_vector (%d downto 0);\n\
		sin  : out std_logic_vector (%d downto 0);\n\
		cos  : out std_logic_vector (%d downto 0)\n\
	);\n\
end entity;\n\
\n\
architecture arch of wfmlut is\n\
begin\n\
\n\
	process (addr)\n\
	begin\n\
		case addr is\n\
%s\
			when others => sin <= (others => '0'); cos <= (others => '0');\n\
		end case;\n\
	end process;\n\
\n\
end architecture;\n";

/**
 * TODO
 */
char *dtob(double x, char *b, size_t nbits)
{
	double xi, xf;
	xf = modf(x, &xi);

	if (b == NULL)
		return NULL;

	if (x < 0)
	{
		*b++ = '1';
		xf *= -1.0f;
	}
	else 
	{
		*b++ = '0';
	}

	for (size_t i = 1; i <= nbits; ++i)
	{
		double val = 1.0f/pow(2, i);
		printf("x %f , xf %f , val %f , b %s\n", x, xf, val, b);
		if (xf >= val)
		{
			*b++ = '1';
			xf -= val;
		}
		else
		{
			*b++ = '0';
		}
	}
	printf("\n");
	
	return b;
}

/**
 * Main program entry point
 * @param argc Number of command line arguments
 * @param argv Command line arguments
 */
int main(int argc, char **argv)
{
	FILE *fp;
	size_t nfile;
	size_t nlut;
	size_t nline;
	char *vhdl;
	size_t rc;

	/* Program configuration */
	cfg_t cfg =
	{
		.verbose = DEFAULT_VERBOSE,
		.width = DEFAULT_WIDTH,
		.depth = DEFAULT_DEPTH,
		.output_file = DEFAULT_OUTPUT_FILE,
	};

	/* Begin arg parsing stuff */
	putenv("ARGP_HELP_FMT=no-dup-args-note");
	argp_parse(&argp, argc, argv, 0, 0, &cfg);
	/* End arg parsing stuff */

	if (cfg.verbose)
	{
		printf("Width       : %d\n", cfg.width);
		printf("Depth       : %d\n", cfg.depth);
		printf("Output file : %s\n", cfg.output_file);
	}

	if (cfg.verbose)
		printf("Opening output file '%s' for writing\n", cfg.output_file);

	if ((fp = fopen(cfg.output_file, "w")) == NULL)
	{
		printf("ERROR: Could not open output file handle: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	/* 36 is the number of hard-coded characters in the line */
	nline = 36 + 3 * cfg.width;

	/* The LUT portion of the VHDL (inside the case statement) will be
	 * cfg.depth lines long, each line is nline characters
	 */
	nlut = cfg.depth * nline;

	/* The overall file size is the length of the vhdl_fmt string minus
	 * the 6 '%d' and the 2 '%s' characters plus the length of the strings
	 * that take the place of those format specifiers
	 */
	nfile = strlen(vhdl_fmt) - 6 - 2 + 3 + nlut;

	/* If the width is 2 digits, then need to add 3 more to the file length
	 * to account for the extra 3 characters
	 */
	if (cfg.width >= 10)
		nfile += 3;

	if (cfg.verbose)
		printf("VHDL file will be at most %zu bytes\n", nfile);

	vhdl = calloc(1, nfile);
	
	char *line_fmt = "\t\t\twhen \"%s\" => sin <= \"%s\"; cos <= \"%s\";\n";
	char lut[nlut];
	memset(lut, 0, sizeof(lut));

	/* Create the VHDL lines in the case statement */
	for (int i = 0; i < cfg.depth; ++i)
	{
		char indstr[cfg.width+1];
		char sinstr[cfg.width+1];
		char cosstr[cfg.width+1];
		char line[nline];
		double phi, s, c;

		memset(indstr, 0, sizeof(indstr));
		memset(sinstr, 0, sizeof(sinstr));
		memset(cosstr, 0, sizeof(cosstr));
		memset(line, 0, sizeof(line));

		phi = 2.0f * M_PI * i / cfg.depth;
		s = sin(phi);
		c = cos(phi);

		snprintf(indstr, sizeof(indstr), "%d", i);
		dtob(s, &sinstr[0], cfg.width);
		dtob(c, &cosstr[0], cfg.width);

		snprintf(line, sizeof(line), line_fmt, indstr, sinstr, cosstr);
		memcpy(&lut[strlen(lut)], line, strlen(line));
	} 

	snprintf(vhdl, nfile, vhdl_fmt, cfg.width-1, cfg.width-1, cfg.width-1, lut);

	if (cfg.verbose)
		printf("Writing %zu bytes to %s\n", strlen(vhdl), cfg.output_file);

	if ((rc = fwrite(vhdl, 1, strlen(vhdl), fp)) != strlen(vhdl))
	{
		printf("ERROR: Problem writing output file. "
			"Wrote %zu bytes, expected %zu\n", rc, strlen(vhdl));
		goto cleanup;
	}

cleanup:

	if (vhdl)
		free(vhdl);

	if (cfg.verbose)
		printf("Closing output file\n");

	if (fclose(fp) == EOF)
	{
		printf("ERROR: Could not close output file handle: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	

	return 0;
}
