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
 * representation that is @c m + @c n + 1 bits wide. The binary number is in
 * Qm.n fixed-point format. There is an extra character in the output to
 * account for the trailing '\0'. 
 * @param x The double to be converted
 * @param m The number of integer bits in the output, including the sign
 * @param n The number of fractional bits in the output.
 * @return Returns a pointer to the binary string representation of @c x.
 *   Must call free() to release memory.
 */
char *dtob(double x, size_t m, size_t n)
{
	double *possibles_dbl;
	char **possibles_str;
	size_t n_possibles;
	char *b;
	int closest;

	/* Validate inputs */
	if (m == 0)
		return NULL;

	n_possibles = (size_t) lround(pow(2, m+n));

	if ((b = calloc(m+n+1, sizeof(char))) == NULL)
		goto cleanup_error;

	if ((possibles_dbl = calloc(n_possibles, sizeof(double))) == NULL)
		goto cleanup_error;

	if ((possibles_str = malloc(n_possibles * sizeof(char *))) == NULL)
		goto cleanup_error;

	for (size_t i = 0; i < n_possibles; ++i)
	{
		if ((possibles_str[i] = calloc(1, m+n+1)) == NULL)
			goto cleanup_error;
	}
	
	/* Create array of possible binary values, given this m and n */
	for (size_t i = 0; i < n_possibles; ++i)
	{
		char tmp[m+n+1];
		memset(tmp, 0, sizeof(tmp));

		/* Create binary string of possible values */
		for (int j = m+n-1; j >= 0; --j)
			tmp[j] = ((i >> (m+n-1-j)) & 1) == 0 ? '0' : '1';

		/* Store the binary string in the overall array */
		strncpy(possibles_str[i], tmp, strlen(tmp));

		/* Convert integer bits of possible values to real numbers */
		for (size_t j = 0; j < m; ++j)
		{
			int mul = (j == 0) ? -1 : 1; /* The MSB is the sign bit */
			double val = (tmp[j] == '0' ? 0 : 1) * mul * pow(2, m-1-j);
			possibles_dbl[i] += val;
		}

		/* Convert fractional bits of possible values to real numbers */
		for (size_t j = m; j < m+n; ++j)
		{
			double val = (tmp[j] == '0' ? 0 : 1) * 1.0/pow(2, j);
			possibles_dbl[i] += val;
		}
	}
	
	/* Loop over possible values and find the closest match */
	closest = 0;
	for (size_t i = 0; i < n_possibles; ++i)
	{
		if (fabs(possibles_dbl[i]-x) <= fabs(possibles_dbl[closest]-x))
			closest = i;
	}
	
	strncpy(b, possibles_str[closest], m+n);

	if (possibles_dbl)
		free(possibles_dbl);
	for (size_t i = 0; i < n_possibles; ++i)
	{
		if (possibles_str[i])
			free(possibles_str[i]);
	}
	if (possibles_str)
		free(possibles_str);
	return b;

cleanup_error:
	if (possibles_dbl)
		free(possibles_dbl);
	for (size_t i = 0; i < n_possibles; ++i)
	{
		if (possibles_str[i] != NULL)
			free(possibles_str[i]);
	}
	if (possibles_str)
		free(possibles_str);
	if (b)
		free(b);
	return NULL;

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
	int bitwidth;

	/* Program configuration */
	cfg_t cfg =
	{
		.verbose = DEFAULT_VERBOSE,
		.ibits = DEFAULT_IBITS,
		.fbits = DEFAULT_FBITS,
		.depth = DEFAULT_DEPTH,
		.output_file = DEFAULT_OUTPUT_FILE,
	};

	/* Begin arg parsing stuff */
	putenv("ARGP_HELP_FMT=no-dup-args-note");
	argp_parse(&argp, argc, argv, 0, 0, &cfg);
	/* End arg parsing stuff */

	/* The number of output bits */
	bitwidth = cfg.ibits + cfg.fbits;

	/* the lines between the case statement */
	line_fmt = "\t\t\twhen \"%s\" => sin <= \"%s\"; cos <= \"%s\";\n";

	/* 36 is the number of hard-coded characters in the line format
	 * log2(cfg.depth) is the number of bits it takes to code the first '%s'
	 * and bitwidth is the for the next two '%s'. Plus 1 for trailing \0
	 */
	nline = 36 + (size_t) lround(log2(cfg.depth)) + 2 * bitwidth + 1;

	/* The LUT portion of the VHDL (inside the case statement) will be
	 * cfg.depth lines long, each line is nline characters
	 */
	nlut = cfg.depth * nline;

	/* The overall file size is the length of the vhdl_fmt string minus
	 * the 6 '%d' and the 2 '%s' characters plus the length of the strings
	 * that take the place of those format specifiers. The 3 assumes that
	 * the '%d' strings in the entity declaration are replaced by a single
	 * digit number. See below if more digits are needed.
	 */
	nfile = strlen(vhdl_fmt) - 6 - 2 + 3 + nlut;

	/* We assumed the bitwidth was 1 above, which replaced 3 of the '%d'
	 * characters in the vhdl_fmt string, for a loss of 6 characters and a 
	 * gain of 3 (net 3). If instead the bitwidth is a number that takes up 2
	 * character digits to display then we need to add 3 more to the file
	 * length to account for the extra 3 characters. If it takes more than 2
	 * (i.e. representing a std_logic_vector(xxx downto 0)), then you've lost
	 * your mind.
	 */
	if (bitwidth >= 10)
		nfile += 3;

	if (cfg.verbose)
	{
		int pad = -1 * (int)strlen("Num line characters");
		printf("%*s : %d\n", pad, "Integer bits", cfg.ibits);
		printf("%*s : %d\n", pad, "Fractional bits", cfg.fbits);
		printf("%*s : %d\n", pad, "Width", bitwidth);
		printf("%*s : %d\n", pad, "Depth", cfg.depth);
		printf("%*s : %zu\n", pad, "Depth bits", (size_t) lround(log2(cfg.depth)));
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
		char indstr[bitwidth + 1];
		char *sinstr;
		char *cosstr;
		char line[nline];
		double phi, s, c;
		size_t nbits_depth;

		memset(indstr, 0, sizeof(indstr));
		memset(line, 0, sizeof(line));

		/* Create binary string of int index */
		nbits_depth = (size_t) lround(log2(cfg.depth));
		for (int j = nbits_depth-1; j >= 0; --j)
			indstr[j] = ((i >> (nbits_depth-1-j)) & 1) == 0 ? '0' : '1';

		phi = 2.0f * M_PI * i / cfg.depth;
		s = sin(phi);
		c = cos(phi);

		sinstr = dtob(s, 1, bitwidth - 1);
		cosstr = dtob(c, 1, bitwidth - 1);

		snprintf(line, sizeof(line), line_fmt, indstr, sinstr, cosstr);
		memcpy(&lut[strlen(lut)], line, strlen(line));

		if (cfg.verbose)
			printf("%s", line);

		if (sinstr)
			free(sinstr);
		if (cosstr)
			free(cosstr);
	} 

	snprintf(vhdl, nfile, vhdl_fmt, bitwidth-1, bitwidth-1, bitwidth-1, lut);

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
