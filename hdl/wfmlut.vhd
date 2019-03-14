library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity wfmlut is
	port (
		addr : in std_logic_vector (7 downto 0);
		sin  : out std_logic_vector (7 downto 0);
		cos  : out std_logic_vector (7 downto 0)
	);
end entity;

architecture arch of wfmlut is
begin

	process (addr)
	begin
		case addr is
			when X"00" => sin <= X"00"; cos <= X"00";
			when X"01" => sin <= X"01"; cos <= X"10";
			when X"02" => sin <= X"02"; cos <= X"20";
			when X"03" => sin <= X"03"; cos <= X"30";
			when X"04" => sin <= X"04"; cos <= X"40";
			when X"05" => sin <= X"05"; cos <= X"50";
			when X"06" => sin <= X"06"; cos <= X"60";
			when X"07" => sin <= X"07"; cos <= X"70";
			when X"08" => sin <= X"08"; cos <= X"80";
			when X"09" => sin <= X"09"; cos <= X"90";
			when X"0a" => sin <= X"0a"; cos <= X"a0";
			when X"0b" => sin <= X"0b"; cos <= X"b0";
			when X"0c" => sin <= X"0c"; cos <= X"c0";
			when X"0d" => sin <= X"0d"; cos <= X"d0";
			when X"0e" => sin <= X"0e"; cos <= X"e0";
			when X"0f" => sin <= X"0f"; cos <= X"f0";
			when others => sin <= X"ff"; cos <= X"ff";
		end case;
	end process;

end architecture;
