#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <RadioLib.h>

#ifndef RADIOLIB_LORAWAN_JOIN_EUI
#define RADIOLIB_LORAWAN_JOIN_EUI  0x0000000000000000
#endif

#ifndef RADIOLIB_LORAWAN_DEV_EUI
#define RADIOLIB_LORAWAN_DEV_EUI   //your dev eui
#endif

#ifndef RADIOLIB_LORAWAN_APP_KEY
#define RADIOLIB_LORAWAN_APP_KEY   // your app key
#endif

#ifndef RADIOLIB_LORAWAN_NWK_KEY
#define RADIOLIB_LORAWAN_NWK_KEY   // you network key
#endif

#define LORAWAN_UPLINK_USER_PORT  2
#define LORAWAN_UPLINK_DATA_RATE  3

#define LORAWAN_UPLINK_PERIOD     10000 // ms

#define LORAWAN_UPLINK_DATA_MAX   115 // byte


// result code to text ...
String stateDecode(const int16_t result) {
  switch (result) {
  case RADIOLIB_ERR_NONE:
    return "ERR_NONE";
  case RADIOLIB_ERR_CHIP_NOT_FOUND:
    return "ERR_CHIP_NOT_FOUND";
  case RADIOLIB_ERR_PACKET_TOO_LONG:
    return "ERR_PACKET_TOO_LONG";
  case RADIOLIB_ERR_RX_TIMEOUT:
    return "ERR_RX_TIMEOUT";
  case RADIOLIB_ERR_CRC_MISMATCH:
    return "ERR_CRC_MISMATCH";
  case RADIOLIB_ERR_INVALID_BANDWIDTH:
    return "ERR_INVALID_BANDWIDTH";
  case RADIOLIB_ERR_INVALID_SPREADING_FACTOR:
    return "ERR_INVALID_SPREADING_FACTOR";
  case RADIOLIB_ERR_INVALID_CODING_RATE:
    return "ERR_INVALID_CODING_RATE";
  case RADIOLIB_ERR_INVALID_FREQUENCY:
    return "ERR_INVALID_FREQUENCY";
  case RADIOLIB_ERR_INVALID_OUTPUT_POWER:
    return "ERR_INVALID_OUTPUT_POWER";
  case RADIOLIB_ERR_NETWORK_NOT_JOINED:
	  return "RADIOLIB_ERR_NETWORK_NOT_JOINED";

  case RADIOLIB_ERR_DOWNLINK_MALFORMED:
    return "RADIOLIB_ERR_DOWNLINK_MALFORMED";
  case RADIOLIB_ERR_INVALID_REVISION:
    return "RADIOLIB_ERR_INVALID_REVISION";
  case RADIOLIB_ERR_INVALID_PORT:
    return "RADIOLIB_ERR_INVALID_PORT";
  case RADIOLIB_ERR_NO_RX_WINDOW:
    return "RADIOLIB_ERR_NO_RX_WINDOW";
  case RADIOLIB_ERR_INVALID_CID:
    return "RADIOLIB_ERR_INVALID_CID";
  case RADIOLIB_ERR_UPLINK_UNAVAILABLE:
    return "RADIOLIB_ERR_UPLINK_UNAVAILABLE";
  case RADIOLIB_ERR_COMMAND_QUEUE_FULL:
    return "RADIOLIB_ERR_COMMAND_QUEUE_FULL";
  case RADIOLIB_ERR_COMMAND_QUEUE_ITEM_NOT_FOUND:
    return "RADIOLIB_ERR_COMMAND_QUEUE_ITEM_NOT_FOUND";
  case RADIOLIB_ERR_JOIN_NONCE_INVALID:
    return "RADIOLIB_ERR_JOIN_NONCE_INVALID";
  case RADIOLIB_ERR_N_FCNT_DOWN_INVALID:
    return "RADIOLIB_ERR_N_FCNT_DOWN_INVALID";
  case RADIOLIB_ERR_A_FCNT_DOWN_INVALID:
    return "RADIOLIB_ERR_A_FCNT_DOWN_INVALID";
  case RADIOLIB_ERR_DWELL_TIME_EXCEEDED:
    return "RADIOLIB_ERR_DWELL_TIME_EXCEEDED";
  case RADIOLIB_ERR_CHECKSUM_MISMATCH:
    return "RADIOLIB_ERR_CHECKSUM_MISMATCH";
  case RADIOLIB_LORAWAN_NO_DOWNLINK:
    return "RADIOLIB_LORAWAN_NO_DOWNLINK";
  case RADIOLIB_LORAWAN_SESSION_RESTORED:
    return "RADIOLIB_LORAWAN_SESSION_RESTORED";
  case RADIOLIB_LORAWAN_NEW_SESSION:
    return "RADIOLIB_LORAWAN_NEW_SESSION";
  case RADIOLIB_LORAWAN_NONCES_DISCARDED:
    return "RADIOLIB_LORAWAN_NONCES_DISCARDED";
  case RADIOLIB_LORAWAN_SESSION_DISCARDED:
    return "RADIOLIB_LORAWAN_SESSION_DISCARDED";
  }
  return "See TypeDef.h";
}

// helper function to display any issues
void debug(bool isFail, const __FlashStringHelper* message, int state, bool Freeze) {
  if (isFail) {
    Serial.print(message);
    Serial.print(" - ");
    Serial.print(stateDecode(state));
    Serial.print(" (");
    Serial.print(state);
    Serial.println(")");
    while (Freeze);
  }
}

// helper function to display a byte array
void arrayDump(uint8_t *buffer, uint16_t len) {
  for(uint16_t c = 0; c < len; c++) {
    char b = buffer[c];
    if(b < 0x10) { Serial.print('0'); }
    Serial.print(b, HEX);
  }
  Serial.println();
}

void memcpyr(uint8_t *dst, const uint8_t *src, uint16_t size)
{
    dst = dst + ( size - 1 );
    while( size-- )
    {
        *dst-- = *src++;
    }
}

#endif
