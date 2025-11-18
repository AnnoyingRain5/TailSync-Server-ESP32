#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <libTailSync.h>

uint8_t Broadcast_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void handleColourPacket(ColourPacket packet) {
  Serial.printf("[LOG]: colour! r: %d g: %d b: %d", packet.colour[0][0].red,
                packet.colour[0][0].green, packet.colour[0][0].blue);
  Serial.printf("[LOG]: Average head colour: %d",
                AverageColour(packet.colour[1][3], packet.colour[1][4],
                              packet.colour[2][3], packet.colour[2][4]));
}

void handlePulsePacket() { Serial.println("[LOG]: pulse!"); }

void handleEndSessionPacket() { Serial.println("[LOG]: End session"); }

void setup() {
  Serial.begin(921600);
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    while (1) {
      Serial.println("[FATAL]: Failed to initialize ESP-NOW");
    }
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, Broadcast_MAC, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    while (1) {
      Serial.println("[FATAL]: Failed to initialize ESP-NOW peer");
    }
  }
}

void loop() {
  static uint8_t serialBuffer[192];
  uint16_t len =
      Serial.readBytesUntil('\n', serialBuffer, sizeof(serialBuffer));

  if (len < 192) {
    return;
  }

  PacketHeader header{0x54, 0x53, 0x01};
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

  esp_now_send(Broadcast_MAC, out, sizeof(out));
  Serial.flush();
  Serial.write("data sent");
}
