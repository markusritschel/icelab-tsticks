/* Some helper functions */


// ==========================================
// =========== Format Function ==============
// ==========================================
String twodigits(int x) {
    /* Helper function to format datetime elements as two digit (leading zeros) string */
    String output = "";
    if (x < 10) {
        output += "0";
        output += x;
    } else {
        output = x;
    }
    return output;
}


// ==========================================
// === Format Timestamp in ISO8601 format ===
// ==========================================
String getISOtime() {
    /* Creates a timestamp of the ISO8601 format */
    DateTime now = rtc.now();
    String ISO8601_time = "";

    ISO8601_time +=  now.year();
    ISO8601_time +=  "-";
    ISO8601_time +=  twodigits(now.month());
    ISO8601_time +=  "-";
    ISO8601_time +=  twodigits(now.day());
    ISO8601_time +=  "T";
    ISO8601_time +=  twodigits(now.hour());
    ISO8601_time +=  ":";
    ISO8601_time +=  twodigits(now.minute());
    ISO8601_time +=  ":";
    ISO8601_time +=  twodigits(now.second());
    ISO8601_time +=  "Z";

    return ISO8601_time;
}


void writeln2SD(String dataString) {
    /* Helper function to write a line to both the Serial output and the SD card */
    File dataFile = SD.open(logfile, FILE_WRITE);
    // // if the file is available, write to it:
    if (dataFile) {
        dataFile.println(dataString);
        dataFile.close();
        Serial.println(dataString);
//        Serial.flush();
    };
}

void write2SD(String dataString) {
    /* Helper function to write a string to both the Serial output and the SD card */
    File dataFile = SD.open(logfile, FILE_WRITE);
    // // if the file is available, write to it:
    if (dataFile) {
        dataFile.print(dataString);
        dataFile.close();
        Serial.print(dataString);
//        Serial.flush();
    };
}
