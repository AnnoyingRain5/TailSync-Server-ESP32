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
  Serial.begin(115200);
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
  uint8_t serialbuffer[3];
  while (1) {
    if (Serial.available() >= 3) {
      break;
    }
  }
  int message = Serial.readBytesUntil('\n', serialbuffer, sizeof(serialbuffer));
  Serial.flush();
  PacketHeader header = {0x54, 0x53, 0x01};
  ColourPacket packet = {};
  packet.colour[0][0].red = serialbuffer[0];
  packet.colour[0][0].green = serialbuffer[1];
  packet.colour[0][0].blue = serialbuffer[2];

  uint8_t buffer[sizeof(header) + sizeof(packet)];
  memcpy(buffer, &header, sizeof(header));
  memcpy(buffer + sizeof(header), &packet, sizeof(packet));
  esp_now_send(Broadcast_MAC, buffer, sizeof(buffer));
  delay(1000);
}
