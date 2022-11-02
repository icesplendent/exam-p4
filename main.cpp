#include "mbed.h"

Thread thread_master;
Thread thread_slave;

EventQueue queue_note;

// master
SPI spi(D11, D12, D13);  // mosi, miso, sclk
DigitalOut cs(D9);

// slave
SPISlave device(PD_4, PD_3, PD_1, PD_0);  // mosi, miso, sclk, cs; PMOD pins

DigitalOut led(LED3);

int note = 0x01;
int length = 0x01;

int slave() {
    device.format(8, 3);
    device.frequency(1000000);
    int8_t note = 0, length = 0, v = 0;
    static int8_t note_played = 0;
    device.reply(note);  // Prime SPI with first reply
    while (1) {
        if (device.receive()) {
            note = device.read();    // Read note from master
            length = device.read();  // Read length from master

            note_played++;
            // printf("Slave: Number of note played = %d\n", note_played);
            device.reply(note_played);  // Make this the next reply
            v = device.read();          // Send to master

            //   queue_note.call(playNote, note, length);
        }
    }
}

void master() {
    // Setup the spi for 8 bit data, high steady state clock,
    // second edge capture, with a 1MHz clock rate
    spi.format(8, 3);
    spi.frequency(1000000);

    for (int i = 0; i < 5; ++i, note++) {  // Run for 5 times
        // Chip must be deselected
        cs = 1;
        // Select the device by seting chip select low
        cs = 0;

        printf("New chapter starts from here.\n");

        int response = spi.write(note);  // Send type of mode
        response = spi.write(length);    // Send type of length
        cs = 1;                          // Deselect the device
        ThisThread::sleep_for(100ms);    // Wait for debug print
        printf("First response from slave = %d\n", response);

        // Select the device by seting chip select low
        cs = 0;

        response = spi.write(note);    // Read slave reply
        ThisThread::sleep_for(100ms);  // Wait for debug print
        printf("The note received from slave = %d\n", response);
        response = spi.write(0x88);    // extra data to send due to requirement
        ThisThread::sleep_for(100ms);  // Wait for debug print
        response = spi.write(note);    // Read the answer from slave
        ThisThread::sleep_for(100ms);  // Wait for debug print
        printf("Answer from slave = %d\n", response);
        cs = 1;  // Deselect the device
    }
}

int main() {
    thread_slave.start(slave);
    thread_master.start(master);
}