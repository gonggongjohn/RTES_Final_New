#include "mbed.h"
#include "stm32f4xx_hal.h"
#include "LCD_DISCO_F429ZI.h"

//Used lecture 5 demo on SPI as the starting point

SPI spi(PF_9, PF_8, PF_7); // mosi, miso, sclk
DigitalOut cs(PC_1);

LCD_DISCO_F429ZI lcd; // On Display LCD is initialized

//Arrays to store the raw gyroscope values of X Y and Z
double arr_x[40];
double arr_y[40];
double arr_z[40];

double dist_x, dist_y, dist_z;

// Wait for the specified number of seconds
void wait_s(int seconds)
{
  for (int i = 0; i < seconds; i++)
  {
    for (int j = 0; j < 1000; j++)
    {
      wait_us(1000);
    }
  }
}

void initializeSPI() {
    cs = 1;
    spi.format(8, 3); // 8 bit data, high steady state clock, second edge capture
    spi.frequency(1000000); // 1MHz clock rate
}

int16_t readGyroRegister(uint8_t reg) {
    cs = 1;
    cs = 0;
    spi.write(0x80 | reg); // 0x80 | reg: set MSB for read operation
    int16_t value = spi.write(0x00); // Dummy write to read data
    return value;
}

void readGyroData(int samples) {
    for (int i = 0; i < samples; i++) {
        // Setup the control register to enable the gyroscope
        cs = 0;
        spi.write(0x20); // Control register address
        spi.write(0x0f); // Control register value (Xen, Yen, Zen)
        cs = 1;
        // Read high and low registers for each axis
        int16_t xl = readGyroRegister(0x28);
        int16_t xh = readGyroRegister(0x29);
        int16_t yl = readGyroRegister(0x2A);
        int16_t yh = readGyroRegister(0x2B);
        int16_t zl = readGyroRegister(0x2C);
        int16_t zh = readGyroRegister(0x2D);

        int16_t x_full = (xh << 8) | xl;
        int16_t y_full = (yh << 8) | yl;
        int16_t z_full = (zh << 8) | zl;

        double x_raw = (double) x_full;
        double y_raw = (double) y_full;
        double z_raw = (double) z_full;

        // Concatenate high and low values and map them
        arr_x[i] = (x_raw / (1<<15)) * 245;
        arr_y[i] = (y_raw / (1<<15)) * 245;
        arr_z[i] = (z_raw / (1<<15)) * 245;

        // Print the angular velocity
        printf("x = %lf y = %lf z = %lf\n", arr_x[i], arr_y[i], arr_z[i]);

        // Wait for 500 milliseconds
        wait_us(500000);
        printf("\n%d\n", i); //loop variable
    }
}

void displayCountdown() {
    for (int countdown = 5; countdown > 0; countdown--) {
        char countdownText[20];
        sprintf(countdownText, "Starting in %d...", countdown);
        lcd.DisplayStringAt(0, LINE(7), (uint8_t *)countdownText, CENTER_MODE);
        wait_s(1);
    }
}

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

    int radius = 40;
    double x_lin = avg_x * radius;
    double y_lin = avg_y * radius;
    double z_lin = avg_z * radius;
    printf("Linear Velocity is %lf\n\n\n", z_lin);

    int dur = 20;
    dist_x = x_lin * dur;
    dist_y = y_lin * dur;
    dist_z = z_lin * dur;
}


void displayResults(){
      uint8_t xdist[30], ydist[30], zdist[30];
      sprintf((char *)xdist, "Dist X = %.2f", dist_x);
      sprintf((char *)ydist, "Dist Y = %.2f", dist_y);
      sprintf((char *)zdist, "Distance = %.2f cm", abs(dist_z));

      lcd.Clear(LCD_COLOR_WHITE);
      lcd.DisplayStringAt(0, LINE(7), (uint8_t *)zdist, CENTER_MODE);
      printf("Distance is %lf\n\n\n", abs(dist_z));
}


int main() {
    initializeSPI();

    while (true) {
        lcd.DisplayStringAt(0, LINE(10), (uint8_t *)"Prepare to walk", CENTER_MODE);
        displayCountdown();

        lcd.Clear(LCD_COLOR_WHITE);
        lcd.DisplayStringAt(0, LINE(10), (uint8_t *)"Walk now", CENTER_MODE);

        // '40' is the number of samples you want to read
        readGyroData(40);

        lcd.Clear(LCD_COLOR_WHITE);
        lcd.DisplayStringAt(0, LINE(10), (uint8_t *)"Stop now", CENTER_MODE);
        wait_s(3);
        printf("DONE");

        calculateDistance();
        displayResults();

        wait_s(10); // Wait for 10 seconds until the loop restarts
    }
}



// int main()
// {
//   // Deselect the device
//   cs = 1;

//   // Setup the spi for 8 bit data, high steady state clock,
//   // second edge capture, with a 1MHz clock rate
//   spi.format(8, 3);
//   spi.frequency(1000000);

//   while (true)
//   {
//     // Orange screen to indicate that the loop is about to begin
//     //lcd.Clear(LCD_COLOR_YELLOW);
//     lcd.DisplayStringAt(0, LINE(10), (uint8_t *)"Prepare to walk", CENTER_MODE);

//     for (int countdown = 5; countdown > 0; countdown--) {
//     char countdownText[20];
//     sprintf(countdownText, "Starting in %d...", countdown);
//     lcd.DisplayStringAt(0, LINE(7), (uint8_t *)countdownText, CENTER_MODE);
//     wait_s(1);
//   }

//     // Green to indicate that the loop is running
//     lcd.Clear(LCD_COLOR_WHITE);
//     lcd.DisplayStringAt(0, LINE(10), (uint8_t *)"Walk now", CENTER_MODE);

//     // Record gyroscope data 40 times
//     readGyroData(40);

//     // Display red to indicate the loop is stopped
//     lcd.Clear(LCD_COLOR_WHITE);
//     lcd.DisplayStringAt(0, LINE(10), (uint8_t *)"Stop now", CENTER_MODE);


//     wait_s(3);
//     printf("DONE");

//     // Calculate the average angular velocities
//     double avg_x = 0;
//     double avg_y = 0;
//     double avg_z = 0;
//     for (int i = 0; i < 40; i++)
//     {
//       avg_x += arr_x[i] / 40;
//       avg_y += arr_y[i] / 40;
//       avg_z += arr_y[i] / 40;
//     }

//     // printf("\nAngular X = %lf dps\n\n", avg_x);
//     // printf("\nAngular Y = %lf dps\n\n", avg_y);
//     printf("\nAngular Z = %lf dps\n", avg_z);

//     // Multiply with radius to convert from angular to linear velocity
//     int radius = 40;
//     double x_lin = avg_x * radius;
//     double y_lin = avg_y * radius;
//     double z_lin = avg_z * radius;
//     printf("Linear Velocity is %lf\n\n\n", z_lin);


//     // Multiply the linear velocity with the duration of measurement
//     int dur = 20;
//     double dist_x = x_lin * dur;
//     double dist_y = y_lin * dur;
//     double dist_z = z_lin * dur;

//     lcd.Clear(LCD_COLOR_WHITE);

//     // Arrays to concatenate the distance
//     //Reference from MBED's demo on LCD
//     //https://os.mbed.com/teams/ST/code/DISCO-F429ZI_LCDTS_demo//file/4faee567c996/main.cpp/
//     uint8_t xdist[30];
//     uint8_t ydist[30];
//     uint8_t zdist[30];

//     sprintf((char *)xdist, "Dist X = %.2f", dist_x);
//     sprintf((char *)ydist, "Dist Y = %.2f", dist_y);
//     sprintf((char *)zdist, "Distance = %.2f cm", dist_z);

//     // lcd.DisplayStringAt(0, LINE(5), (uint8_t *) xdist, CENTER_MODE);
//     // lcd.DisplayStringAt(0, LINE(6), (uint8_t *) ydist, CENTER_MODE);

//     // Display the distance in Z direction
//     lcd.DisplayStringAt(0, LINE(7), (uint8_t *)zdist, CENTER_MODE);

//     printf("Distance is %lf\n\n\n", abs(dist_z));

//     // Wait for 10 seconds until the loop restarts
//     wait_s(10);
//   }
// }