const String logfile = "tsticks.log";  // USER INPUT: name of the log file on the SD card
const String csv_sep = ", ";           // separator for the csv log file

//Control Function Commands
#define CHAIN            0x99
#define PIO_ACCESS_WRITE 0xA5


//1-Wire ROM Function Commands
#define CONDITIONAL_READ_ROM    0x0F
#define MATCH_ROM               0x55


//Chain States
#define CHAIN_OFF  0x3C
#define CHAIN_ON   0x5A
#define CHAIN_DONE 0x96

#define VALID_SEQUENCE 0xAA
#define END_OF_BUS 0xFF
#define IS_DS28EA00_SENSOR 0x42


typedef struct{
  unsigned char rom_code[8];
  int           raw_temp;
  unsigned char config_register;
  float         temperature;
  unsigned char pio_state;
} ds28ea00_t;

typedef struct{
  uint8_t           pin;
  OneWire           ow_bus;
  DallasTemperature sensors;
  String            registration_number;
  ds28ea00_t        sensor_array[10];
} tstick_t;
