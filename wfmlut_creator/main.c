/*
 * This program produces a VHDL file for a sine and cosine lookup table.
 * Parameters for the program can be found with the --help flag at run-time.
 */

#include "args.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>

/**
 * This is the format specifier (template, if you will) of the VHDL
 * code that will be created by this program.
 */
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
 * This function converts a double @c x into a binary string 
 * representation that is @c nbits wide. The binary number is in Q1.nbits-1
 * format. In other words, there will be 1 sign bit and @c nbits-1 fraction
 * bits. There are no integer bits other than the sign, thus the input number
 * must be normalized from -1 to 1 to avoid truncation. 
 * @param x The double to be converted
 * @param nbits The width of the output binary number. The actual string
 *   buffer is nbits+1 to account for the trailing '\0'
 * @return Returns a pointer to the binary string representation of @c x.
 *   Must call free() to release memory.
 */
char *dtob(double x, size_t nbits)
{
	double val;
	char *b, *bptr;

	if ((b = calloc(nbits+1, 1)) == NULL)
		return NULL;

	bptr = b;

	if (x < -1.0f)
	{
		for (size_t i = 0; i < nbits; ++i)
		{
			b[i] = i == 0 ? '1' : '0';
		}
		return b;
	}

	if (x < 0)
	{
		x += 1.0f;
		*bptr++ = '1';
	}
	else
	{
		*bptr++ = '0';
	}

	for (size_t i = 1; i <= nbits; ++i)
	{
		val = 1.0f/pow(2, i);
		if (x >= val)
		{
			*bptr++ = '1';
			x -= val;
		}
		else
		{
			*bptr++ = '0';
		}
	}
	
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
	char *line_fmt;
	char *lut;
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

	/* the lines between the case statement */
	line_fmt = "\t\t\twhen \"%s\" => sin <= \"%s\"; cos <= \"%s\";\n";

	/* 36 is the number of hard-coded characters in the line */
	nline = 36 + 3 * (int) ceil(log2(cfg.depth));

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
	{
		int pad = -1 * (int)strlen("Num line characters");
		printf("%*s : %d\n", pad, "Width", cfg.width);
		printf("%*s : %d\n", pad, "Depth", cfg.depth);
		printf("%*s : %s\n", pad, "Output file", cfg.output_file);
		printf("%*s : %zu\n", pad, "Num line characters", nline);
		printf("%*s : %zu\n", pad, "Num LUT characters", nlut);
		printf("%*s : %zu\n", pad, "Num file characters", nfile);
	}

	if (cfg.verbose)
		printf("Opening output file '%s' for writing\n", cfg.output_file);

	/* Open the output file */
	if ((fp = fopen(cfg.output_file, "w")) == NULL)
	{
		printf("ERROR: Could not open output file handle: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	if ((vhdl = calloc(1, nfile)) == NULL)
	{
		printf("ERROR: Could not allocate memory for file: %s\n", strerror(errno));
		goto cleanup;
	}
	
	if ((lut = calloc(1, nlut)) == NULL)
	{
		printf("ERROR: Could not allocate memory for LUT: %s\n", strerror(errno));
		goto cleanup;
	}

	/* Create the VHDL lines in the case statement */
	for (int i = 0; i < cfg.depth; ++i)
	{
		char indstr[cfg.width+1];
		char *sinstr;
		char *cosstr;
		char line[nline];
		double phi, s, c;

		memset(indstr, 0, sizeof(indstr));
		memset(line, 0, sizeof(line));

		phi = 2.0f * M_PI * i / cfg.depth;
		s = sin(phi);
		c = cos(phi);

		snprintf(indstr, sizeof(indstr), "%d", i);
		sinstr = dtob(s, cfg.width);
		cosstr = dtob(c, cfg.width);

		snprintf(line, sizeof(line), line_fmt, indstr, sinstr, cosstr);
		memcpy(&lut[strlen(lut)], line, strlen(line));
		printf("%s\n", line);

		if (sinstr)
			free(sinstr);
		if (cosstr)
			free(cosstr);
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
