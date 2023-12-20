#include "mbed.h"
#include "stm32f4xx_hal.h"
#include "LCD_DISCO_F429ZI.h"

// SPI interface initialization with pins for MOSI, MISO, and SCLK
SPI spi(PF_9, PF_8, PF_7);
// Chip select pin for SPI
DigitalOut cs(PC_1);

LCD_DISCO_F429ZI lcd;

// Arrays to store gyroscope data for each axis
double arr_x[40];
double arr_y[40];
double arr_z[40];

// Variables to store calculated distance for each axis
double dist_x, dist_y, dist_z;

// Wait function wrapper (second as unit)
void wait_s(int seconds) {
    for (int i = 0; i < seconds; i++) {
        wait_us(1000000);
    }
}

// Function to initialize the SPI interface
void initializeSPI() {
    cs = 1; // Deselect the SPI device
    spi.format(8, 3); // Set up SPI format
    spi.frequency(1000000); // 1 MHz SPI clock frequency
}

// Read data from a given gyroscope register
int16_t readGyroRegister(uint8_t reg) {
    cs = 0; // Select the device
    spi.write(0x80 | reg); // SPI Read
    int16_t value = spi.write(0x00); //Dummy write to read value
    cs = 1; // Deselect the device
    return value;
}

// Read gyroscope data
void readGyroData(int samples) {
    for (int i = 0; i < samples; i++) {
        cs = 0;
        spi.write(0x20); // Write to control register address
        spi.write(0x0F); // Set control register value for enabling axes
        cs = 1;

        // Read from gyroscope registers for each axis
        int16_t xl = readGyroRegister(0x28); // X-axis low byte
        int16_t xh = readGyroRegister(0x29); // X-axis high byte
        int16_t yl = readGyroRegister(0x2A); // Y-axis low byte
        int16_t yh = readGyroRegister(0x2B); // Y-axis high byte
        int16_t zl = readGyroRegister(0x2C); // Z-axis low byte
        int16_t zh = readGyroRegister(0x2D); // Z-axis high byte

        // Combine high and low bytes
        int16_t x_full = (xh << 8) | xl;
        int16_t y_full = (yh << 8) | yl;
        int16_t z_full = (zh << 8) | zl;
        
        double x_raw = (double) x_full;
        double y_raw = (double) y_full;
        double z_raw = (double) z_full;

        // Scale raw data to -245 ~ 245
        arr_x[i] = x_raw / (1<<15) * 245;
        arr_y[i] = y_raw / (1<<15) * 245;
        arr_z[i] = z_raw / (1<<15) * 245;

        // Print angular velocities for each sample
        printf("Sample %d: x = %lf y = %lf z = %lf\n", i, arr_x[i], arr_y[i], arr_z[i]);

        wait_us(500000); // Wait for 500ms between samples
    }
}

// Display a countdown on the LCD before starting the measurements
void displayCountdown() {
    for (int countdown = 5; countdown > 0; countdown--) {
        char countdownText[20];
        sprintf(countdownText, "Starting in %d...", countdown);
        lcd.DisplayStringAt(0, LINE(7), (uint8_t *)countdownText, CENTER_MODE);
        wait_s(1);
    }
}

// Calculate the distance moved based on gyroscope data
void calculateDistance() {
    double avg_x = 0, avg_y = 0, avg_z = 0;
    for (int i = 0; i < 40; i++) {
        avg_x += arr_x[i] / 40;
        avg_y += arr_y[i] / 40;
        avg_z += arr_z[i] / 40;
    }

    printf("Angular X = %lf dps\n", avg_x);
    printf("Angular Y = %lf dps\n", avg_y);
    printf("Angular Z = %lf dps\n", avg_z);

    // Convert angular velocity to linear velocity
    int radius = 40; // Radius of leg
    double x_lin = avg_x * radius;
    double y_lin = avg_y * radius;
    double z_lin = avg_z * radius;

    printf("Linear Velocity X = %lf, Y = %lf, Z = %lf\n", x_lin, y_lin, z_lin);

    // Calculate the distance
    int duration = 20; // Duration of measurement in seconds
    dist_x = x_lin * duration;
    dist_y = y_lin * duration;
    dist_z = z_lin * duration;
}

// Display the results
void displayResults() {
    char xdist[30], ydist[30], zdist[30];
    
    sprintf(xdist, "Dist X = %.2f", dist_x);
    sprintf(ydist, "Dist Y = %.2f", dist_y);
    sprintf(zdist, "Distance Z = %.2f cm", abs(dist_z));

    // Clear LCD and display Z distance
    lcd.Clear(LCD_COLOR_WHITE);
    lcd.DisplayStringAt(0, LINE(7), (uint8_t *)zdist, CENTER_MODE);
    // Log Z distance
    printf("Distance Z = %lf cm\n", abs(dist_z));
}

int main() {
    initializeSPI();

    while (true) {
        // Display instructions on the LCD screen
        lcd.DisplayStringAt(0, LINE(10), (uint8_t *)"Prepare to walk", CENTER_MODE);
        displayCountdown();

        lcd.Clear(LCD_COLOR_WHITE);
        lcd.DisplayStringAt(0, LINE(10), (uint8_t *)"Walk now", CENTER_MODE);

        // Collect gyroscope data
        readGyroData(40);

        lcd.Clear(LCD_COLOR_WHITE);
        lcd.DisplayStringAt(0, LINE(10), (uint8_t *)"Stop now", CENTER_MODE);
        wait_s(3);
        printf("DONE\n");

        // Process and display the results
        calculateDistance();
        displayResults();
        
        wait_s(10); // Wait 10 seconds before starting the next measurement cycle
    }
}
