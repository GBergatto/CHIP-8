#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "chip8.h"

int init_chip8(chip8_t *chip8, const char *rom_name) {
  const uint32_t entrypoint = 0x200;
  const uint8_t font[] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
  };

  // Load font (by tradition, put it at 0x50–0x9F)
  memcpy(&chip8->ram[0x50], font, sizeof(font));

  // Seed random number generator
  srand(time(NULL));

  // Open ROM file
  FILE *rom_file = fopen(rom_name, "rb");
  if (rom_file == NULL) {
    fprintf(stderr, "Could not open ROM file %s\n", rom_name);
    return 1;
  }

  // Get ROM size
  fseek(rom_file, 0, SEEK_END);
  const size_t rom_size = ftell(rom_file);
  const size_t max_size = sizeof chip8->ram - entrypoint;
  rewind(rom_file);

  if (rom_size > sizeof(chip8->ram)) {
    fprintf(stderr, "ROM file %s is too large! Size: %zu. Max size: %zu\n",
            rom_name, rom_size, max_size);
    return 1;
  }

  // Load ROM
  size_t ret =
      fread(&chip8->ram[entrypoint], sizeof(uint8_t), rom_size, rom_file);
  if (ret != rom_size) {
    fprintf(stderr, "Could not load ROM into RAM\n");
    return 1;
  }
  fclose(rom_file);

  chip8->state = RUNNING;
  chip8->PC = entrypoint;
  return 0;
}

void emulate_instruction(chip8_t *chip8, const config_t config) {
  instruction_t inst;
  inst.opcode = (chip8->ram[chip8->PC] << 8) | chip8->ram[chip8->PC + 1];
  chip8->PC += 2; // Pre-increment program counter

  printf("%04X\n", inst.opcode);

  switch (inst.nnn.MSN) {
  case 0x0:
    if (inst.nnn.NNN == 0xE0) {
      // 0x00E0: clear the screen
      memset(&chip8->display[0], false, sizeof chip8->display);
    } else {
      // 0x00EE: return from subrutine
      if (chip8->SP == 0) {
        printf("Error: trying to pop from empty stack\n");
      }
      chip8->PC = chip8->stack[--chip8->SP];
    }
    break;
  case 0x1:
    // 0x1NNN: jump
    chip8->PC = inst.nnn.NNN;
    break;
  case 0x2:
    // 0x2NNN: call subroutine at NNN
    // Push current PC to the stack
    chip8->stack[chip8->SP++] = chip8->PC;
    if (chip8->SP > 12) {
      printf("Error: stack overflow\n");
    }
    // Jump to NNN
    chip8->PC = inst.nnn.NNN;
    break;
  case 0x3:
    // 0x3XNN: skip the next instruction if VX equals NN
    if (chip8->V[inst.xnn.X] == inst.xnn.NN) {
      chip8->PC += 2;
    }
    break;
  case 0x4:
    // 0x4XNN: skip the next instruction if VX does not equal NN
    if (chip8->V[inst.xnn.X] != inst.xnn.NN) {
      chip8->PC += 2;
    }
    break;
  case 0x5:
    // 0x5XYN: skip the next instruction if VX equals VY
    if (chip8->V[inst.xyn.X] == chip8->V[inst.xyn.Y]) {
      chip8->PC += 2;
    }
    break;
  case 0x6:
    // 0x6XNN: set VX to NN
    chip8->V[inst.xnn.X] = inst.xnn.NN;
    break;
  case 0x7:
    // 0x7XNN: add NN to VX (carry flag is not changed)
    chip8->V[inst.xnn.X] += inst.xnn.NN;
    break;
  case 0x8:
    switch (inst.xyn.N) {
    case 0x0:
      // 0x8XY0: set VX to the value of VY
      chip8->V[inst.xyn.X] = chip8->V[inst.xyn.Y];
      break;
    case 0x1:
      // 0x8XY1: set VX to VX bitwise OR VY
      chip8->V[inst.xyn.X] |= chip8->V[inst.xyn.Y];
      break;
    case 0x2:
      // 0x8XY2: set VX to VX bitwise AND VY
      chip8->V[inst.xyn.X] &= chip8->V[inst.xyn.Y];
      break;
    case 0x3:
      // 0x8XY3: set VX to VX bitwise XOR VY
      chip8->V[inst.xyn.X] ^= chip8->V[inst.xyn.Y];
      break;
    case 0x4:
      // 0x8XY4: add VY to VX (with carry flag)
      if ((uint16_t)(chip8->V[inst.xyn.X] + chip8->V[inst.xyn.Y]) > 255) {
        chip8->V[0xF] = 1;
      } else {
        chip8->V[0xF] = 0;
      }
      chip8->V[inst.xyn.X] += chip8->V[inst.xyn.Y];
      break;
    case 0x5:
      // 0x8XY5: VY is subtracted from VX
      if (chip8->V[inst.xyn.X] >= chip8->V[inst.xyn.Y]) {
        chip8->V[0xF] = 1; // no underflow
      } else {
        chip8->V[0xF] = 0; // underflow
      }
      chip8->V[inst.xyn.X] -= chip8->V[inst.xyn.Y];
      break;
    case 0x6:
      // 0x8XY6: right shift VX
      // Shift VY for original interpreter, ignore VY for CHIP-48 and SUPER-CHIP
      if (!config.shift_VX_only) {
        chip8->V[inst.xyn.X] = chip8->V[inst.xyn.Y];
      }
      // Store the least significant bit of VX in VF
      chip8->V[0xF] = chip8->V[inst.xyn.X] & 1;
      chip8->V[inst.xyn.X] >>= 1;
      break;
    case 0x7:
      // 0x8XY7: Sets VX to VY minus VX
      if (chip8->V[inst.xyn.Y] >= chip8->V[inst.xyn.X]) {
        chip8->V[0xF] = 1; // no underflow
      } else {
        chip8->V[0xF] = 0; // underflow
      }
      chip8->V[inst.xyn.X] = chip8->V[inst.xyn.Y] - chip8->V[inst.xyn.X];
      break;
    case 0xE:
      // 0x8XYE: left shift VX
      // Shift VY for original interpreter, ignore VY for CHIP-48 and SUPER-CHIP
      if (!config.shift_VX_only) {
        chip8->V[inst.xyn.X] = chip8->V[inst.xyn.Y];
      }
      // Store the most significant bit of VX in VF
      chip8->V[0xF] = (chip8->V[inst.xyn.X] & (1 << 7)) >> 7;
      chip8->V[inst.xyn.X] <<= 1;
      break;
    }
    break;
  case 0x9:
    // 0x9XYN: skip the next instruction if VX does not equal VY
    if (chip8->V[inst.xyn.X] != chip8->V[inst.xyn.Y]) {
      chip8->PC += 2;
    }
    break;
  case 0xA:
    // 0xANNN: set I to the address NNN
    chip8->I = inst.nnn.NNN;
    break;
  case 0xB:
    if (config.use_BXNN) {
      // BXNN: jump to the address XNN plus VX (CHIP-48 and SUPER-CHIP)
      chip8->PC = inst.nnn.NNN + chip8->V[inst.xnn.X];
    } else {
      // BNNN: jump to the address NNN plus V0 (original behavior)
      chip8->PC = inst.nnn.NNN + chip8->V[0];
    }
    break;
  case 0xC:
    chip8->V[inst.xnn.X] = (rand() % 256) & inst.xnn.NN;
    break;
  case 0xD: { // 0xDXYN: draw a sprite
    uint16_t x, y;
    chip8->V[0xF] = 0;
    y = chip8->V[inst.xyn.Y] % config.window_height;

    // Loop over N rows of the sprite
    for (int i = 0; i < inst.xyn.N; i++) {
      x = chip8->V[inst.xyn.X] % config.window_width;
      uint8_t byte = chip8->ram[chip8->I + i];

      for (int j = 0; j < 8; j++) {
        const bool bit = byte & (1 << (7 - j));
        bool *pixel = &chip8->display[y * config.window_width + x];

        // If sprite bit and display pixel are on, set carry flag
        if (bit && *pixel) {
          chip8->V[0xF] = 1;
        }

        // XOR display pixel
        *pixel ^= bit;

        // Stop drawing at the right edge of the screen
        if (++x >= config.window_width)
          break;
      }
      // Stop drawing at the bottom of the screen
      if (++y >= config.window_height)
        break;
    }
    break;
  }
  case 0xE:
    switch (inst.xnn.NN) {
    case 0x9E:
      // 0xEX9E: skip instruction if key in VX is pressed
      if (chip8->keypad[chip8->V[inst.xnn.X]]) {
        chip8->PC += 2;
      }
      break;
    case 0xA1:
      // 0xEXA1: skip instruction if key in VX is not pressed
      if (!chip8->keypad[chip8->V[inst.xnn.X]]) {
        chip8->PC += 2;
      }
      break;
    default:
      break;
    }
    break;
  case 0xF:
    switch (inst.xnn.NN) {
    case 0x07:
      // 0xFX07: set VX to the value of the delay timer
      chip8->V[inst.xnn.X] = chip8->delay;
      break;
    case 0x0A: {
      // 0xFX0A: wait for key press then store it in VX
      bool key_pressed = false;
      for (uint8_t i = 0; i < sizeof chip8->keypad && !key_pressed; i++) {
        if (chip8->keypad[i]) {
          chip8->V[inst.xnn.X] = i;
          key_pressed = true;
        }
      }

      // If no key was pressed, run the same instruction again
      if (!key_pressed) {
        chip8->PC -= 2;
      }
      break;
    }
    case 0x15:
      // 0xFX15: set the delay timer to VX
      chip8->delay = chip8->V[inst.xnn.X];
      break;
    case 0x18:
      // 0xFX18: set the sound timer to VX
      chip8->sound = chip8->V[inst.xnn.X];
      break;
    case 0x1E:
      // 0xFX1E: add VX to I (carry flag VF is not affected)
      chip8->I += chip8->V[inst.xnn.X];
      break;
    case 0x29:
      // 0xFX29: set I to the location of the sprite for the character in VX
      chip8->I = chip8->V[inst.xnn.X] * 5 + 0x50;
      break;
    case 0x33: {
      // 0xFX33:
      uint8_t bcd = chip8->V[inst.xnn.X];
      chip8->ram[chip8->I + 2] = bcd % 10;
      bcd /= 10;
      chip8->ram[chip8->I + 1] = bcd % 10;
      bcd /= 10;
      chip8->ram[chip8->I] = bcd % 10;
      break;
    }
    case 0x55:
      // 0xFX55: store from V0 to VX (included) in memory, starting at address I
      for (uint8_t i = 0; i <= inst.xnn.X; i++) {
        chip8->ram[chip8->I + i] = chip8->V[i];
      }
      break;
    case 0x65:
      // 0xFX65: fill from V0 to VX (included) with values from memory, starting
      // at address I
      for (uint8_t i = 0; i <= inst.xnn.X; i++) {
        chip8->V[i] = chip8->ram[chip8->I + i];
      }
      break;
    default:
      break;
    }
    break;
  default:
    printf("Unimplemented\n");
    break; // Unimplemented or invalid opcode
  }
}
