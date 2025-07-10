#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCALE 10


typedef struct{

    uint8_t memory[4096];
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;
    uint8_t gfx[SCREEN_WIDTH * SCREEN_HEIGHT];
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint16_t stack[16];
    uint8_t sp;
    uint8_t key[16];
    uint16_t opcode;
    int draw_flag;

}       Chip8;

Chip8 chip8;


uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80   // F
};

void initialize() {
    chip8.pc = 0x200;
    chip8.opcode = 0;
    chip8.I = 0;
    chip8.sp = 0;

    for (int i = 0; i < 4096; i++)
        chip8.memory[i] = 0;
    for (int i = 0; i < 64 * 32; i++)
        chip8.gfx[i] = 0;
    for (int i = 0; i < 16; i++) {
        chip8.V[i] = 0;
        chip8.stack[i] = 0;
        chip8.key[i] = 0;
    }
    for (int i = 0; i < 80; ++i)
        chip8.memory[0x50 + i] = fontset[i];
    chip8.draw_flag = 0;
    chip8.delay_timer = 0;
    chip8.sound_timer = 0;
}

int loadRom(const char *filename) {
    printf("Trying to open: %s\n", filename);
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("Failed to open ROM");
        return 0;
    }
    fread(&chip8.memory[0x200], 1, 4096 - 0x200, f);
    fclose(f);
    return 1;
}

void emulateCycle() {
    chip8.opcode = chip8.memory[chip8.pc] << 8 | chip8.memory[chip8.pc + 1];
    printf("Opcode: %04X\n", chip8.opcode);

    chip8.pc += 2;

    switch (chip8.opcode & 0xF000) {
    case 0x0000:
        switch (chip8.opcode & 0x00FF) {
        case 0x00E0:
            memset(chip8.gfx, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
            chip8.draw_flag = 1;
            break;
        case 0x00EE:
            chip8.sp--;
            chip8.pc = chip8.stack[chip8.sp];
            break;
        default:
            break;
        }
        break;

    case 0x1000:
        chip8.pc = chip8.opcode & 0x0FFF;
        break;

    case 0x2000:
        chip8.stack[chip8.sp] = chip8.pc;
        chip8.sp++;
        chip8.pc = chip8.opcode & 0x0FFF;
        break;

    case 0x3000: {
        uint8_t x = (chip8.opcode & 0x0F00) >> 8;
        if (chip8.V[x] == (chip8.opcode & 0x00FF)) chip8.pc += 2;
        break;
    }

    case 0x4000: {
        uint8_t x = (chip8.opcode & 0x0F00) >> 8;
        if (chip8.V[x] != (chip8.opcode & 0x00FF)) chip8.pc += 2;
        break;
    }

    case 0x5000: {
        uint8_t x = (chip8.opcode & 0x0F00) >> 8;
        uint8_t y = (chip8.opcode & 0x00F0) >> 4;
        if (chip8.V[x] == chip8.V[y]) chip8.pc += 2;
        break;
    }

    case 0x6000: {
        uint8_t x = (chip8.opcode & 0x0F00) >> 8;
        chip8.V[x] = chip8.opcode & 0x00FF;
        break;
    }

    case 0x7000: {
        uint8_t x = (chip8.opcode & 0x0F00) >> 8;
        chip8.V[x] += chip8.opcode & 0x00FF;
        break;
    }

    case 0x8000: {
        uint8_t x = (chip8.opcode & 0x0F00) >> 8;
        uint8_t y = (chip8.opcode & 0x00F0) >> 4;
        switch (chip8.opcode & 0x000F) {
        case 0x0: chip8.V[x] = chip8.V[y]; break;
        case 0x1: chip8.V[x] |= chip8.V[y]; break;
        case 0x2: chip8.V[x] &= chip8.V[y]; break;
        case 0x3: chip8.V[x] ^= chip8.V[y]; break;
        case 0x4: {
            uint16_t sum = chip8.V[x] + chip8.V[y];
            chip8.V[0xF] = sum > 255;
            chip8.V[x] = sum & 0xFF;
            break;
        }
        case 0x5:
            chip8.V[0xF] = chip8.V[x] > chip8.V[y];
            chip8.V[x] -= chip8.V[y];
            break;
        case 0x6:
            chip8.V[0xF] = chip8.V[x] & 0x1;
            chip8.V[x] >>= 1;
            break;
        case 0x7:
            chip8.V[0xF] = chip8.V[y] > chip8.V[x];
            chip8.V[x] = chip8.V[y] - chip8.V[x];
            break;
        case 0xE:
            chip8.V[0xF] = (chip8.V[x] & 0x80) >> 7;
            chip8.V[x] <<= 1;
            break;
        default:
            printf("Unknown opcode: 0x%X\n", chip8.opcode);
        }
        break;
    }

    case 0x9000: {
        uint8_t x = (chip8.opcode & 0x0F00) >> 8;
        uint8_t y = (chip8.opcode & 0x00F0) >> 4;
        if (chip8.V[x] != chip8.V[y]) chip8.pc += 2;
        break;
    }

    case 0xA000:
        chip8.I = chip8.opcode & 0x0FFF;
        break;

    case 0xB000:
        chip8.pc = (chip8.opcode & 0x0FFF) + chip8.V[0];
        break;

    case 0xC000: {
        uint8_t x = (chip8.opcode & 0x0F00) >> 8;
        chip8.V[x] = (rand() % 256) & (chip8.opcode & 0x00FF);
        break;
    }

    case 0xD000: {
        uint8_t x = chip8.V[(chip8.opcode & 0x0F00) >> 8];
        uint8_t y = chip8.V[(chip8.opcode & 0x00F0) >> 4];
        uint8_t height = chip8.opcode & 0x000F;
        chip8.V[0xF] = 0;

        for (int row = 0; row < height; ++row) {
            uint8_t sprite = chip8.memory[chip8.I + row];

            for (int col = 0; col < 8; ++col) {
                uint8_t pixel = sprite & (0x80 >> col);
                if (pixel) {
                    int index = ((y + row) % SCREEN_HEIGHT) * SCREEN_WIDTH + ((x + col) % SCREEN_WIDTH);
                    if (chip8.gfx[index]) chip8.V[0xF] = 1;
                    chip8.gfx[index] ^= 1;
                }
            }
        }
        chip8.draw_flag = 1;
        break;
    }

    case 0xE000: {
        uint8_t x = (chip8.opcode & 0x0F00) >> 8;
        switch (chip8.opcode & 0x00FF) {
        case 0x9E:
            if (chip8.key[chip8.V[x]]) chip8.pc += 2;
            break;
        case 0xA1:
            if (!chip8.key[chip8.V[x]]) chip8.pc += 2;
            break;
        default:
            printf("Unknown opcode: 0x%X\n", chip8.opcode);
        }
        break;
    }

    case 0xF000: {
        uint8_t x = (chip8.opcode & 0x0F00) >> 8;
        switch (chip8.opcode & 0x00FF) {
        case 0x07: chip8.V[x] = chip8.delay_timer; break;
        case 0x0A: {
            int key_pressed = 0;
            for (int i = 0; i < 16; ++i) {
                if (chip8.key[i]) {
                    chip8.V[x] = i;
                    key_pressed = 1;
                    break;
                }
            }
            if (!key_pressed) chip8.pc -= 2;
            break;
        }
        case 0x15: chip8.delay_timer = chip8.V[x]; break;
        case 0x18: chip8.sound_timer = chip8.V[x]; break;
        case 0x1E: chip8.I += chip8.V[x]; break;
        case 0x29: {
            unsigned char x = (chip8.opcode & 0x0F00) >> 8;
            chip8.I = chip8.V[x] * 5 + 0x50;
            break;
        }
        case 0x33: {
            chip8.memory[chip8.I] = chip8.V[x] / 100;
            chip8.memory[chip8.I + 1] = (chip8.V[x] / 10) % 10;
            chip8.memory[chip8.I + 2] = chip8.V[x] % 10;
            break;
        }
        case 0x55:
            for (int i = 0; i <= x; ++i) chip8.memory[chip8.I + i] = chip8.V[i];
            break;
        case 0x65:
            for (int i = 0; i <= x; ++i) chip8.V[i] = chip8.memory[chip8.I + i];
            break;
        default:
            printf("Unknown opcode: 0x%X\n", chip8.opcode);
        }
        break;
    }

    default:
        printf("Unknown opcode: 0x%X\n", chip8.opcode);
    }
}



void drawScreen(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            if (chip8.gfx[y * SCREEN_WIDTH + x]) {
                SDL_FRect pixel = { x * SCALE, y * SCALE, SCALE, SCALE };
                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }
    SDL_RenderPresent(renderer);
}
void handleKeyDownSDL3(SDL_Scancode scancode) {
    switch (scancode) {
    case SDL_SCANCODE_1: chip8.key[0x1] = 1; break;
    case SDL_SCANCODE_2: chip8.key[0x2] = 1; break;
    case SDL_SCANCODE_3: chip8.key[0x3] = 1; break;
    case SDL_SCANCODE_4: chip8.key[0xC] = 1; break;

    case SDL_SCANCODE_Q: chip8.key[0x4] = 1; break;
    case SDL_SCANCODE_W: chip8.key[0x5] = 1; break;
    case SDL_SCANCODE_E: chip8.key[0x6] = 1; break;
    case SDL_SCANCODE_R: chip8.key[0xD] = 1; break;

    case SDL_SCANCODE_A: chip8.key[0x7] = 1; break;
    case SDL_SCANCODE_S: chip8.key[0x8] = 1; break;
    case SDL_SCANCODE_D: chip8.key[0x9] = 1; break;
    case SDL_SCANCODE_F: chip8.key[0xE] = 1; break;

    case SDL_SCANCODE_Z: chip8.key[0xA] = 1; break;
    case SDL_SCANCODE_X: chip8.key[0x0] = 1; break;
    case SDL_SCANCODE_C: chip8.key[0xB] = 1; break;
    case SDL_SCANCODE_V: chip8.key[0xF] = 1; break;
    }
}

void handleKeyUpSDL3(SDL_Scancode scancode) {
    switch (scancode) {
    case SDL_SCANCODE_1: chip8.key[0x1] = 0; break;
    case SDL_SCANCODE_2: chip8.key[0x2] = 0; break;
    case SDL_SCANCODE_3: chip8.key[0x3] = 0; break;
    case SDL_SCANCODE_4: chip8.key[0xC] = 0; break;

    case SDL_SCANCODE_Q: chip8.key[0x4] = 0; break;
    case SDL_SCANCODE_W: chip8.key[0x5] = 0; break;
    case SDL_SCANCODE_E: chip8.key[0x6] = 0; break;
    case SDL_SCANCODE_R: chip8.key[0xD] = 0; break;

    case SDL_SCANCODE_A: chip8.key[0x7] = 0; break;
    case SDL_SCANCODE_S: chip8.key[0x8] = 0; break;
    case SDL_SCANCODE_D: chip8.key[0x9] = 0; break;
    case SDL_SCANCODE_F: chip8.key[0xE] = 0; break;

    case SDL_SCANCODE_Z: chip8.key[0xA] = 0; break;
    case SDL_SCANCODE_X: chip8.key[0x0] = 0; break;
    case SDL_SCANCODE_C: chip8.key[0xB] = 0; break;
    case SDL_SCANCODE_V: chip8.key[0xF] = 0; break;
    }
}


int main(int argc, char* argv[]) {

    if (argc != 2) {
        printf("Usage: %s <ROM Path>\n", argv[0]);
        return 1;
    }

    initialize();

    loadRom(argv[1]);


    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }


    SDL_Window* window = SDL_CreateWindow("CHIP-8 Emulator", SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, SDL_WINDOW_OPENGL);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (window == NULL) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN) {
                handleKeyDownSDL3(event.key.scancode);
            }
            else if (event.type == SDL_EVENT_KEY_UP) {
                handleKeyUpSDL3(event.key.scancode);
            }
        }

        for (int i = 0; i < 10; i++) {
            emulateCycle();
        }

        Uint64 currentTicks = SDL_GetTicks();
        static Uint64 lastTimerTick = 0;

        if (currentTicks - lastTimerTick >= 16) {
            if (chip8.delay_timer > 0) chip8.delay_timer--;
            if (chip8.sound_timer > 0) chip8.sound_timer--;
            lastTimerTick = currentTicks;
        }

        if (chip8.draw_flag) {
            drawScreen(renderer);
            chip8.draw_flag = 0;
        }
        
        SDL_Delay(8);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
