library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_textio.all;
use std.textio.all;

entity sinelut_tb is
end entity;

architecture arch of sinelut_tb is

	constant T     : time := 10 ns;
	constant DEPTH : positive := 4;
	constant WIDTH : positive := 4;

	component sinelut is
		generic (
			FILENAME : string;
			DEPTH    : positive; 
			WIDTH    : positive
		);
		port (
			addr : in std_logic_vector (WIDTH-1 downto 0);
			sin  : out std_logic_vector (WIDTH-1 downto 0)
		);
	end component;
	
	signal clk   : std_logic;
	signal rst_n : std_logic;
	signal addr  : std_logic_vector (WIDTH-1 downto 0);
	signal sin   : std_logic_vector (WIDTH-1 downto 0);

begin

	uut : sinelut
		generic map (
			FILENAME => "/home/rory/sdr-sandbox/data/sine.data",
			DEPTH    => DEPTH,
			WIDTH    => WIDTH
		)
		port map (
			addr => addr,
			sin  => sin
		);

	proc_clk_gen : process
	begin
		clk <= '0';
		wait for T;
		loop
			clk <= '1';
			wait for T/2;
			clk <= '0';
			wait for T/2;
		end loop;
	end process;

	proc_rst_gen : process
	begin
		rst_n <= '0';
		wait until rising_edge(clk);
		wait for T/4;
		rst_n <= '1';
		wait;
	end process;

	proc_input : process (clk, rst_n)
		variable addr_int : integer;
	begin
		if (rst_n = '0') then
			addr <= (others => '0');
			addr_int := 0;
		elsif rising_edge(clk) then
			addr_int := to_integer(unsigned(addr)) + 1;
			addr <= std_logic_vector(to_unsigned(addr_int, WIDTH));
		end if;
	end process;

end architecture;
