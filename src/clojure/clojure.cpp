#include "../sdl/Drive.h"
#include "../gb/GB.h"
#include "../gb/gbMemory.h"
#include "../gb/gbCheats.h"
#include "../gb/gbGlobals.h"

// for VBAMovieOpen
#include "../common/movie.h"

#include <string.h>

#define UNUSED(x)  (void)(x)

extern int showScreen;
extern u16 currentButtons[4];

extern "C" {
    /*void say_hello(void) {
        printf("hello cruel world\n");
    }*/

    void step_until_f12(void) {
        stepUntilCapture();
    }

    void emu_step(int keymask) {
        step(keymask);
    }

    void emu_tick() {
        tick();
    }

    int get_current_buttons() {
        return currentButtons[0];
    }

    void set_showScreen(int status) {
        showScreen = status;
    }

    // dunno if this is correct or useful
    int get_rom_bank() {
        return gbDataMBC3.mapperROMBank;
    }

    void write_memory_at(u16 address, u8 value) {
        // borked? gbWriteMemory(address, value);
        gbWriteMemoryQuick(address, value);
    }

    int read_memory_at(u16 address) {
        return (int) gbReadMemoryQuick(address);
    }

    // this is probably a memory leak
    /*
    char * buggy_get_state() {
        char * buffer = new char[MAX_SAVE_SIZE];
        gbWriteMemSaveStatePos(buffer, MAX_SAVE_SIZE);
        return buffer;
    }*/

    void get_state(char * buffer, int size) {
        gbWriteMemSaveStatePos(buffer, size);
    }

    void set_state(char * buffer, int size) {
        gbReadMemSaveState(buffer, size);
    }

    // 65536 bytes (0x10000)
    void get_memory(int32 * memory) {
        storeMemory(memory);
    }

    void set_memory(int32 * new_memory) {
        writeMemory(new_memory);
    }

    // 29 bytes
    void get_registers(int32 * registers) {
        storeRegisters(registers);
    }

    void set_registers(int32 * new_registers) {
        setRegisters(new_registers);
    }

    int get_ram_size() {
        return getRamSize();
    }

    int get_rom_size() {
        return getRomSize();
    }

    // TODO: why doesn't this work with ctypes.c_int.in_dll(self._vba, "MAX_SAVE_SIZE").value ?
    const int MAX_SAVE_SIZE = 20000;

    int get_max_save_size() {
        return MAX_SAVE_SIZE;
    }

    void get_ram(int32 * ram) {
        storeRam(ram);
    }

    // size: 0x8000
    void get_wram(int32 * wram) {
        storeWRam(wram);
    }

    // size: 0x4000
    void get_vram(int32 * vram) {
        storeVRam(vram);
    }

    void get_rom(int32 * rom) {
        storeRom(rom);
    }

    void set_rom(int32 * new_rom) {
        writeRom(new_rom);
    }

    void save_png(const char *path) {
        gbWritePNGFile(path);
    }

    int get_cheat_count() {
        return gbCheatNumber;
    }

    bool cheat_read_gameshark_file(const char *path) {
        return gbCheatReadGSCodeFile(path);
    }

    void cheat_add_gameshark(const char *code, const char *description) {
        gbAddGsCheat(code, description);
    }

    void cheat_add_gamegenie(const char *code, const char *description) {
        gbAddGgCheat(code, description);
    }

    void cheat_enable(int id) {
        gbCheatEnable(id);
    }

    void cheat_disable(int id) {
        gbCheatDisable(id);
    }

    void cheat_remove(int id) {
        gbCheatRemove(id);
    }

    void cheat_remove_all(void) {
        gbCheatRemoveAll();
    }

    // TODO: Java_com_aurellem_gb_Gb_VBAMovieOpen
    // TODO: Java_com_aurellem_gb_Gb_getPixels
}
