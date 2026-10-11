library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity rv32i_register_file is

  port (
    clk : in std_logic;
    rst : in std_logic;

    rs1_select : in std_logic_vector(4 downto 0);
    rs2_select : in std_logic_vector(4 downto 0);

    rd_select  : in std_logic_vector( 4 downto 0);
    rd         : in std_logic_vector(31 downto 0);

    rs1 : out std_logic_vector(31 downto 0);
    rs2 : out std_logic_vector(31 downto 0)
  );

end entity rv32i_register_file;

architecture rtl of rv32i_register_file is

  type   register_file_array_type is array (1 to 31) of std_logic_vector(31 downto 0);
  signal register_file_array : register_file_array_type := (others => (others => '0'));

  signal rs1_internal : std_logic_vector(31 downto 0);
  signal rs2_internal : std_logic_vector(31 downto 0);

  -- Reads from register file and forwards destination register to source.
  procedure read_from_register_file(
    variable source_index : in integer range 0 to 31;
    signal register_file  : in register_file_array_type;

    signal output: out std_logic_vector(31 downto 0)
  ) is
  begin

    if (source_index = 0) then
      output <= (others => '0');
    else
      output <= register_file(source_index);
    end if;

  end procedure read_from_register_file;

begin

  rs1 <= rs1_internal;
  rs2 <= rs2_internal;

  seq: process (clk)

    variable rs1_index  : integer range 0 to 31;
    variable rs2_index  : integer range 0 to 31;
    variable rd_index   : integer range 0 to 31;

  begin

    if rising_edge(clk) then
      rd_index  := to_integer(unsigned(rd_select));
      rs1_index := to_integer(unsigned(rs1_select));
      rs2_index := to_integer(unsigned(rs2_select));

      if rst = '1' then
        rs1_internal <= (others => '0');
        rs2_internal <= (others => '0');
      else

        read_from_register_file(rs1_index,
                                register_file_array,
                                rs1_internal);

        read_from_register_file(rs2_index,
                                register_file_array,
                                rs2_internal);

        if rd_index /= 0 then
          register_file_array(rd_index) <= rd;
        end if;

      end if; -- reset

    end if; -- clk

  end process seq;

end architecture rtl;
