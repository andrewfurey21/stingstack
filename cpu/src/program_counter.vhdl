library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity program_counter is
  generic (
    WIDTH : integer
  );
  port (
    clk    : in std_logic;
    rst    : in std_logic;
    pc_next: in std_logic_vector(WIDTH - 1 downto 0);
    stalled: in std_logic;
    pc     : out std_logic_vector(WIDTH - 1 downto 0)
  );
end entity program_counter;

architecture rtl of program_counter is
begin

  seq: process(clk)
  begin
    if rising_edge(clk) then

      if rst = '1' then
        pc <= (others => '0');
      else

        if stalled = '0' then
          pc <= pc_next;
        end if;

      end if;

    end if;
  end process;

end architecture rtl;
