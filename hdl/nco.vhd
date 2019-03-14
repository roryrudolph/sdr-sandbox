-- For a good tutorial on NCO theory and operation, see:
-- https://zipcpu.com/dsp/2017/12/09/nco.html

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity nco is
	port (
		clk   : in std_logic;
		rst_n : in std_logic;
		f_hz  : in unsigned (31 downto 0);
		sin   : out unsigned (31 downto 0);
		cos   : out unsigned (31 downto 0)
	);
end entity;

architecture arch of nco is
	signal phi    : unsigned (31 downto 0) := (others => '0');
	signal sinout : unsigned (31 downto 0) := (others => '0');
	signal cosout : unsigned (31 downto 0) := (others => '0');
begin

	process (clk, rst_n)
	begin
		if rst_n = '0' then
			phi    <= (others => '0');
			sinout <= (others => '0');
			cosout <= (others => '0');
		elsif rising_edge(clk) then
		end if;
	end process;

	sin <= sinout;
	cos <= cosout;
	
end architecture;
