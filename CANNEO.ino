#include <Arduino.h>
#include <SPI.h>
#include <MergCBUS.h>
#include <Message.h>
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>

#define GREEN_LED     A5     //merg green led port
#define YELLOW_LED     2     //merg yellow led port
#define PUSH_BUTTON   A4     //std merg push button
#define CANPORT       10
#define INTPIN         8
#define NEOPIN         4

/**
   Node Variables:
      1         Number of Pixels
*/
#define NUM_NODE_VARS  1    //the node variables
#define NUM_EVENTS     32   //supported events
#define NUM_EVENT_VARS 4    //supported event variables
#define NUM_DEVICES    1    //one device number
#define MODULE_ID      57   //module id
#define MANUFACTURER_ID 165 //manufacturer id
#define MIN_CODE       0    //min code version
#define MAX_CODE       1    //max code version

MergCBUS cbus = MergCBUS(NUM_NODE_VARS, NUM_EVENTS, NUM_EVENT_VARS, NUM_DEVICES);

Adafruit_NeoPixel  *strip;

void myUserFunc(Message * msg, MergCBUS * mcbus) {
  /* getting standard on/off events */
  boolean onEvent;

  if (mcbus->eventMatch()) {
    onEvent = mcbus->isAccOn();
    int pixel = mcbus->getEventVar(msg, 1); // Get first event variable

    if (onEvent)
    {
      uint8_t r = mcbus->getEventVar(msg, 2);
      uint8_t g = mcbus->getEventVar(msg, 3);
      uint8_t b = mcbus->getEventVar(msg, 4);
      uint32_t c = strip->Color(r, g, b);
      strip->setPixelColor(pixel, c);
      strip->show();
    }
    else
    {
      uint32_t c = strip->Color(0, 0, 0);
      strip->setPixelColor(pixel, c);
      strip->show();
    }
  }
}

void myUserFuncDCC(Message * msg, MergCBUS * mcbus) {
  //  Serial.print("DCC Code: ");
  //  Serial.println(msg->getOpc());
}

void setup() {
  Serial.begin(9600);
  //Configuration data for the node
  cbus.getNodeId()->setNodeName("NEOP", 4);        //node name
  cbus.getNodeId()->setModuleId(MODULE_ID);            //module number
  cbus.getNodeId()->setManufacturerId(MANUFACTURER_ID);//merg code
  cbus.getNodeId()->setMinCodeVersion(MIN_CODE);       //Version 1
  cbus.getNodeId()->setMaxCodeVersion(MAX_CODE);
  cbus.getNodeId()->setProducerNode(true);
  cbus.getNodeId()->setConsumerNode(true);
  cbus.setPushButton(PUSH_BUTTON);//set the push button ports
  cbus.setStdNN(999); //standard node number

  //used to manually reset the node. while turning on keep the button pressed
  //this forces the node for slim mode with an empty memory for learned events and devices
  if (digitalRead(PUSH_BUTTON) == LOW) {
    Serial.println("Setup new memory");
    cbus.setUpNewMemory();
    cbus.saveNodeFlags();
    cbus.setNodeVariable(1, 1);
  }

  cbus.setLeds(GREEN_LED, YELLOW_LED); //set the led ports

  cbus.setUserHandlerFunction(&myUserFunc);//function that implements the node logic
  cbus.setDCCHandlerFunction(&myUserFuncDCC);
  cbus.initCanBus(CANPORT, CAN_125KBPS, MCP_16MHz, 20, 30);  //initiate the transport layer
  cbus.setFlimMode();

  pinMode(INTPIN, INPUT);

  int stripSize = cbus.getNodeVar(1);
  strip = new Adafruit_NeoPixel(stripSize, NEOPIN, NEO_GRB + NEO_KHZ800);
  for (int i = 0; i < stripSize; i++)
  {
    uint32_t c = strip->Color(0, 0, 0);
    strip->setPixelColor(i, c);
  }
  strip->show();
}

void loop() {
  cbus.cbusRead();
  cbus.run();//do all logic
  // Custom loop code

  //debug memory
  if (digitalRead(PUSH_BUTTON) == LOW) {
    cbus.dumpMemory();
  }
}
