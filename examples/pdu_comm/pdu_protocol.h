#ifndef PDU_PROTOCOL_H
#define PDU_PROTOCOL_H

#include <stdint.h>

// Portable packed attribute macro
#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

// -----------------------------------------------------------------------------
// Artemis PDU ↔ OBC Simple Protocol Definitions (with Torque Coil Support)
// v1.0 - June 27 2025
// -----------------------------------------------------------------------------

/**
 * @brief Packet types exchanged between the PDU and the On-Board Computer (OBC).
 *
 * - CommandPing       : OBC requests a Pong response to verify link.
 * - CommandSetSwitch  : OBC commands PDU to set a particular switch state.
 * - CommandGetSwitchStatus: OBC requests current switch states.
 * - CommandGetTelemetry: OBC requests full PDU telemetry (voltages, currents, temps). TODO
 * - CommandSetTorque  : OBC commands PDU to set torque coil PWM output.
 * - DataPong          : PDU acknowledges a ping.
 * - DataSwitchTelem   : PDU returns switch states for one or all switches.
 * - DataPDUTelemetry  : PDU returns voltage/current/temperature telemetry.
 * - DataTorqueTelem   : PDU returns actual drive currents or status from torque coils.
 */
typedef enum {
    NOP = 0,  /**< No operation */
    CommandPing,            /**< Request a Pong response */
    CommandSetSwitch,       /**< Set one switch on/off */
    CommandGetSwitchStatus, /**< Get state of one or all switches */
    CommandGetTelemetry,    /**< Get full PDU telemetry */
    CommandSetTorque,       /**< Set PWM for torque coil */
    DataPong,               /**< Pong response */
    DataSwitchTelem,        /**< Switch state telemetry */
    DataPDUTelemetry,       /**< Power & thermal telemetry */
    DataTorqueTelem         /**< Torque coil telemetry */
} PDU_Type;

/**
 * @brief Identifiers for PDU switches and torque coil drivers.
 *
 * "All" affects every switch; "None" is used when no switch action is desired.
 */
typedef enum {
    None      = 0,  /**< No switch/coil selected */
    All,            /**< All switches */
    SW_3V3_1,       /**< 3.3 V bus #1 */
    SW_3V3_2,       /**< 3.3 V bus #2 */
    SW_5V_1,        /**< 5 V bus #1 */
    SW_5V_2,        /**< 5 V bus #2 */
    SW_5V_3,        /**< 5 V bus #3 */
    SW_5V_4,        /**< 5 V bus #4 */
    SW_12V,         /**< 12 V bus enable */
    VBATT,          /**< Battery main bus */
    WDT,            /**< Watchdog reset control */
    HBRIDGE1,       /**< Torque coil driver 1 */
    HBRIDGE2,       /**< Torque coil driver 2 */
    BURN,           /**< Generic burn-wire command */
    BURN1,          /**< Burn-wire heater #1 */
    BURN2,          /**< Burn-wire heater #2 */
    RPI             /**< Raspberry Pi power control */
} PDU_SW;

/**
 * @brief Simple command/status packet structure.
 *
 * - type: specifies the command or data packet type.
 * - sw:   selects which switch or coil the packet refers to.
 * - sw_state: 0 = off, 1 = on (used for switch commands/status).
 * - trq_value: PWM duty (0–255) for torque coil commands.
 */
typedef struct PACKED {
    PDU_Type type;     /**< Packet type */
    PDU_SW    sw;       /**< Switch or coil identifier */
    uint8_t   sw_state; /**< Switch on/off state */
    uint8_t   trq_value;/**< Torque PWM value */
} pdu_packet;

/**
 * @brief Telemetry packet reporting all switch states.
 *
 * The order of sw_state[] entries matches PDU_SW enum, excluding None and All.
 */
typedef struct PACKED {
    PDU_Type type;      /**< Should be DataSwitchTelem */
    uint8_t sw_state[12]; /**< States of each switch (1=on,0=off) */
} pdu_telem;

#define PDU_VERSION_MAX_LEN 32

typedef struct PACKED {
    PDU_Type type; // DataPong
    char version[PDU_VERSION_MAX_LEN]; // Null-terminated version string
} pdu_pong_packet;

// -----------------------------------------------------------------------------
// Function prototypes
// -----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Send a PDU packet over UART/SPI/etc.
 * @param packet The packet to send.
 * @return 0 on success, negative on error.
 */
int32_t pdu_send(const pdu_packet *packet);

/**
 * @brief Receive a raw response string from the PDU for simple parsing.
 * @param response Buffer to fill with response characters.
 * @param max_len Maximum buffer length.
 * @return Number of bytes received, or negative on error.
 */
int32_t pdu_recv(char* response, uint16_t max_len);

// -----------------------------------------------------------------------------
// Interactive console helpers
// -----------------------------------------------------------------------------

/**
 * @brief Convert a switch name string to its PDU_SW enum value.
 * @param str Null-terminated switch name (e.g. "SW_3V3_1").
 * @return Corresponding PDU_SW value, or PDU_SW::None if not found.
 */
uint8_t get_sw(const char* str);

/**
 * @brief Display available command options to the console.
 */
void display_options();

/**
 * @brief Print current switch identifiers and their human-readable names.
 */
void display_switches();

/**
 * @brief Timeout counter (milliseconds) for UART operations.
 */
extern uint32_t timeout;

/**
 * @brief ASCII offset for PDU command characters.
 * 
 * This offset is used to convert between ASCII character representations
 * and numerical command values in the PDU protocol. Commands are typically
 * transmitted as ASCII characters starting from this offset value.
 * 
 * Example: CommandPing (value 1) would be transmitted as ASCII character 49 ('1')
 */
#define PDU_CMD_ASCII_OFFSET 48

#ifdef __cplusplus
}
#endif

#endif // PDU_PROTOCOL_H