library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_textio.all;

entity top is
	port (
		clk  : in std_logic
	);
end entity;

architecture arch of top is

	constant FILENAME : string := "/home/rory/sdr-sandbox/data/sine.data";
	constant DEPTH    : positive := 4;
	constant WIDTH    : positive := 4;

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
	
	signal addr  : std_logic_vector (WIDTH-1 downto 0) := (others => '0');
	signal sin   : std_logic_vector (WIDTH-1 downto 0);

begin

	sineram : sinelut
		generic map (
			FILENAME => FILENAME,
			DEPTH    => DEPTH,
			WIDTH    => WIDTH
		)
		port map (
			addr => addr,
			sin  => sin
		);

end architecture;


