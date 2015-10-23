/**************************************************************************
  The MIT License (MIT)

  Copyright (c) 2015 by bbx10node@gmail.com

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
 ************************************************************************/

/*
 * ESP8266 SPIFFS versus SD write and read performance
 */

#define USE_SPIFFS (1)

#if USE_SPIFFS
#include <FS.h>
#else
#include <SPI.h>
#include <SD.h>
#endif

void setup() {
  Serial.begin(115200);
  Serial.println();

  Serial.println(F("WARNING: This test writes to Flash in an endless loop so running it for\r\n"
        "long periods may reduce the life of your Flash."));

#if USE_SPIFFS
  Serial.print(F("SPIFFS begin "));
  if (SPIFFS.begin()) {
    Serial.println(F("OK"));
  }
  else {
    Serial.println(F("fail"));
  }
#else
  Serial.println();
  Serial.println(F("SD begin"));
  if (SD.begin(4)) {
    Serial.println(F("OK"));
  }
  else {
    Serial.println(F("fail"));
  }
  //SPI.setFrequency(8000000);
#endif
}

static uint8_t FSbuffer[8 * 1024];

#if USE_SPIFFS
void SPIFFS_test(const char *fileName)
{
  uint32_t startMillis, deltaMillis;
  uint32_t bytesIn, bytesOut, bytesTotal;

  memset(FSbuffer, 0xAA, sizeof(FSbuffer));
  SPIFFS.remove(fileName);

  // Write test file
  startMillis = millis();
  File f = SPIFFS.open(fileName, "w");
  if (!f) {
    Serial.println(F("open for write failed"));
    return;
  }
  bytesTotal = 0;
  for (int i = 0; i < (1 * 1024 * 1024) / sizeof(FSbuffer); i++) {
    if ((bytesOut = f.write(FSbuffer, sizeof(FSbuffer))) != sizeof(FSbuffer)) {
      Serial.println(F("write failed"));
      return;
    }
    bytesTotal += bytesOut;
  }
  f.close();

  deltaMillis = millis() - startMillis;
  Serial.println();
  Serial.print(deltaMillis); Serial.println(F(" msecs"));
  Serial.print(bytesTotal); Serial.println(F(" bytes"));
  Serial.print((bytesTotal * 1000) / deltaMillis); Serial.println(F(" write bytes/sec"));

  // Read test file
  startMillis = millis();
  f = SPIFFS.open(fileName, "r");
  if (!f) {
    Serial.println(F("open for read failed"));
    return;
  }

  bytesTotal = 0;
  while (f.available() > 0) {
    if ((bytesIn = f.read(FSbuffer, sizeof(FSbuffer))) > 0) {
      bytesTotal += bytesIn;
    }
  }
  f.close();

  deltaMillis = millis() - startMillis;
  Serial.println();
  Serial.print(deltaMillis); Serial.println(F(" msecs"));
  Serial.print(bytesTotal); Serial.println(F(" bytes"));
  Serial.print((bytesTotal * 1000) / deltaMillis); Serial.println(F(" read bytes/sec"));

  SPIFFS.remove(fileName);
}

#else
void SD_test(const char *fileName)
{
  uint32_t startMillis, deltaMillis;
  uint32_t bytesIn, bytesOut, bytesTotal;

  memset(FSbuffer, 0xAA, sizeof(FSbuffer));
  //if (SD.exists(fileName)) SD.remove(fileName);

  // Write test file
  startMillis = millis();
  File f = SD.open(fileName, FILE_WRITE);
  if (!f) {
    Serial.println(F("open for write failed"));
    return;
  }
  f.seek(0);
  bytesTotal = 0;
  for (int i = 0; i < (1 * 1024 * 1024) / sizeof(FSbuffer); i++) {
    if ((bytesOut = f.write(FSbuffer, sizeof(FSbuffer))) != sizeof(FSbuffer)) {
      Serial.println(F("write failed"));
      return;
    }
    bytesTotal += bytesOut;
    yield();
  }
  f.close();

  deltaMillis = millis() - startMillis;
  Serial.println();
  Serial.print(deltaMillis); Serial.println(F(" msecs"));
  Serial.print(bytesTotal); Serial.println(F(" bytes"));
  Serial.print((bytesTotal * 1000) / deltaMillis); Serial.println(F(" write bytes/sec"));

  // Read test file
  Serial.println();
  startMillis = millis();
  f = SD.open(fileName);
  if (!f) {
    Serial.println(F("open for read failed"));
    return;
  }

  bytesTotal = 0;
  while (f.available() > 0) {
    if ((bytesIn = f.read(FSbuffer, sizeof(FSbuffer))) > 0) {
      bytesTotal += bytesIn;
    }
    yield();
  }
  f.close();

  deltaMillis = millis() - startMillis;
  Serial.print(deltaMillis); Serial.println(F(" msecs"));
  Serial.print(bytesTotal); Serial.println(F(" bytes"));
  Serial.print((bytesTotal * 1000) / deltaMillis); Serial.println(F(" read bytes/sec"));

  //SD.remove(fileName);
}
#endif

void loop() {
  Serial.println();
#if USE_SPIFFS
  Serial.println(F("SPIFFS Performance"));
  SPIFFS_test("/testfile.dat");
#else
  Serial.println("SD Performance");
  SD_test("/testfile.dat");
#endif
  delay(1000);
}

