#include "syscalls.h"
#include "cpu.h"

static void speaker_beep(unsigned int frequency)
{
	unsigned int count = 1193180 / frequency;
	
	// Switch on the speaker
	outb(inb(0x61) | 3, 0x61);
	
	// Set command for counter 2, 2 byte write
	outb(0xB6, 0x43);
	
	// Select desired Hz
	outb(count & 0xff, 0x42);
	outb((count >> 8) & 0xff, 0x42);
}

// Switch off the speaker
static void speaker_beep_off()
{
	outb(inb(0x61) & 0xFC, 0x61);
}

static volatile int speaker_LUT[128] = {
8, 8, 9, 9, 10, 10, 11, 12, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 23, 24,
25, 27, 29, 30, 32, 34, 36, 38, 41, 43, 46, 48, 51, 55, 58, 61, 65, 69, 73,
77, 82, 87, 92, 97, 103, 110, 116, 123, 130, 138, 146, 155, 164, 174, 184,
195, 207, 220, 233, 246, 261, 277, 293, 311, 329, 349, 369, 391, 415, 440,
466, 493, 523, 554, 587, 622, 659, 698, 739, 783, 830, 880, 932, 987, 1046,
1108, 1174, 1244, 1318, 1396, 1479, 1567, 1661, 1760, 1864, 1975, 2093, 2217,
2349, 2489, 2637, 2793, 2959, 3135, 3322, 3520, 3729, 3951, 4186, 4434, 4698,
4978, 5274, 5587, 5919, 6271, 6644, 7040, 7458, 7902, 8372, 8869, 9397, 9956,
10548, 11175, 11839, 12543,
};


static int speaker_midi_to_hz(unsigned char note_number)
{
	
	if(note_number>127) note_number = 60;
	
	return speaker_LUT[note_number];
}

void buzzer_beep(int note, int milliseconds)
{
    speaker_beep(speaker_midi_to_hz(note+48));
    usleep(1000*milliseconds);
    speaker_beep_off();
}

