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
		addr : in std_logic_vector (%zu downto 0);\n\
		sin  : out std_logic_vector (%zu downto 0);\n\
		cos  : out std_logic_vector (%zu downto 0)\n\
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
	size_t nbits_out;
	size_t nbits_depth;

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
	nbits_out = cfg.ibits + cfg.fbits;

	/* The number of bits in the address in parameter, which is what indexes
	 * into the case statement, a.k.a. the depth
	 */
	nbits_depth = (size_t) lround(log2(cfg.depth));


	/* the lines between the case statement */
	line_fmt = "\t\t\twhen \"%s\" => sin <= \"%s\"; cos <= \"%s\";\n";

	/* 36 is the number of hard-coded characters in the line format.
	 * nbits_depth is the number of bits it takes to code the first '%s'.
	 * nbits_out is the for the next two '%s'. Plus 1 for trailing \0
	 */
	nline = 36 + nbits_depth + 2 * nbits_out + 1;

	/* The LUT portion of the VHDL (inside the case statement) will be
	 * cfg.depth lines long, each line is nline characters
	 */
	nlut = cfg.depth * nline;

	/* The overall file size is the length of the vhdl_fmt string minus
	 * the '%zu' characters and the '%s' characters, plus the length of the
	 * strings that replace them. We assume we are replacing the '%zu' strings
	 * with a single-digit integer, and thus we lose 9 characters
	 * (strlen("%zu")*3) and gain 3. We replace the '%s' string with the
	 * number of characters in the LUT string.
	 */
	nfile = strlen(vhdl_fmt) - 9 + 3 - 2 + nlut;

	/* We assumed above we replaced the two 'out' parameters (the %zu strings)
	 * with a single-digit number. If in fact it's a two-digit number we have
	 * to add two more characters to the total. If it's more than two
	 * you're design probably won't work; is crazy.
	 */
	if (nbits_out >= 10)
		nfile += 2;

	/* Same thing as nbits_out but with the 'in' address parameter */
	if (nbits_depth >= 10)
		nfile += 1;

	if (cfg.verbose)
	{
		int pad = -1 * (int)strlen("Num chars in VHDL fmt");
		printf("%*s : %d\n", pad, "Integer bits", cfg.ibits);
		printf("%*s : %d\n", pad, "Fractional bits", cfg.fbits);
		printf("%*s : %zu\n", pad, "Output width", nbits_out);
		printf("%*s : %d\n", pad, "Depth", cfg.depth);
		printf("%*s : %zu\n", pad, "Depth bits", (size_t) lround(log2(cfg.depth)));
		printf("%*s : %s\n", pad, "Output file", cfg.output_file);
		printf("%*s : %zu\n", pad, "Num chars in line", nline);
		printf("%*s : %zu\n", pad, "Num chars in LUT", nlut);
		printf("%*s : %zu\n", pad, "Num chars in file", nfile);
		printf("%*s : %zu\n", pad, "Num chars in VHDL fmt", strlen(vhdl_fmt));
		printf("%*s : %zu\n", pad, "Num chars in line fmt", strlen(line_fmt));
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
		char indstr[nbits_out + 1];
		char *sinstr;
		char *cosstr;
		char line[nline];
		double phi, s, c;

		memset(indstr, 0, sizeof(indstr));
		memset(line, 0, sizeof(line));

		/* Create binary string of int index */
		for (int j = nbits_depth-1; j >= 0; --j)
			indstr[j] = ((i >> (nbits_depth-1-j)) & 1) == 0 ? '0' : '1';

		phi = 2.0f * M_PI * i / cfg.depth;
		s = sin(phi);
		c = cos(phi);

		sinstr = dtob(s, 1, nbits_out - 1);
		cosstr = dtob(c, 1, nbits_out - 1);

		snprintf(line, sizeof(line), line_fmt, indstr, sinstr, cosstr);
		memcpy(&lut[strlen(lut)], line, strlen(line));

		if (cfg.verbose)
			printf("%s", line);

		if (sinstr)
			free(sinstr);
		if (cosstr)
			free(cosstr);
	} 

	snprintf(vhdl, nfile, vhdl_fmt, nbits_depth-1, nbits_out-1, nbits_out-1, lut);

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
