/* RFM69 library and code by Felix Rusu - felix@lowpowerlab.com
  // Get libraries at: https://github.com/LowPowerLab/
  // Make sure you adjust the settings in the configuration section below !!!
  // **********************************************************************************
  // Copyright Felix Rusu, LowPowerLab.com
  // Library and code by Felix Rusu - felix@lowpowerlab.com
  // **********************************************************************************
  // License
  // **********************************************************************************
  // This program is free software; you can redistribute it
  // and/or modify it under the terms of the GNU General
  // Public License as published by the Free Software
  // Foundation; either version 3 of the License, or
  // (at your option) any later version.
  //
  // This program is distributed in the hope that it will
  // be useful, but WITHOUT ANY WARRANTY; without even the
  // implied warranty of MERCHANTABILITY or FITNESS FOR A
  // PARTICULAR PURPOSE. See the GNU General Public
  // License for more details.
  //
  // You should have received a copy of the GNU General
  // Public License along with this program.
  // If not, see <http://www.gnu.org/licenses></http:>.
  //
  // Licence can be viewed at
  // http://www.gnu.org/licenses/gpl-3.0.txt
  //
  // Please maintain this license information along with authorship
  // and copyright notices in any redistribution of this code
  // **********************************************************************************/
#include <RFM69.h> //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NETWORKID 150 // The same on all nodes that talk to each other
#define NODEID 7 // The unique identifier of this node
#define RECEIVER 8 // The recipient of packets
//Match frequency to the hardware version of the radio on your Feather
#define FREQUENCY RF69_433MHZ
//#define FREQUENCY RF69_868MHZ
//#define FREQUENCY RF69_915MHZ
#define ENCRYPTKEY "testtesttesttest" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HCW true // set to 'true' if you are using an RFM69HCW module
//*********************************************************************************************
#define SERIAL_BAUD 115200
/* for Feather 32u4 Radio */
#define RFM69_CS 8
#define RFM69_IRQ 7
#define RFM69_IRQN 4 // Pin 7 is IRQ 4!
#define RFM69_RST 4

#define LED 13 // onboard blinky

int16_t packetnum = 0; // packet counter, we increment per xmission
RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);


void setup() {
  //while (!Serial); // wait until serial console is open, remove if not tethered to computer. Delete this line on ESP8266
  Serial.begin(SERIAL_BAUD);
  Serial.println("Uart init done. Starting Init.");
  // Hard Reset the RFM module
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, HIGH);
  delay(100);
  digitalWrite(RFM69_RST, LOW);
  delay(100);
  // Initialize radio

  radio.initialize(FREQUENCY, NODEID, NETWORKID);
  if (IS_RFM69HCW) {
    radio.setHighPower(); // Only for RFM69HCW & HW!
  }
  radio.setPowerLevel(31); // power output ranges from 0 (5dBm) to 31 (20dBm)
  radio.encrypt(ENCRYPTKEY);
  pinMode(LED, OUTPUT);
  Serial.print("Radio Init done. Transmitting at ");
  Serial.print(FREQUENCY == RF69_433MHZ ? 433 : FREQUENCY == RF69_868MHZ ? 868 : 915);
  Serial.println(" MHz");
}


//====================================================================
void loop() {
  char radiopacket[65];
  unsigned char radioPacketLen;
  static unsigned int loopCounter = 0;
  static unsigned int subLoopCounter = 0;



  //=================================
  // Test si un paquet est disponible
  //=================================
  if (radio.receiveDone())
  {
    // une trame a été recue
    // TODO : depiler les données de la trame

    Serial.print(F("Received Frame "));
    if (radio.ACKRequested())
    {
      radio.sendACK();
      Serial.println("Sending ACK");
    }
    else
    {
      Serial.println("NO ACK");
    }
    Blink(LED, 40, 1);
  }


  //======================================
  //Tous les 2000 passages ..
  //
  if (loopCounter > 2000)
  {
    subLoopCounter ++;
    packetnum ++;

    //==== On construit une trame arbitraire pour test
    radiopacket[0] = 01;
    radiopacket[1] = radio.readTemperature(4);
    radiopacket[2] = packetnum >> 8;  // Les poids forts
    radiopacket[3] = packetnum ;      // Les poids faibles
    radiopacket[4] = 00;
    radiopacket[5] = 255;

    radiopacket[6] = 11;
    radiopacket[7] = 22;
    radiopacket[8] = 33;
    radiopacket[9] = 0;
    radiopacket[10] = 0;
    radiopacket[11] = 0;
    radioPacketLen = 15;

    //
    if (radio.sendWithRetry(RECEIVER, radiopacket, radioPacketLen))
    { //target node Id, message as string or byte array, message length
      Serial.print("OK >");
      Blink(LED, 40, 3); //blink LED 3 times, 50ms between blinks
    }
    else
    {
      Serial.print("FAIL >");
    }



    Serial.print("[");   Serial.print(subLoopCounter);
    loopCounter = 0;
    Serial.print("] Send [");
    for (int i = 0; i < radioPacketLen; i++)
    {
      Serial.print((unsigned char) radiopacket[i], DEC);
      Serial.print(",");
    }
    Serial.println("]");

    //Dump des registres 
    //radio.readAllRegs();

  }


  Serial.flush(); //make sure all serial data is clocked out before sleeping the MCU
  loopCounter ++;
  delay(1);
}




void Blink(byte PIN, byte DELAY_MS, byte loops)
{
  for (byte i = 0; i < loops; i++)
  {
    digitalWrite(PIN, HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN, LOW);
    delay(DELAY_MS);
  }
}

