#include "driver_init.h"
#include "utils.h"
#include <hal_gpio.h>
#include <hal_delay.h>
#include <stdio.h>

int MAX_RECORD_COUNT = 2;

const int ARRAY_MAX = 10;

#define DEVICE_ADDRESS 0x68
#define WHO_AM_I 0x75
#define ACCEL_X_OUT 0x6b
 
#define SW0 GPIO(GPIO_PORTA, 15) // board switch, pin
#define LED0 GPIO(GPIO_PORTB, 30) // board led, pin

/* 
Physical Connections 
MPU6050 | Micro-controller (SAMD21)
SDA -	PA08
SCL -	PA09
VCC - 	VCC
GND - 	GND, AD0

Note:- HAL Library Modifications
Function - int32_t i2c_m_sync_cmd_write had to be modified
to match the sequence with the MPU6050 single byte write
File - hal/src/hal_i2c_m_sync.c

New Files Added -
GestureClassifier.c
GestureClassifier.h

Return values of successful operations oni2c_m_sync_cmd_read, i2c_m_sync_cmd_write is,
 Successful - 0
 Not Completed - <0
*/

// states to store the directions +-(x,y,z)
enum directions { X_POS = 1, X_NEG = -1, Y_POS = 2, Y_NEG = -2 , Z_POS = 3, Z_NEG = -3 };

// util routine to push into array dynamically
void push(int *arr, int index, int value, int *size, int *capacity){
     if(*size > *capacity){
          realloc(arr, sizeof(arr) * 2);
          *capacity = sizeof(arr) * 2;
     }
     arr[index] = value;
     *size = *size + 1;
}

// Function to record the gesture and the output as the sequence of directions
int recordGesture(int max, int *directions) {
	
	// record i values from the sensor
	int  i = 0;

	// counter to extract all the 6 register values
	int counter = 0;

	// counter to add directions into the sequence (directions)
	int direction_counter = 0;
	
	// Store values of the registers acX, acY, acZin in t-1 state 
	int ac_t_minus_1[2][3] = {{0,0,0},{0,0,0}};

	int size = 0;
    int capacity = ARRAY_MAX;
	while(i<max) {
		// read after a small delay
		delay_ms(10);
		static uint8_t buffer[6];
		
		// store the last value in the second row
		ac_t_minus_1[1][0] = ac_t_minus_1[0][0];
		ac_t_minus_1[1][1] = ac_t_minus_1[0][1];
		ac_t_minus_1[1][2] = ac_t_minus_1[0][2];
		
		// read the values from the sensor
		int32_t read_ack = i2c_m_sync_cmd_read(&I2C_0, 0x3b, buffer, 6);
		int16_t acX = buffer[counter++] << 8 | buffer[counter++]; // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
		
		int16_t acY = buffer[counter++] << 8 | buffer[counter++]; // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
		
		int16_t acZ = buffer[counter++] << 8 | buffer[counter++]; // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
		
		// update the first row of values as the latest values
		ac_t_minus_1[0][0] = acX;
		ac_t_minus_1[0][1] = acY;
		ac_t_minus_1[0][2] = acZ;
		
		// detect a direction change in X and save in the sequence
		if(differenceInDirection(ac_t_minus_1[0][0], ac_t_minus_1[1][0]) && ac_t_minus_1[1][0] != 0) {
			printf("direction change in x\n");
			if (ac_t_minus_1[0][0] > ac_t_minus_1[1][0]) {
				push(directions,direction_counter, X_POS, &size, &capacity);
				direction_counter += 1;
			}
			else {
				push(directions,direction_counter, X_NEG, &size, &capacity);
				direction_counter += 1;
			}
		}
		// detect a direction change in Y and save in the sequence
		if(differenceInDirection(ac_t_minus_1[0][1], ac_t_minus_1[1][1]) && ac_t_minus_1[1][1] != 0) {
			printf("direction change in y\n");
			if (ac_t_minus_1[0][1] > ac_t_minus_1[1][1]) {
				push(directions,direction_counter, Y_POS, &size, &capacity);
				direction_counter += 1;
			}
			else {;
				push(directions,direction_counter, Y_NEG, &size, &capacity);
				direction_counter += 1;
			}
		}
		// detect a direction change in z and save in the sequence
		if(differenceInDirection(ac_t_minus_1[0][2], ac_t_minus_1[1][2]) && ac_t_minus_1[1][2] != 0) {
			printf("direction change in z\n");
			if (ac_t_minus_1[0][2] > ac_t_minus_1[1][2]) {
				push(directions,direction_counter, Z_POS, &size, &capacity);
				direction_counter += 1;
			}
			else {
				push(directions,direction_counter, Z_NEG, &size, &capacity);
				direction_counter += 1;
			}
		}
		
		
		printf("X: %d        Y: %d        Z: %d\n",acX,acY,acZ);
		
		i++;
		 
		counter=0;
	}
	
	return directions;
}

// routine to carefully toggle the LED on the button press;
void toggleLED(){
	do {
		delay_ms(100);
	} while (gpio_get_pin_level(SW0));

	gpio_toggle_pin_level(LED0);
	
	printf("Button pressed\n");
	do {
		delay_ms(100);
	} while (!gpio_get_pin_level(SW0));
}

/* External button press interrupt handler */
static void button_pressed(void){
	// switch on gpio led pin for indicating a record
	toggleLED();

	// initialize two arrays for storing the gesture directions and test directions
	int* gesture_one_directions = malloc(ARRAY_MAX * sizeof(int));
	int* gesture_two_directions = malloc(ARRAY_MAX * sizeof(int));
	
	int* test_directions = malloc(ARRAY_MAX * sizeof(int));
	
	/* Record once again and Toggle LED if gesture is classified */
	delay_ms(200);
	if(MAX_RECORD_COUNT==0){
		printf("\nRecording Final Gesture\n");
		
		recordGesture(50, test_directions);
		
		delay_ms(2000);
		
		for (int i =0;i<ARRAY_MAX;i++)
		{
			printf("%d ", test_directions[i]);
		}
		printf("\n");
		
		// check if it was classified
		bool classified = isGesture(gesture_one_directions, gesture_two_directions, test_directions);

		int i = 0;
		printf("\n Return whether the gesture was true(1)- %d\n", classified);
		if(classified){
			// toggle 10 times to show classified
			printf("Successfully classified\n");
			while(i<10) {
				gpio_toggle_pin_level(LED0);
				delay_ms(250);
				i++;
			}
		}
		else {
			printf("Gesture not recognized\n");
			// toggle indication to show misclassification
			while(i<3){
				gpio_toggle_pin_level(LED0);
				delay_ms(250);
				i++;
			}
		}
	}
	
	// record only on press/HIGH/LED is on
	if(!gpio_get_pin_level(LED0) && MAX_RECORD_COUNT>0) {
		if(MAX_RECORD_COUNT==1){
			
			printf("\nRecording Gesture 2\n");
			recordGesture(50, gesture_two_directions);
			delay_ms(1000);
			for (int i=0;i<ARRAY_MAX;i++)
			{
				printf("%d ", gesture_two_directions[i]);
			}
			printf("\n");
		}
		else {
			
			printf("\nRecording Gesture 1\n");
			recordGesture(50, gesture_one_directions);
			delay_ms(1000);
			
			for (int i=0;i<ARRAY_MAX;i++)
			{
				printf("%d ", gesture_one_directions[i]);
			}
			printf("\n");
		}
		
		// turn off LED after recording
		gpio_toggle_pin_level(LED0);
		delay_ms(250);
		MAX_RECORD_COUNT -= 1;
		
	}
} 

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	/* Register the interrupt */
	ext_irq_register(PA15, button_pressed);
	
	struct io_descriptor *I2C_0_io;
	i2c_m_sync_get_io_descriptor(&I2C_0, &I2C_0_io);
	
	// sync enable for i2c
	int32_t i2c_ack = i2c_m_sync_enable(&I2C_0);
	printf("I2C Enabled - %d\n", i2c_ack);
	
	// setting device address as slave address
	int32_t slave_addr = i2c_m_sync_set_slaveaddr(&I2C_0, DEVICE_ADDRESS, I2C_M_SEVEN);
	
	// gives 104 in decimal
	printf("Masked Slave Address - %d\n", slave_addr);
	
	// test with WHO_AM_I register
	uint8_t readBuffer;
	int32_t readData = i2c_m_sync_cmd_read(&I2C_0, WHO_AM_I, &readBuffer, 1);
	
	printf("WHO_AM_I Read Acknowledgment: %x\n", readData);
	printf("Value read from WHO_AM_I: %x\n", readBuffer);
	delay_ms(500);

	// read the ACCEL_X_OUT first
	uint8_t readBuffer1;
	int32_t read_ack1 = i2c_m_sync_cmd_read(&I2C_0, ACCEL_X_OUT, &readBuffer1, 1);
	
	printf("\n1st ACCEL_X_OUT Read Acknowledgment: %d\n",read_ack1);
	printf("Value read from ACCEL_X_OUT: %x\n",readBuffer1);
	delay_ms(500);

	// testing write on the ACCEL_X_OUT address
	uint8_t writeBuffer = 0x20;
	int32_t write = i2c_m_sync_cmd_write(&I2C_0, ACCEL_X_OUT, &writeBuffer, 1);
	
	
	// reading the value which was written on ACCEL_X_OUT
	printf("ACCEL_X_OUT Write Acknowledgment: %d\n", write);
	delay_ms(500);
	uint8_t readBuffer2;
	int32_t read_ack2 = i2c_m_sync_cmd_read(&I2C_0, ACCEL_X_OUT, &readBuffer2, 1);
	printf("\nACCEL_X_OUT Read Acknowledgment : %d\n",read_ack2);
	printf("Value read from ACCEL_X_OUT : %x      \n",readBuffer2);
}

