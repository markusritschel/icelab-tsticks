const String logfile = "tsticks.log";  // USER INPUT: name of the log file on the SD card
const String csv_sep = ", ";           // separator for the csv log file

//Control Function Commands
#define CHAIN            0x99


//1-Wire ROM Function Commands
#define CONDITIONAL_READ_ROM    0x0F
#define MATCH_ROM               0x55


//Chain States
#define CHAIN_OFF  0x3C
#define CHAIN_ON   0x5A
#define CHAIN_DONE 0x96

#define VALID_SEQUENCE 0xAA
