library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity wfmlut_tb is
end entity;

architecture arch of wfmlut_tb is

	constant T : time := 10 ns;

	component wfmlut is
		port (
			addr : in std_logic_vector (7 downto 0);
			sin  : out std_logic_vector (15 downto 0);
			cos  : out std_logic_vector (15 downto 0)
		);
	end component;

	signal clk      : std_logic;
	signal addr     : std_logic_vector (7 downto 0) := (others => '0');
	signal sin      : std_logic_vector (15 downto 0) := (others => '0');
	signal cos      : std_logic_vector (15 downto 0) := (others => '0');

begin

	uut : wfmlut
		port map (
			addr => addr,
			sin  => sin,
			cos  => cos
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

	proc_input : process (clk)
		variable addr_uns : unsigned (7 downto 0) := (others => '0');
	begin
		if rising_edge(clk) then
			addr_uns := unsigned(addr) + to_unsigned(1, 8);
			addr <= std_logic_vector(addr_uns);
		end if;	
	end process;

end architecture;
