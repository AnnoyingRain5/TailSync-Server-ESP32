#include <Arduino.h>
#include <TailSyncLogging.h>
#include <WiFi.h>
#include <esp_now.h>
#include <libTailSync.h>

static Logger logger = Logger("Server");
uint8_t Broadcast_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t version_type = 0;
bool versionTypeSet = false;

uint8_t genNonce() {
  uint8_t result = (uint8_t)random(256);
  // if we get a collision, keep generating until we don't get one!
  while (result == lastNonce) {
    result = (uint8_t)random(256);
  }
  return result;
}

void handleColourPacket(ColourPacket packet) {
  logger.log(DEBUG, "Colour! r: %d g: %d b: %d", packet.colour[0][0].red,
             packet.colour[0][0].green, packet.colour[0][0].blue);
  logger.log(DEBUG, "Average head colour: %d",
             AverageColour(packet.colour[1][3], packet.colour[1][4],
                           packet.colour[2][3], packet.colour[2][4]));
}

void handlePulsePacket() { logger.log(DEBUG, "pulse!"); }

void handleEndSessionPacket() { logger.log(DEBUG, "End session"); }

void flushSerialRX() {
  while (Serial.available()) {
    Serial.read();
  }
}

void setup() {
  Serial.begin(460800);
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    while (1) {
      logger.log(FATAL, "Failed to initialize ESP-NOW");
    }
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, Broadcast_MAC, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    while (1) {
      logger.log(FATAL, "Failed to initialize ESP-NOW peer");
    }
  }
}

void loop() {

  if (Serial.available() >= 1 && !versionTypeSet) {
    version_type = Serial.read();
    versionTypeSet = true;
  }

  if (versionTypeSet) {
    switch (version_type) {

    // version 0 pulse
    case 0x00: {
      PacketHeader header{0x54, 0x53, 0x00, genNonce()};
      uint8_t out[sizeof(header)];
      memcpy(out, &header, sizeof(header));

      esp_now_send(Broadcast_MAC, out, sizeof(out));
      esp_now_send(Broadcast_MAC, out, sizeof(out));
      esp_now_send(Broadcast_MAC, out, sizeof(out));
      logger.log(DEBUG, "Sent Pulse");
      versionTypeSet = false;
      break;
    }

    // version 0 colour
    case 0x01: {
      static uint8_t serialBuffer[192];
      uint16_t len = Serial.readBytes(serialBuffer, sizeof(serialBuffer));
      if (len < 192) {
        // timeout reached and not enough data
        versionTypeSet = false;
        flushSerialRX();
        logger.log(ERROR, "Timeout reached, but not enough data was sent!");
        return;
      }

      PacketHeader header{0x54, 0x53, 0x01, genNonce()};
      ColourPacket packet{};

      for (uint16_t i = 0; i < len; i += 3) {
        uint16_t pixelIdx = i / 3;
        uint16_t row = pixelIdx / 8;
        uint16_t col = pixelIdx % 8;
        packet.colour[row][col].red = serialBuffer[i];
        packet.colour[row][col].green = serialBuffer[i + 1];
        packet.colour[row][col].blue = serialBuffer[i + 2];
      }
      uint8_t out[sizeof(header) + sizeof(packet)];
      memcpy(out, &header, sizeof(header));
      memcpy(out + sizeof(header), &packet, sizeof(packet));
      // send identical packet three times
      esp_now_send(Broadcast_MAC, out, sizeof(out));
      esp_now_send(Broadcast_MAC, out, sizeof(out));
      esp_now_send(Broadcast_MAC, out, sizeof(out));
      versionTypeSet = false;
      logger.log(DEBUG, "Sent Colour");
      break;
    }
    default: {
      logger.log(ERROR, "Unexpected versiontype: %d", version_type);
      versionTypeSet = false;
      flushSerialRX();
    }
    }
  }
}
