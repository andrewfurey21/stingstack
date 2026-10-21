library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- TODO:
-- upgrade to vhdl2008 to use case? statement
-- go through each instruction. for example, mret is very specific, and anything else should give an illegal instruction (i believe)
-- not correct for shifts

-- Notes:
-- Can't really use generics here.

entity rv32i_decoder is

  port (
    instruction: in std_logic_vector(31 downto 0);

    illegal  : out std_logic;
    opcode   : out std_logic_vector( 6 downto 0);
    funct3   : out std_logic_vector( 2 downto 0);
    funct7   : out std_logic_vector( 6 downto 0);
    immediate: out std_logic_vector(31 downto 0);
    shamt    : out std_logic_vector( 4 downto 0);

    rs1      : out std_logic_vector( 4 downto 0);
    rs2      : out std_logic_vector( 4 downto 0);
    rd       : out std_logic_vector( 4 downto 0)
  );

end entity rv32i_decoder;

architecture rtl of rv32i_decoder is

  signal i_illegal : std_logic := '0';

begin

  seq: process (instruction)
    variable v_opcode : std_logic_vector(6 downto 0);
  begin

    v_opcode  := instruction(6 downto 0);
    i_illegal <= '0';

    case v_opcode is
      when "0110011" => null; -- R
      when "0010011" => null; -- I (+ I*)
      when "0000011" => null; -- R (loads)
      when "0100011" => null; -- S
      when "1100011" => null; -- B
      when "1101111" => null; -- J (jal)
      when "1100111" => null; -- I (jalr)
      when "0110111" => null; -- U (lui)
      when "0010111" => null; -- U (auipc)
      when "0001111" => null; -- I (fence)
      when "1110011" => null; -- I** (ecall, ebreak, mret, csr)
      when others => i_illegal <= '1';
    end case; -- instruction

  end process seq;

  illegal <= i_illegal;
  opcode  <= instruction( 6 downto  0);
  funct3  <= instruction(14 downto 12);
  funct7  <= instruction(31 downto 25);
  shamt   <= instruction(24 downto 20);

  rs1 <= instruction(19 downto 15);
  rs2 <= instruction(24 downto 20);
  rd  <= instruction(11 downto  7);

end architecture rtl;
