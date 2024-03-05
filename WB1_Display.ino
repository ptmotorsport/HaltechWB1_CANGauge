#include <mcp_can.h>
#include <SPI.h>
#include <U8g2lib.h>

// Initialize MCP2515 on SPI CS Pin 10 
MCP_CAN CAN(10);
#define CAN0_INT 2                              // Set INT to pin 2
#define CAN_ID_1 0x2B1                          // Set CAN ID 1 to 0x2B1
#define CAN_ID_2 0x2B0                          // Set CAN ID 2 to 0x2B0

// Initialize OLED screen on I2C address 0x78
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);
// 'PT logo', 29x13px
const unsigned char bmpPT_logo [] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x05, 0x5f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x05, 0x5c, 0x1c, 0x06, 0x60, 0xc3, 0xc3, 0xf8, 0xf0, 0x7c, 0x1e, 0x1f, 0x07, 0x83, 0xe3, 0xfc, 
	0x05, 0x58, 0x08, 0x06, 0x71, 0xc7, 0xf7, 0xf9, 0xf8, 0x7e, 0x3f, 0x3f, 0x8f, 0xc3, 0xf3, 0xfc, 
	0x05, 0x58, 0xcf, 0x3e, 0x71, 0xce, 0x70, 0xc3, 0x9c, 0x67, 0x72, 0x31, 0x9c, 0xe3, 0x38, 0xe0, 
	0x0a, 0xd9, 0xc7, 0x3e, 0x73, 0xcc, 0x30, 0xc3, 0x0c, 0xc3, 0x70, 0x31, 0x98, 0x67, 0x18, 0xc0, 
	0x0a, 0xb9, 0xce, 0x3c, 0xf3, 0xdc, 0x30, 0xc7, 0x0c, 0xc6, 0x38, 0x31, 0xb8, 0x66, 0x30, 0xc0, 
	0x0a, 0xb8, 0x0e, 0x3c, 0xd6, 0x98, 0x31, 0xc6, 0x0c, 0xfe, 0x1e, 0x7f, 0x30, 0x67, 0xf0, 0xc0, 
	0x0a, 0xb0, 0x3e, 0x7c, 0xdd, 0x98, 0x31, 0x86, 0x0c, 0xf8, 0x0e, 0x7e, 0x30, 0x67, 0xc0, 0xc0, 
	0x0a, 0xb1, 0xfe, 0x7c, 0xd9, 0x9c, 0x71, 0x86, 0x19, 0xd8, 0x06, 0x60, 0x30, 0xc6, 0xe1, 0xc0, 
	0x15, 0xb3, 0xfe, 0x7d, 0xd9, 0x8c, 0xe1, 0x87, 0x39, 0xcc, 0xe6, 0x60, 0x39, 0xce, 0x61, 0x80, 
	0x15, 0x73, 0xfc, 0x79, 0x83, 0x8f, 0xc3, 0x83, 0xf1, 0x8c, 0xfe, 0x60, 0x1f, 0x8e, 0x71, 0x80, 
	0x15, 0x7f, 0xff, 0xf8, 0x81, 0x03, 0x01, 0x00, 0xc1, 0x84, 0x38, 0x40, 0x06, 0x04, 0x21, 0x80, 
	0x15, 0x7f, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};



void setup() {
    Serial.begin(115200); // Start the serial communication with the baud rate of 115200
    u8g2.begin();
    
    // Display a startup message
    u8g2.firstPage();
    do {
         u8g2.drawBitmap(2, 26, 128/8, 16, bmpPT_logo);
   } 
 while (u8g2.nextPage());

    delay(2000); // Keep the startup message on the screen for 2 seconds

     // Start SPI CAN module
    while (CAN_OK != CAN.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ)) {
        Serial.println("CAN initialization failed. Retrying...");
        delay(100);
    }
    Serial.println("CAN initialization successful.");
    CAN.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.
    pinMode(CAN0_INT, INPUT);                            // Configuring pin for /INT input
    // Set up filters
    CAN.init_Mask(0, 0, 0x7FC); // Use mask to filter out irrelevant bits
    CAN.init_Filt(0, 0, CAN_ID_1); // Filter for CAN ID 1
    CAN.init_Filt(1, 0, CAN_ID_2); // Filter for CAN ID 2
}

void loop() {
    uint8_t len = 0;
    uint8_t buf[8];
    uint32_t canId;

    // Check if a message is available and read it
    if (readCANMessage(&canId, &len, buf)) {
        // Process the message if it has the correct CAN ID
        if (canId == 0x2B1) {
          processCANMessage(canId ,buf, len);
        } else {
            Serial.println("Ignoring message with different CAN ID.");
        }
    } else {
        Serial.println("No CAN message available.");
    }
    // Check if a message is available and read it
    if (readCANMessage(&canId, &len, buf)) {
        // Process the message if it has the correct CAN ID
        if (canId == 0x2B0) {
          processCANMessage(canId, buf, len);
        } else {
            Serial.println("Ignoring message with different CAN ID.");
        }
    } else {
        Serial.println("No CAN message available.");
    }
}

unsigned long lastCANMessageTime = 0;
#define CAN_MESSAGE_TIMEOUT 1000  // Timeout in milliseconds

bool readCANMessage(uint32_t* canId, uint8_t* len, uint8_t* buf) {
    // Check if a message is available
    if(!digitalRead(CAN0_INT)) {
        Serial.println("CAN message available.");

        // Read the message
        if (CAN_OK == CAN.readMsgBuf(canId, len, buf)) {
            Serial.println("CAN message read successfully.");
            lastCANMessageTime = millis();  // Update the time of the last successful read
            return true;
        } else {
            Serial.println("Failed to read CAN message.");
            return false;
        }
    } else {
        Serial.println("No CAN message available.");

        // Only display the error message if no CAN message has been available for a certain amount of time
        if (millis() - lastCANMessageTime > CAN_MESSAGE_TIMEOUT) {
            // No CAN message available, display an error message
            u8g2.firstPage();
            do {
                u8g2.setFont(u8g2_font_courB14_tf);
                int strWidth = u8g2.getStrWidth("CAN error");
                u8g2.drawStr((128 - strWidth) / 2, 40, "CAN error");
                u8g2.setFont(u8g2_font_helvR08_tr);
                strWidth = u8g2.getStrWidth("PT Motorsport");
                u8g2.drawStr((128 - strWidth) / 2, 60, "PT Motorsport");
            } while (u8g2.nextPage());
            //delay(2000);  // Wait for 2 seconds so you can see the message
        }
        return false;
    }
}

void processCANMessage(uint32_t canId, uint8_t* buf, uint8_t len) {
    if (canId == 0x2B1) {
    // Process the message...
    // Combine the first two bytes of the message to get the value
    unsigned int value = (buf[0] << 8) | buf[1];

    // Multiply the value by 0.001
    float scaledValue = value * 0.001;

    // Start drawing on the screen
    u8g2.firstPage();
    do {
        // Set the font
        u8g2.setFont(u8g2_font_helvR08_tr);

        // Print Pt Motorsport to the screen
        int strWidth = u8g2.getStrWidth("PT Motorsport");
        u8g2.drawStr((128 - strWidth) / 2, 60, "PT Motorsport");

        // Set the font
        u8g2.setFont(u8g2_font_courB18_tn);

        // Print the value and a lambda symbol to the screen
        String valueStr = String(scaledValue);
        strWidth = u8g2.getStrWidth(valueStr.c_str());
        u8g2.setCursor((128 - strWidth) / 2, 40);
        u8g2.print(scaledValue);

        // Extract the 4 bits starting at bit 51
        uint8_t state = buf[6];

        // Set the cursor to the desired position
        u8g2.setFont(u8g2_font_helvB10_tr); // Set the font to 8 pixels high

        // Only change the state message if the first two bytes are both 0x00
        String stateStr;
        if (buf[0] == 0x00 && buf[1] == 0x00) {
            // Display the state based on the CAN message
            switch (state) {
                case 0x00:
                    stateStr = "Lambda";
                    break;
                case 0x01:
                    stateStr = "Battery Low";
                    break;
                case 0x02:
                    stateStr = "Battery High";
                    break;
                case 0x03:
                    stateStr = "Short Circuit";
                    break;
                case 0x04:
                    stateStr = "Open Circuit";
                    break;
                case 0x05:
                    stateStr = "Sensor Cold";
                    break;
                case 0x06:
                    stateStr = "Free Air";
                    break;
                case 0x07:
                    stateStr = "HTR Short Circuit";
                    break;
                case 0x08:
                    stateStr = "Sensor Drying";
                    break;
                case 0x09:
                    stateStr = "Controller Off";
                    break;
                case 0x0A:
                    stateStr = "Engine Stopped";
                    break;
                case 0x0B:
                    stateStr = "Not Calibrated";
                    break;
                case 0x0C:
                    stateStr = "Calibrating";
                    break;
                default:
                    stateStr = "Lambda";
                    break;
            }
        } else {
            stateStr = "Lambda";
        }
        strWidth = u8g2.getStrWidth(stateStr.c_str());
        u8g2.setCursor((128 - strWidth) / 2, 15);
        u8g2.print(stateStr);
    } while (u8g2.nextPage());
    } else {
        Serial.println("Unknown CAN ID.");
      }
}
