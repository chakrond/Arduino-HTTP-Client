/*
  SD card read/write

  This example shows how to read and write data to and from an SD card file
  The circuit:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

  created   Nov 2010
  by David A. Mellis
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

*/

#include <SPI.h>
#include <SD.h>

File myFile;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print("Initializing SD card...");

  if (!SD.begin(5)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  //   open the file. note that only one file can be open at a time,
  //   so you have to close this one before opening another.
  //    myFile = SD.open("test4.txt", FILE_WRITE);
  //
  //    // if the file opened okay, write to it:
  //    if (myFile) {
  //      Serial.print("Writing to test.txt...");
  //      myFile.println("testing,7,8,9");
  //      // close the file:
  //      myFile.close();
  //      Serial.println("done.");
  //    } else {
  //      // if the file didn't open, print an error:
  //      Serial.println("error opening test.txt");
  //    }


  //  // Read whole file
  //  myFile = SD.open("test4.txt");
  //  if (myFile) {
  //    Serial.println("reading test4.txt:");
  //
  //    // read from the file until there's nothing else in it:
  //    while (myFile.available()) {
  //      Serial.write(myFile.read());
  //    }
  //    // close the file:
  //    myFile.close();
  //  } else {
  //    // if the file didn't open, print an error:
  //    Serial.println("error opening test.txt");
  //  }


  // determin array size




  String list[4];
  //  String str;
  String readstr;
  int index = 0;
  char copy[10];
  // Read by line
  myFile = SD.open("test4.txt");
  if (myFile) {
    Serial.println("reading by line test4.txt:");

    for ( int i = 0; i <= 3; i++) {

      if (index == 3) {
        readstr = myFile.readStringUntil('\r');
        //        if (readstr.length() == 0) {
        //          break;
        //        }
      } else {
        readstr = myFile.readStringUntil(',');
      }

      Serial.println("readstr: " + readstr);


      //      str = readstr;
      //      readstr.toCharArray(copy, 10);
      //      Serial.println(copy);
      list[index] = readstr;
      //      Serial.println(list[index]);
      index++;
    }

    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // Count element in array
  Serial.println( String(sizeof(list)/sizeof(list[0])) );

}

void loop() {
  // nothing happens after setup
}
