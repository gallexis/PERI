/*******************************************************************************
 * lcdr_user.c - Controleur pour LCd HD44780 ( 20x4 )
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

/*******************************************************************************
 * GPIO Pins
 ******************************************************************************/
#define RS 7
#define E  8
#define D4 22
#define D5 23
#define D6 24
#define D7 25

#define GPIO_INPUT  0
#define GPIO_OUTPUT 1

#define RPI_BLOCK_SIZE  0xB4
#define RPI_GPIO_BASE   0x20200000

struct gpio_s
{
  uint32_t gpfsel[7];
  uint32_t gpset[3];
  uint32_t gpclr[3];
  uint32_t gplev[3];
  uint32_t gpeds[3];
  uint32_t gpren[3];
  uint32_t gpfen[3];
  uint32_t gphen[3];
  uint32_t gplen[3];
  uint32_t gparen[3];
  uint32_t gpafen[3];
  uint32_t gppud[1];
  uint32_t gppudclk[3];
  uint32_t test[1];
};
volatile struct gpio_s* gpio_regs;

/*******************************************************************************
 * GPIO Operations 
 ******************************************************************************/

int gpio_setup ( void ){

  int mmap_fd = open ("/dev/mem", O_RDWR | O_SYNC );
  if ( mmap_fd < 0 ) {
    return -1;
  }

  gpio_regs = mmap ( NULL, RPI_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mmap_fd, RPI_GPIO_BASE );
  if ( gpio_regs == MAP_FAILED ) {
    close ( mmap_fd );
    return -1;
  }

  return 0;
}

void  gpio_teardown ( void ){
  munmap((void*)gpio_regs, RPI_BLOCK_SIZE);
}


void gpio_config(int gpio, int value)
{
  gpio_regs->gpfsel[gpio/10] = ( gpio_regs->gpfsel[gpio/10] & ~(0x7 << ((gpio % 10) * 3)) )
                             | (value << ((gpio % 10) * 3));
}

void gpio_update(int gpio, int value)
{
  if( value )
    gpio_regs->gpset[gpio/32] = (0x1 << (gpio % 32) );
  else
    gpio_regs->gpclr[gpio/32] = (0x1 << (gpio % 32) );
}

/*******************************************************************************
 * LCD's Instructions ( source = doc )
 ******************************************************************************/

#define B_0000  0
#define B_0001  1
#define B_0010  2
#define B_0011  3
#define B_0100  4
#define B_0101  5
#define B_0110  6
#define B_0111  7
#define B_1000  8
#define B_1001  9
#define B_1010  a
#define B_1011  b
#define B_1100  c
#define B_1101  d
#define B_1110  e
#define B_1111  f

#define __HEX(h,l) 0x##h##l
#define _HEX(h,l)  __HEX(h,l)
#define HEX(h,l)   _HEX(B_##h,B_##l)
#define BIT(b,n)   (((n)>>(b))&1)

/* commands */
#define LCD_CLEARDISPLAY        HEX(0000,0001)
#define LCD_RETURNHOME          HEX(0000,0010)
#define LCD_ENTRYMODESET        HEX(0000,0100)
#define LCD_DISPLAYCONTROL      HEX(0000,1000)
#define LCD_CURSORSHIFT         HEX(0001,0000)
#define LCD_FUNCTIONSET         HEX(0010,0000)
#define LCD_SETCGRAMADDR        HEX(0100,0000)
#define LCD_SETDDRAMADDR        HEX(1000,0000)

/* flags for display entry mode : combine with LCD_ENTRYMODESET */
#define LCD_EM_RIGHT            HEX(0000,0000)
#define LCD_EM_LEFT             HEX(0000,0010)
#define LCD_EM_DISPLAYSHIFT     HEX(0000,0001)
#define LCD_EM_DISPLAYNOSHIFT   HEX(0000,0000)

/* flags for display on/off control : combine with LCD_DISPLAYCONTROL */
#define LCD_DC_DISPLAYON        HEX(0000,0100)
#define LCD_DC_DISPLAYOFF       HEX(0000,0000)
#define LCD_DC_CURSORON         HEX(0000,0010)
#define LCD_DC_CURSOROFF        HEX(0000,0000)
#define LCD_DC_BLINKON          HEX(0000,0001)
#define LCD_DC_BLINKOFF         HEX(0000,0000)

/* flags for display/cursor shift : combine with LCD_CURSORSHIFT */
#define LCD_CS_DISPLAYMOVE      HEX(0000,1000)
#define LCD_CS_CURSORMOVE       HEX(0000,0000)
#define LCD_CS_MOVERIGHT        HEX(0000,0100)
#define LCD_CS_MOVELEFT         HEX(0000,0000)

/* flags for function set : combine with LCD_FUNCTIONSET */
#define LCD_FS_8BITMODE         HEX(0001,0000)
#define LCD_FS_4BITMODE         HEX(0000,0000)
#define LCD_FS_2LINE            HEX(0000,1000)
#define LCD_FS_1LINE            HEX(0000,0000)
#define LCD_FS_5x10DOTS         HEX(0000,0100)
#define LCD_FS_5x8DOTS          HEX(0000,0000)

/*******************************************************************************
 * LCD's Operations
 ******************************************************************************/

/* generate E signal */
void lcd_strobe()
{
  /* A compl�ter : done*/

	gpio_update(E, 1);
	usleep(1);	//enable pulse width 	>= 300ns
	gpio_update(E, 0);
	usleep(1);
}

/* send 4bits to LCD */
void lcd_write4bits(int data)
{
  /* first 4 bits */
  gpio_update(D7, BIT(7,data)); 
  gpio_update(D6, BIT(6,data)); 
  gpio_update(D5, BIT(5,data)); 
  gpio_update(D4, BIT(4,data));  
  lcd_strobe();
  
  /* second 4 bits */
  gpio_update(D7, BIT(3,data)); 
  gpio_update(D6, BIT(2,data)); 
  gpio_update(D5, BIT(1,data)); 
  gpio_update(D4, BIT(0,data));  
  lcd_strobe();

}

void lcd_command(int cmd)
{
  gpio_update(RS, 0);
  lcd_write4bits(cmd);
  usleep(2000); /* ça prend du temps */
}

void lcd_data(int character)
{
  gpio_update(RS, 1);
  lcd_write4bits(character);
}

/* initialization */
void lcd_init()
{
  gpio_update(E, 0);
  lcd_command(HEX(0011,0011)); /* initialization */
  lcd_command(HEX(0011,0010)); /* initialization */
  lcd_command(LCD_FUNCTIONSET    | LCD_FS_4BITMODE | LCD_FS_2LINE | LCD_FS_5x8DOTS /* a completer : done*/ );
  lcd_command(LCD_DISPLAYCONTROL | LCD_DC_DISPLAYON  | LCD_DC_CURSOROFF |  LCD_DC_BLINKOFF/* a completer : done */ );
  lcd_command(LCD_ENTRYMODESET   | LCD_EM_LEFT | LCD_EM_DISPLAYNOSHIFT/* a completer : done */ );
}

void lcd_clear()
{
  lcd_command(LCD_CLEARDISPLAY);
  lcd_command(LCD_RETURNHOME);
}

void lcd_message(char* txt)
{
  int i, j;
  for(i=0; i<20 && i<strlen(txt); i++){
    lcd_data(txt[i]);
  }
}

void lcd_set_cursor(int x, int y)
{	
	int i;
	if (y == 0) y = 0x00;
	else if (y == 1) y = 0x40;
	else if (y == 2) y = 0x14;
	else  y = 0x54;

	lcd_command(LCD_SETDDRAMADDR |  y);

	for(i = 0; i < x; i++)
		lcd_command(LCD_CURSORSHIFT |  LCD_CS_MOVERIGHT);
}


/*******************************************************************************
 * Finally, the main function
 ******************************************************************************/
int main(int argc, char** argv)
{
  /* arg */
  if( argc < 2 ){
    fprintf(stderr, "ERROR: must take a string as argument\n");
    exit(1);
  }

  /* Retreive the mapped GPIO memory */
  if( gpio_setup() == -1 ){
    perror("ERROR: gpio_setup\n");
    exit(1);
  }

  /* Setting up GPIOs to output */
  gpio_config(RS, GPIO_OUTPUT);
  gpio_config(E , GPIO_OUTPUT);
  gpio_config(D4, GPIO_OUTPUT);
  gpio_config(D5, GPIO_OUTPUT);
  gpio_config(D6, GPIO_OUTPUT);
  gpio_config(D7, GPIO_OUTPUT);

  /* initialization */
  lcd_init();

  /* default test direction */
  //lcd_command(LCD_ENTRYMODESET | LCD_EM_LEFT | LCD_EM_DISPLAYNOSHIFT);

  lcd_clear();
  lcd_set_cursor(2, 1);

  lcd_message(argv[1]);

  /* Release the GPIO memory mapping */
  gpio_teardown();

  return 0;
}
