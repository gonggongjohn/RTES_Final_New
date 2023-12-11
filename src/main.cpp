#include "mbed.h"
#include "stm32f4xx_hal.h"
#include "LCD_DISCO_F429ZI.h"
#include "stm32f429i_discovery_lcd.h"

//Used lecture 5 demo on SPI as the starting point

SPI spi(PF_9, PF_8, PF_7); // mosi, miso, sclk
DigitalOut cs(PC_1);

LCD_DISCO_F429ZI lcd; // On Display LCD is initialized

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

int main()
{
  // Deselect the device
  cs = 1;

  // Setup the spi for 8 bit data, high steady state clock,
  // second edge capture, with a 1MHz clock rate
  spi.format(8, 3);
  spi.frequency(1000000);

  //Arrays to store the raw gyroscope values of X Y and Z
  double arr_x[40];
  double arr_y[40];
  double arr_z[40];

  while (true)
  {
    // Orange screen to indicate that the loop is about to begin
    lcd.Clear(LCD_COLOR_ORANGE);
    lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"GET READY", CENTER_MODE);

    wait_s(5);

    // Green to indicate that the loop is running
    lcd.Clear(LCD_COLOR_GREEN);
    lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"WALK", CENTER_MODE);

    int i = 0;

    // loop to sample the data at the interval of 0.5 seconds
    while (i < 40)
    {
      i++;

      // The command to setup the control register 1 to enable the gyroscope, Xen, Yen and Zen (0b00001111)
      cs = 0;

      spi.write(0x20);
      spi.write(0x0f);

      // The command to read the control register
      // cs = 1;
      // cs = 0;
      // spi.write(0xA0);
      // int val = spi.write(0x00);
      // printf("cntrl1 register = %d\n", val);

      // Read the contents from XL register (address 0xA8)
      cs = 1;
      cs = 0;
      spi.write(0xA8);
      // Send a dummy byte to receive the contents of the xl register
      // The value is in 2's complement form, so signed int is used
      int16_t xl = spi.write(0x00);

      // Read the contents from XH register (address 0xA8)
      cs = 1;
      cs = 0;
      spi.write(0xA9);
      // Send a dummy byte to receive the contents of the xh register
      int16_t xh = spi.write(0x00);

      // Similarly read from YL, YH, ZL and ZH
      cs = 1;
      cs = 0;
      spi.write(0xAA);
      // Send a dummy byte to receive the contents of the yl register
      int16_t yl = spi.write(0x00);

      cs = 1;
      cs = 0;
      spi.write(0xAB);
      // Receive the contents of the yh register
      int16_t yh = spi.write(0x00);

      cs = 1;
      cs = 0;
      spi.write(0xAC);
      // Receive the contents of the zl register
      int16_t zl = spi.write(0x00);

      cs = 1;
      cs = 0;
      spi.write(0xAD);
      // Receive the contents of the zh register
      int16_t zh = spi.write(0x00);

      // Concatenate the high and low registers
      int16_t xr = ((xh << 8) | xl);
      int16_t yr = ((yh << 8) | yl);
      int16_t zr = ((zh << 8) | zl);

      // Cast the signed 16bit integer into double
      double xraw = (double)xr;
      double yraw = (double)yr;
      double zraw = (double)zr;

      // Map the values from (-2^15, 2^15) to (-245, 245)
      double x = (xraw / 32768) * 245;
      double y = (yraw / 32768) * 245;
      double z = (zraw / 32768) * 245;

      // Store it into the array
      arr_x[i] = x;
      arr_y[i] = y;
      arr_z[i] = z;

      //Print the angular velocity
      printf("x = %lf\n", x);
      printf("y = %lf\n", y);
      printf("z  = %lf\n\n", z);

      // Deselect the device
      cs = 1;

      // wait for 500milliseconds
      wait_us(500000);
      printf("\n%d\n", i); //loop variable
    }

    // Display red to indicate the loop is stopped
    lcd.Clear(LCD_COLOR_RED);
    lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"STOP", CENTER_MODE);

    wait_s(3);
    printf("DONE");

    // Calculate the average angular velocities
    double avg_x = 0;
    double avg_y = 0;
    double avg_z = 0;
    for (int i = 0; i < 40; i++)
    {
      avg_x += arr_x[i] / 40;
      avg_y += arr_y[i] / 40;
      avg_z += arr_y[i] / 40;
    }

    // printf("\nAngular X = %lf dps\n\n", avg_x);
    // printf("\nAngular Y = %lf dps\n\n", avg_y);
    printf("\nAngular Z = %lf dps\n", avg_z);

    // Multiply with radius to convert from angular to linear velocity
    int radius = 40;
    double x_lin = avg_x * radius;
    double y_lin = avg_y * radius;
    double z_lin = avg_z * radius;
    printf("Linear Velocity is %lf\n\n\n", z_lin);


    // Multiply the linear velocity with the duration of measurement
    int dur = 20;
    double dist_x = x_lin * dur;
    double dist_y = y_lin * dur;
    double dist_z = z_lin * dur;

    lcd.Clear(LCD_COLOR_WHITE);

    // Arrays to concatenate the distance
    //Reference from MBED's demo on LCD
    //https://os.mbed.com/teams/ST/code/DISCO-F429ZI_LCDTS_demo//file/4faee567c996/main.cpp/
    uint8_t xdist[30];
    uint8_t ydist[30];
    uint8_t zdist[30];

    sprintf((char *)xdist, "Dist X = %.2f", dist_x);
    sprintf((char *)ydist, "Dist Y = %.2f", dist_y);
    sprintf((char *)zdist, "Distance = %.2f cm", dist_z);

    // lcd.DisplayStringAt(0, LINE(5), (uint8_t *) xdist, CENTER_MODE);
    // lcd.DisplayStringAt(0, LINE(6), (uint8_t *) ydist, CENTER_MODE);

    // Display the distance in Z direction
    lcd.DisplayStringAt(0, LINE(7), (uint8_t *)zdist, CENTER_MODE);

    printf("Distance is %lf\n\n\n", dist_z);

    // Wait for 10 seconds until the loop restarts
    wait_s(10);
  }
}