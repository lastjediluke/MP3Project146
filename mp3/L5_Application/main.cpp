#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include "printf_lib.h"
#include "str.hpp"

#include "Flash.h"
#include "storage.hpp"

Flash obj;

void vprintMp3Files(void * pvParam) {
	obj.get_mp3_files();
	obj.get_mp3_metadata();

	while (1) {
		vTaskDelay(50);
	}
}

int main(int argc, char const *argv[]){
	xTaskCreate(vprintMp3Files, "printMp3Files", 4096, (void*) 0, 1, NULL);

	vTaskStartScheduler();

	while(1);

	return 0;
}