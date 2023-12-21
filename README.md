If you are using MacOS, please do the following steps after building the environment:

1. Go to `.pio/libdeps/disco_f429zi/BSP_DISCO_F429ZI/Drivers/BSP/STM32F429I-Discovery/stm32f429i_discovery_sdram.c`, comment `wait_ms(1);` and uncomment `HAL_Delay(1);` at line 177;
2. Go to `.pio/libdeps/disco_f429zi/BSP_DISCO_F429ZI/Drivers/BSP/STM32F429I-Discovery/stm32f429i_discovery.c`, comment `wait_ms(Delay);` and uncomment `thread_sleep_for(Delay);` at line 897.
