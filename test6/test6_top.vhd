library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity test6_top is
port
(
   LED 	   : out std_logic_vector(7 downto 0);
	KEY      : in  std_logic_vector(1 downto 0);
	SW       : in  std_logic_vector(4 downto 0);

	CLOCK_50 : in std_logic
);
end entity;

architecture behavioral of test6_top
is
	signal LED_BUF : std_logic_vector(7 downto 0) := X"0F";

	signal CLOCK 	: std_logic;  -- 125 MHz
	signal RESET	: std_logic := '1';
	
	signal APB_ADDR   : std_logic_vector(15 downto 0);                    
	signal APB_SEL    : std_logic;                                        
	signal APB_ENABLE : std_logic;                                        
	signal APB_WRITE  : std_logic;                                        
	signal APB_WDATA  : std_logic_vector(31 downto 0);                    
	signal APB_RDATA  : std_logic_vector(31 downto 0);
	signal APB_READY  : std_logic;  
	
begin

	CLOCK <= CLOCK_50;
	
	APB_READY <= '1';
	APB_RDATA <= X"87654321";
	
	LED <= LED_BUF;	

	soc : entity work.test6_soc
	port map
	(
		apb_paddr   => APB_ADDR,                    
		apb_psel    => APB_SEL,                    
		apb_penable => APB_ENABLE,                    
		apb_pwrite  => APB_WRITE,
		apb_pwdata  => APB_WDATA,                    
		apb_prdata  => APB_RDATA,
		apb_pready  => APB_READY,
					
		clk_reset_reset_n => '1',
		
		reset_reset_n 	=> RESET,		
		clk_clk     	=> CLOCK
	);
	
   process (CLOCK)
   begin
      if rising_edge(CLOCK)
      then
         if (APB_SEL = '1') and (APB_ENABLE = '1') and (APB_WRITE = '1')
         then
				LED_BUF(7 downto 0) <= APB_WDATA(7 downto 0);
         end if;
      end if;
   end process;
	
end behavioral;