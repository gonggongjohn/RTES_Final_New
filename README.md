.pio/libdeps/disco_f429zi/BSP_DISCO_F429ZI/Drivers/BSP/STM32F429I-Discovery/stm32f429i_discovery_sdram.c:177
.pio/libdeps/disco_f429zi/BSP_DISCO_F429ZI/Drivers/BSP/STM32F429I-Discovery/stm32f429i_discovery.c:897
删除wait_ms(Delay);改成thread_sleep_for(Delay);
删除wait_ms(1);改成thread_sleep_for(1);