library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

-- Single rv32i instruction decoder

-- ops: lw, sw, add, mul, addi, slli, blt, bgt, beq, bne, jalr

entity rv32i_decoder is

  port (
    instruction: in std_logic_vector(31 downto 0)
  );

end entity rv32i_decoder;

architecture rtl of rv32i_decoder is
begin
end architecture rtl;
