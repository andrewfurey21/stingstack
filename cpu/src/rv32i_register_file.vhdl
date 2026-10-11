library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- TODO: hardware zero to 0
-- dont stall writes
-- don't zero out everything on reset.
-- write to read forwarding

entity rv32i_register_file is

  port (
    clk    : in std_logic;
    rst    : in std_logic;
    stalled: in std_logic;

    rs1_select : in std_logic_vector(4 downto 0);
    rs2_select : in std_logic_vector(4 downto 0);

    rd         : in std_logic_vector(31 downto 0);
    rd_select  : in std_logic_vector( 4 downto 0);
    rd_write   : in std_logic;

    rs1 : out std_logic_vector(31 downto 0);
    rs2 : out std_logic_vector(31 downto 0)
  );

end entity rv32i_register_file;

architecture rtl of rv32i_register_file is

  type   register_file_array_type is array (0 to 31) of std_logic_vector(31 downto 0);
  signal register_file_array : register_file_array_type;

  signal rs1_internal : std_logic_vector(31 downto 0);
  signal rs2_internal : std_logic_vector(31 downto 0);

begin

  seq: process (clk)

    variable rs1_index  : integer;
    variable rs2_index  : integer;
    variable rd_index   : integer;

  begin

    if rising_edge(clk) then
      if rst = '1' then
        register_file_array <= (others => (others => '0'));
        rs1_internal <= (others => '0');
        rs2_internal <= (others => '0');
      else

        if stalled = '0' then

          rs1_index    := to_integer(unsigned(rs1_select));
          rs1_internal <= register_file_array(rs1_index);

          rs2_index    := to_integer(unsigned(rs2_select));
          rs2_internal <= register_file_array(rs2_index);

          if rd_write = '1' then
            rd_index := to_integer(unsigned(rd_select));
            register_file_array(rd_index) <= rd;
          end if;

        end if;

      end if;
    end if;

  end process seq;

  rs1 <= rs1_internal;
  rs2 <= rs2_internal;

  -- Q: if i did multiple assignments, UB?

end architecture rtl;
