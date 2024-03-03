#ifndef MY_CHIP8
#define MY_CHIP8
#include <stdbool.h>
#include <stdint.h>

// Emulator configuration
typedef struct {
  uint32_t window_width;
  uint32_t window_height;
  uint32_t scale_factor;
  uint32_t fg_color;
  uint32_t bg_color;
  bool pixel_outline;
} config_t;

// Emulator states
typedef enum {
  QUIT,
  RUNNING,
  PAUSED,
} emulator_state_t;

// CHIP8 Instruction
// CHIP8 instructions are big endian, while x86 architecture is little endian
typedef union {
  struct {
    uint16_t NNN : 12; // 12-bit address/constant
    uint8_t MSN : 4;   // Most Significant Nibble
  } nnn;
  struct {
    uint8_t NN : 8;  // 8-bit constant
    uint8_t X : 4;   // 4-bit register
    uint8_t MSN : 4; // Most Significant Nibble
  } xnn;
  struct {
    uint8_t N : 4;   // 4-bit constant
    uint8_t Y : 4;   // 4-bit register
    uint8_t X : 4;   // 4-bit register
    uint8_t MSN : 4; // Most Significant Nibble
  } xyn;
  uint16_t opcode;
} instruction_t;

// CHIP8 machine
typedef struct {
  emulator_state_t state;
  uint8_t ram[4096];
  bool display[64 * 32]; // Default resolution
  uint16_t stack[12];    // Stack
  uint16_t I;            // Index register
  uint16_t PC;           // Program counter
  uint8_t V[16];         // Data registers
  uint8_t delay;         // Delay timer
  uint8_t sound;         // Sound timer
  bool keypad[16];       // Hexadecimal keypad
} chip8_t;

int init_chip8(chip8_t *chip8, const char *rom_name);
void emulate_instruction(chip8_t *chip8, const config_t config);

#endif
