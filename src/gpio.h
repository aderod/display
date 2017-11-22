
#define cs_pin 20
#define data_pin 18
#define clk_pin 19

//Register GPIO in linux kernel
void gpioExport(int gpio);
void gpioUnExport(int gpio);
//Open file descriptor
int gpioOpen(int gpio);
//Close file descripto
int gpioClose(int fd);
//Data direction
void gpioDirection(int gpio, int direction); // 1 for output, 0 for input
//Set GPIO, '1' = 1, '0' = 0
void gpioSet(int gpioHandler, char value);

