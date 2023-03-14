#ifndef SD_CONFIG
#define SD_CONFIG 

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Arduino.h>
#include <SD_MMC.h>
#include <chrono>
#include <BMS_Config.h>

#define SCK 36    // (CLK)
#define MISO 42   // (DO)
#define MOSI 40   // (DI)
#define SPI_CS 38 // (CS)

SPIClass spi = SPIClass(HSPI);
int counter = 0;  // 0: write column headers for .csv, 1: write data values
int file_counter = 0; // # = log#.csv (naming convention)
char name_buffer[50];
std::chrono::time_point<std::chrono::system_clock> start_time; // get the start timestamp

// listDir(), traverses through and lists all directories
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

// createDir(): uses FS.h to use mkdir()
void createDir(fs::FS &fs, const char * path){
  Serial.printf("Creating Dir: %s\n", path);
  if(fs.mkdir(path)){
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

// readFile(): read input file and print contents
void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

// writeFile(): create file w/ specific contents
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

// appendFile(): uses FS.h (file system) to append data onto end of file
void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
      Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

// spi_setup(): configure SPI for SD card 
void spi_setup(){
    delay(1000);
    Serial.println("spi_setup(): Running...");
    spi.begin(SCK, MISO, MOSI, SPI_CS);

    if (!SD.begin(SPI_CS,spi,80000000)) {
        Serial.println("Error: Card Mount Failed");
        return;
    }
  
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("Error: No SD Card Attached");
        return;
    } 
}

// set_log_start(): set log starting tme
void set_log_start(){
  start_time = std::chrono::high_resolution_clock::now();
}

// spi_write: write data to a .csv file, save file to micro-sd card
void spi_write(float temp, float current, float voltage){

  char buffer[50];

  // creates timestamp in seconds 
  auto current_time = std::chrono::high_resolution_clock::now();
  auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count(); // compute the elapsed time in milliseconds

  // convert int --> char, store min/sec
  char formatted_minutes[100];
  char formatted_seconds[100];

  int minutes = (timestamp / 60);
  int seconds = timestamp - (minutes * 60);

  // create format (min:sec)
  if (minutes < 10)
    sprintf(formatted_minutes, "0%d", minutes);
  else
    sprintf(formatted_minutes, "%d", minutes);
  
  if (seconds < 10)
    sprintf(formatted_seconds, "0%d", seconds);
  else
    sprintf(formatted_seconds, "%d", seconds);
  
  sprintf(buffer, "%s:%s,%f,%f,%f,0\n", formatted_minutes, formatted_seconds, temp, current, voltage);

  // 1st iteration, creates .csv file w/ column headers
  if (counter == 0){
    Serial.println("spi_write(): Running");
    file_counter += 1;  // used for dynamic naming of log files 
    sprintf(name_buffer, "/log%d.csv", file_counter);
    writeFile(SD, name_buffer, "Time(min:sec), Temp,I_o,V_bat,Bat%\n");
    counter += 1;
  }
  
  // post 1st iteration, append to existing .csv file
  appendFile(SD, name_buffer, buffer);
  Serial.println(buffer);
  readFile(SD, name_buffer);
  Serial.println();
    
}

#endif

