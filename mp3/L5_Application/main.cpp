#include "FreeRTOS.h"
#include "task.h"
// #include <stdio.h>
#include "printf_lib.h"

#include "Flash.h"

Flash obj;

void vprintMp3Files(void * pvParam) {
	obj.get_mp3_meta_data();
	while (1) {
		obj.print_meta_data();
		vTaskDelay(500);
	}
}

int main(int argc, char const *argv[]){
	xTaskCreate(vprintMp3Files, "printMp3Files", 1024, (void*) 0, 1, NULL);

	vTaskStartScheduler();

	while(1);

	return 0;
}