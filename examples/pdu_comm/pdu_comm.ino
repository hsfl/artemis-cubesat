/**
 * @file pdu_comm.ino
 * @brief Artemis PDU Communication Example
 *
 * This Arduino sketch demonstrates communication with the Artemis Power Distribution Unit (PDU).
 * It provides an interactive console interface for:
 *
 * - Sending ping commands to verify PDU connectivity
 * - Controlling power switches (3.3V, 5V, 12V buses, heaters, burn wires, etc.)
 * - Reading switch status and telemetry data
 * - Configuring torque coils for attitude control
 * - Displaying available commands and switch options
 *
 * The code uses the PDU protocol defined in pdu_protocol.h to communicate over UART
 * with the PDU board, converting numerical commands to ASCII for transmission.
 *
 * @author Artemis CubeSat Team
 * @version 1.0
 */

#include <stdint.h>
#include <string>
#include <string.h>
/** NOTE FOR NEW USERS: There is a submodule/root folder called artemis-cubesat-protocols.
 * You will need to go to /pdu folder and copy the pdu_protocol.h into the the same folder as this (/pdu_comm)
 * As of now, assume v1.0 of the protocol (June 27 2025. Confirmed working with PDU revision board v2.2 with MCU software v2.2.1)
 */
#include "pdu_protocol.h"

// Global timeout variable
uint32_t timeout = 0;

void setup() {
  pdu_packet pdu_packet;
  char response[256];

  Serial.begin(9600);
  Serial1.begin(9600);  // Begin Serial Connection with the Artemis PDU on Serial1 (9600 Baud Rate for USART)

  while (!Serial || !Serial1)
    ;

  // Ensure PDU is communicating with Teensy
  pdu_packet.type = PDU_Type::CommandPing;
  pdu_packet.sw = PDU_SW::None;
  pdu_packet.sw_state = 0;
  pdu_packet.trq_value = 0;

  while (1) {
    pdu_send(pdu_packet);
    if (pdu_recv(response, sizeof(response)) > 0) {
      if (response[0] == (uint8_t)PDU_Type::DataPong + PDU_CMD_ASCII_OFFSET) {
        // Decode ASCII-encoded response to binary struct
        pdu_pong_packet pong_pkt;
        pong_pkt.type = (PDU_Type)response[0];
        memcpy(pong_pkt.version, response + 1, PDU_VERSION_MAX_LEN);
        pong_pkt.version[PDU_VERSION_MAX_LEN - 1] = '\0';
        Serial.println("PDU Connection Established");
        Serial.print("PDU Version: ");
        Serial.println(pong_pkt.version);
        break;
      }
    }
    delay(500);
  }

  display_options();
}

void loop() {
  pdu_packet pdu_packet;
  char response[256];

  if (Serial.available() > 0) {
    String input = Serial.readString();
    Serial.println(input);
    input = input.toLowerCase();

    if (input.indexOf("help") >= 0) {
      display_options();
    } else if (input.indexOf("list") >= 0) {
      display_switches();
    } else if (input.indexOf("ping") >= 0) {
      pdu_packet.type = PDU_Type::CommandPing;
      pdu_packet.sw = PDU_SW::None;
      pdu_packet.sw_state = 0;
      pdu_packet.trq_value = 0;

      timeout = 0;
      while (1) {
        pdu_send(pdu_packet);
        if (pdu_recv(response, sizeof(response)) > 0) {
          if (response[0] == (uint8_t)PDU_Type::DataPong + PDU_CMD_ASCII_OFFSET) {
            // Decode ASCII-encoded response to binary struct
            pdu_pong_packet pong_pkt;
            pong_pkt.type = (PDU_Type)response[0];
            memcpy(pong_pkt.version, response + 1, PDU_VERSION_MAX_LEN);
            pong_pkt.version[PDU_VERSION_MAX_LEN - 1] = '\0';
            Serial.println("PDU Connection Established");
            Serial.println("Got Pong");
            Serial.print("PDU Version: ");
            Serial.println(pong_pkt.version);
            break;
          }
        }

        if (timeout > 5000) {
          Serial.println("Unable to Ping PDU");
          break;
        }

        delay(100);
      }

      Serial.print("$ ");
    } else if (input.indexOf("get") >= 0 || input.indexOf("set") >= 0) {
      uint8_t sw = get_sw(input.c_str());
      if (sw == 0) {
        Serial.println("Invalid Switch");
        display_options();
        goto end;
      }
      pdu_packet.sw = (PDU_SW)sw;

      if (input.indexOf("set") >= 0) {
        pdu_packet.type = PDU_Type::CommandSetSwitch;

        if (input.indexOf("on") > 0) {
          pdu_packet.sw_state = 1;
        } else if (input.indexOf("off") > 0) {
          pdu_packet.sw_state = 0;
        } else {
          Serial.println("Please indicate ON or OFF");
          goto end;
        }
      } else if (input.indexOf("get") >= 0) {
        pdu_packet.type = PDU_Type::CommandGetSwitchStatus;
        pdu_packet.sw_state = 0;
      }

      pdu_packet.trq_value = 0;

      while (1) {
        pdu_send(pdu_packet);

        timeout = 0;
        int attempts = 1;
        while (pdu_recv(response, sizeof(response)) < 0) {
          if (timeout > 5000) {
            Serial.print("Attempt ");
            Serial.print(attempts);
            Serial.println(": FAIL TO SEND CMD TO PDU");
            timeout = 0;

            if (++attempts == 5) {
              goto end;
            }
          }
        }
        if ((response[1] == sw + PDU_CMD_ASCII_OFFSET) || (sw == (uint8_t)PDU_SW::All && response[0] == (uint8_t)PDU_Type::DataSwitchTelem + PDU_CMD_ASCII_OFFSET)) {
          break;
        }

        delay(100);
      }

      Serial.print("UART RECV: ");
      Serial.println(response);
      Serial.print("$ ");
      goto end;
    } else {
      Serial.println("Invalid Command (Type 'help' for help)");
      Serial.print("$ ");
      goto end;
    }
  }

end:;
}

int32_t pdu_send(const pdu_packet &packet) {
  char *cmd = (char *)malloc(sizeof(packet));
  memcpy(cmd, &packet, sizeof(packet));

  std::string msg = "";
  for (size_t i = 0; i < sizeof(packet); i++) {
    msg += (cmd[i] + PDU_CMD_ASCII_OFFSET);
  }
  Serial1.print(msg.c_str());
  Serial1.print('\n');
  Serial1.flush();

  Serial.print("SENDING TO PDU: [");
  for (size_t i = 0; i < msg.length(); i++) {
    Serial.print((unsigned)(msg[i] - PDU_CMD_ASCII_OFFSET));
  }
  Serial.println(']');

  free(cmd);
  delay(1);
  return 0;
}

int32_t pdu_recv(char *response, uint16_t max_len) {
  if (Serial1.available() > 0) {
    String UART1_RX = Serial1.readString();
    if (UART1_RX.length() > 0) {
      strncpy(response, UART1_RX.c_str(), max_len - 1);
      response[max_len - 1] = '\0';  // Ensure null termination

      Serial1.clear();
      return UART1_RX.length();
    }
  }

  return -1;
}

uint8_t get_sw(const char *str) {
  if (strstr(str, "sw_3v3_1") != nullptr) {
    return (uint8_t)PDU_SW::SW_3V3_1;
  }
  if (strstr(str, "sw_3v3_2") != nullptr) {
    return (uint8_t)PDU_SW::SW_3V3_2;
  }
  if (strstr(str, "sw_5v_1") != nullptr) {
    return (uint8_t)PDU_SW::SW_5V_1;
  }
  if (strstr(str, "sw_5v_2") != nullptr) {
    return (uint8_t)PDU_SW::SW_5V_2;
  }
  if (strstr(str, "sw_5v_3") != nullptr) {
    return (uint8_t)PDU_SW::SW_5V_3;
  }
  if (strstr(str, "sw_5v_4") != nullptr) {
    return (uint8_t)PDU_SW::SW_5V_4;
  }
  if (strstr(str, "sw_12v") != nullptr) {
    return (uint8_t)PDU_SW::SW_12V;
  }
  if (strstr(str, "vbatt") != nullptr) {
    return (uint8_t)PDU_SW::VBATT;
  }
  if (strstr(str, "hbridge1") != nullptr) {
    return (uint8_t)PDU_SW::HBRIDGE1;
  }
  if (strstr(str, "hbridge2") != nullptr) {
    return (uint8_t)PDU_SW::HBRIDGE2;
  }
  if (strstr(str, "burnall") != nullptr) {
    return (uint8_t)PDU_SW::BURN;
  }
  if (strstr(str, "burn1") != nullptr) {
    return (uint8_t)PDU_SW::BURN1;
  }
  if (strstr(str, "burn2") != nullptr) {
    return (uint8_t)PDU_SW::BURN2;
  }
  if (strstr(str, "all") != nullptr) {
    return (uint8_t)PDU_SW::All;
  }
  return 0;
}

void display_options() {
  Serial.println("\n\n");
  Serial.println("PDU Command Options");
  Serial.println("========================================================");
  Serial.println("(1) Ping");
  Serial.println("\tExample Usage:\t$ Ping");
  Serial.println("(2) Set Switch ON/OFF");
  Serial.println("\tExample Usage:\t$ SET SW_3V3_1 ON");
  Serial.println("(3) Get Switch Status");
  Serial.println("\tExample Usage:\t$ GET SW_3V3_1");
  Serial.println("(4) List Available Switches");
  Serial.println("\tExample Usage:\t$ List");
  Serial.print("\n\n$ ");
}

void display_switches()
{
  Serial.println("\n\n");
  Serial.println("Available Switches");
  Serial.println("========================================================");
  Serial.println("\t All (This can be used to get all the switch statuses or turn them all on/off.)");
  Serial.println("\t SW_3V3_1");
  Serial.println("\t SW_3V3_2");
  Serial.println("\t SW_5V_1");
  Serial.println("\t SW_5V_2");
  Serial.println("\t SW_5V_3");
  Serial.println("\t SW_5V_4");
  Serial.println("\t SW_12V");
  Serial.println("\t VBATT");
  Serial.println("\t HBRIDGE1");
  Serial.println("\t HBRIDGE2");
  Serial.println("\t BURN (This switch is for both BURN1 and BURN2.)");
  Serial.println("\t BURN1");
  Serial.println("\t BURN2");
  Serial.print("\n\n$ ");
}