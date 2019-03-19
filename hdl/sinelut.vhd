library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_textio.all;
use std.textio.all;

entity sinelut is
	generic (
		FILENAME : string := "sinelut.data";
		DEPTH    : positive := 256;
		WIDTH    : positive := 8
	);
	port (
		addr : in std_logic_vector (WIDTH-1 downto 0);
		sin  : out std_logic_vector (WIDTH-1 downto 0)
	);
end entity;

architecture arch of sinelut is

	subtype word_t is std_logic_vector(WIDTH-1 downto 0);
	type ram_t is array(0 to DEPTH-1) of word_t;

	impure function initialize_ram(filename : in string) return ram_t is
		file F        : text is in filename;
		variable L    : line;
		variable res  : ram_t := (others => (others => '0'));
	begin
		for i in ram_t'range loop
			readline(F, L);
			if (L'length < WIDTH) then
				report "Line length error" severity failure;
			end if;
			read(L, res(i));
		end loop;
		return res;
	end function;

	signal ram      : ram_t := initialize_ram(FILENAME);
	signal addr_int : integer := 0;
		
begin

	addr_int <= to_integer(unsigned(addr));
	sin <= ram(addr_int);

end architecture;
