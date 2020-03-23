#include "flash.h"

#include <SPI.h>
#include "Adafruit_SPIFlash.h"

#define SECTOR(x) ((int)((x)/SFLASH_SECTOR_SIZE))
#define BLOCK(x) ((int)((x)/SFLASH_BLOCK_SIZE))

Adafruit_FlashTransport_QSPI flashTransport(PIN_QSPI_SCK, PIN_QSPI_CS, PIN_QSPI_IO0, PIN_QSPI_IO1, PIN_QSPI_IO2, PIN_QSPI_IO3);
Adafruit_SPIFlash flash(&flashTransport);

File myFile;

void FlashFs::init() {
  // Init external flash
  if (!flash.begin()){
      Serial.println("Could not find flash on QSPI bus!");
      __BKPT();
      while(1);
  }
}

void FlashFs::WriteState(const char* ptr, long len, int byteoffset) {
  const size_t num_blocks = len/SFLASH_BLOCK_SIZE+1;
  for (size_t i=0; i<num_blocks; ++i) {
     flash.eraseBlock(BLOCK(byteoffset)+i);    
  }
  int num_bytes_written = flash.writeBuffer(byteoffset, (byte*)ptr, len);
 
}


bool FlashFs::ReadState(char* ptr, long len, int byteoffset) {
  int num_bytes_read = flash.readBuffer(byteoffset, (byte*)ptr, len);
}
