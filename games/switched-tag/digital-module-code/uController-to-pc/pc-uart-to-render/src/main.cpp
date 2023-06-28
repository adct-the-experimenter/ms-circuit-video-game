// C library headers
#include <cstdio>
#include <cstring>
#include <string>


#include <libserial/SerialPort.h>

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <bitset>

#include <signal.h> //for signal interrupt


#include "raylib.h" //for drawing 

//for thread handling
#include <thread>
#include <mutex>
#include <atomic>

#include "moving_average.h"

//#define USING_DEBUG

//variable used to indicate that program needs to quit
//made atomic because it needs to be updated in main thread and adc update buffer thread
static std::atomic<bool> g_quitProgram;


// Handler for SIGINT, caused by
// Ctrl-C at keyboard
void handle_sigint(int sig)
{
	g_quitProgram.store(true);
    printf("Caught interrupt signal, signal %d\n", sig);
}	

//the serial port that will host UART and PC communication
LibSerial::SerialPort serial_port;
const char* const SERIAL_PORT_1 = "/dev/ttyACM0" ;


//function to initialize communication between UART and PC
//returns bool indicating if initialization was successful or not.
bool init_UART_PC_comm();

enum class ReaderState { STARTING_READ=0, READING_ADC_CHANNEL, READING_ADC_VALUE, READ_DONE };

//function to read value from UART serial port
void read_adc_value_from_UART(uint16_t* adc_val_ptr, uint8_t* adc_buf_index_ptr, size_t* ms_timeout,
							  char* read_buf_adc_val_ptr, uint8_t* read_buf_adc_val_index_ptr, 
							  ReaderState* reader_state_ptr);


//function to initialize raylib
bool InitRaylib();
void CloseRaylib();

const int screenWidth = 800;
const int screenHeight = 450;
void render();

#define NUM_ADC_INPUTS 8

#define ADC_MAX_VAL 4095
#define ADC_MIN_VAL 0

enum class ADC_Channel : uint8_t {P1X=0, P1Y, P2X, CD, PD, P2Y, P2_IT, P1_IT };
float adc_channel_info[NUM_ADC_INPUTS] = {0,0,0,0,0,0,0,0};


//for granting exclusive access to adc buffer
std::mutex mutex_adc_buf;

//function to update adc buffer
void UpdateADCBuffer();

void UpdateADCBuffer()
{
	// Allocate memory for read buffer, set size according to your needs
	char read_buf_adc_val[100];
	uint8_t read_buf_adc_val_index = 0;
	memset(&read_buf_adc_val, '\0', sizeof(read_buf_adc_val));
	
	//set up state machine for reading adc channel and value and matching channel to value.
	ReaderState reader_state = ReaderState::STARTING_READ;	
	
	uint16_t adc_val = 0;
	uint8_t adc_channel = 0;
	
	// Specify a timeout value (in milliseconds) for UART communication.
	size_t ms_timeout = 250;
	
	//bitset to keep track of which channels have been updated
	std::bitset <NUM_ADC_INPUTS> updated_channels_bitset;
	updated_channels_bitset.reset();
	
	//second buffer to transfer data to main adc buffer
	float temp_adc_buf[NUM_ADC_INPUTS] = {0,0,0,0,0,0,0,0};
	
	//loop forever
	while(true)
	{
		//if quit program is true, then break out of this loop
		if(g_quitProgram.load()){break;}
		
		//read adc channel and value from UART
		read_adc_value_from_UART(&adc_val,&adc_channel,&ms_timeout,
								&read_buf_adc_val[0], &read_buf_adc_val_index,
								&reader_state);
		
		
		 //if adc read done, upate temporary buffer and bitset
		if(reader_state == ReaderState::READ_DONE)
		{	 
			 temp_adc_buf[adc_channel] = (float)adc_val;
			 updated_channels_bitset.set(adc_channel);	 
		}
		 
		
		//update adc buffer if all channels in temporary buffer have been updated.
		if(updated_channels_bitset.all())
		{
			//printf("All channels updated! buffer update unlocked.");
			//put lock on adc buffer to update it
			//critical section
			mutex_adc_buf.lock();
			
			for(int i = 0; i < NUM_ADC_INPUTS; i++)
			{
				adc_channel_info[i] = temp_adc_buf[i];
			}
			
			//unlock adc buffer 
			mutex_adc_buf.unlock();
			
			updated_channels_bitset = 0; 
			
		}
	}
	
	
	
}


int main() {
	
	int num_concurrent_threads = std::thread::hardware_concurrency();
	std::cout << "The number of concurrent threads is " << num_concurrent_threads << "\n";
	
	//if pc does not have at least 2 concurrent threads, exit program
	if(num_concurrent_threads < 2)
	{
		printf("Error! Need CPU with 2 or more concurrent threads to run this program.");
		return -1;
	}
  
	if(!InitRaylib())
	{
		printf("Failed to initialize raylib!\n");
		return -1;
	}
  
	//if initialization of UART PC communication fails, exit program
	if(!init_UART_PC_comm())
	{
	  printf("Failed to initialize PC UART communication!\n");
	  return -1;
	}
	
	//start thread to update adc buffer
	std::thread adc_buf_reader(&UpdateADCBuffer);
	
	g_quitProgram.store(false);
	
	signal(SIGINT, handle_sigint);
	
	while (!g_quitProgram.load())
	{	    
	    //draw stuff.
	    render();   
	}
	
	CloseRaylib();
	
	adc_buf_reader.join();
	
	return 0; // success
  
};

void CloseRaylib()
{
	 CloseWindow(); 
}

#define USE_MOVING_AVERAGE

#define MOV_AVG_PERIOD 6
MovingAverage mavg_p1x(MOV_AVG_PERIOD);
MovingAverage mavg_p1y(MOV_AVG_PERIOD);
MovingAverage mavg_p2x(MOV_AVG_PERIOD);
MovingAverage mavg_p2y(MOV_AVG_PERIOD);

const float circle_size = 10.0f;

void render()
{
	//P1X=0, P1Y, P2X, CD, PD, P2Y, P2_IT, P1_IT
	
	if(WindowShouldClose()){g_quitProgram.store(true);}
	
	float p1x, p1y, p2x, p2y, collision_detect;
	
	//put lock on adc buffer to read from it without buffer being updated during read
	//if not locked, block until mutex is available for use
	//perform all reads from adc in one section
	//critical section 
	mutex_adc_buf.lock();
	
	p1x = adc_channel_info[static_cast<int>(ADC_Channel::P1X)];
	p1y = adc_channel_info[static_cast<int>(ADC_Channel::P1Y)];
	p2x = adc_channel_info[static_cast<int>(ADC_Channel::P2X)];
	p2y = adc_channel_info[static_cast<int>(ADC_Channel::P2Y)];
	collision_detect = adc_channel_info[static_cast<int>(ADC_Channel::CD)];
	
	//unlock adc buffer, let it be updated
	mutex_adc_buf.unlock();
	
	#ifdef USING_DEBUG
	char str[80];
	
	//DrawText(const char *text, int posX, int posY, int fontSize, Color color);
	sprintf(str, "adc[p1x]: %f", adc_channel_info[static_cast<int>(ADC_Channel::P1X)]);
	DrawText(str, 0, 0, 12, BLACK);

	sprintf(str, "adc[p1y]: %f " , adc_channel_info[static_cast<int>(ADC_Channel::P1Y)] );
	DrawText(str, 0, 20, 12, BLACK);
	
	sprintf(str, "adc[p2x]: %f " , adc_channel_info[static_cast<int>(ADC_Channel::P2X)] );
	DrawText(str, 0, 40, 12, BLACK);
	
	sprintf(str, "adc[p2y]: %f " , adc_channel_info[static_cast<int>(ADC_Channel::P2Y)] );
	DrawText(str, 0, 60, 12, BLACK);
	
	sprintf(str, "adc[p1_it]: %f " , adc_channel_info[static_cast<int>(ADC_Channel::P1_IT)] );
	DrawText(str, 0, 80, 12, BLACK);
	
	sprintf(str, "adc[p2_it]: %f " , adc_channel_info[static_cast<int>(ADC_Channel::P2_IT)] );
	DrawText(str, 0, 100, 12, BLACK);
	
	sprintf(str, "adc[cd]: %f " , adc_channel_info[static_cast<int>(ADC_Channel::CD)] );
	DrawText(str, 0, 120, 12, BLACK);
	
	sprintf(str, "adc[pd]: %f " , adc_channel_info[static_cast<int>(ADC_Channel::PD)] );
	DrawText(str, 0, 140, 12, BLACK);
	#endif
	
	//convert to screen coordinates
	//reverse x coordinates because joystick direction is inversely proportional to adc value
	p1x = ( p1x / (float)(ADC_MAX_VAL - ADC_MIN_VAL) )*screenWidth;
	
	#ifdef USE_MOVING_AVERAGE
	mavg_p1x.addData(p1x);
	p1x = mavg_p1x.getMean();
	#endif
	
	p1y = screenHeight - ( p1y / (float)(ADC_MAX_VAL - ADC_MIN_VAL) )*screenHeight;
	
	#ifdef USE_MOVING_AVERAGE
	mavg_p1y.addData(p1y);
	p1y = mavg_p1y.getMean();
	#endif
	
	p2x = ( p2x / (float)(ADC_MAX_VAL - ADC_MIN_VAL) )*screenWidth;
	
	#ifdef USE_MOVING_AVERAGE
	mavg_p2x.addData(p2x);
	p2x = mavg_p2x.getMean();
	#endif
	
	p2y = screenHeight - ( p2y / (float)(ADC_MAX_VAL - ADC_MIN_VAL) )*screenHeight;
	
	#ifdef USE_MOVING_AVERAGE
	mavg_p2y.addData(p2y);
	p2y = mavg_p2y.getMean();
	#endif
	
	Vector2 p1Position = { p1x, p1y};
	Vector2 p2Position = { p2x, p2y};
	
	//set color based on collision detect event
	Color p1_color = RED;
	Color p2_color = BLUE;
	
	if(collision_detect > 3000)
	{
		p1_color = YELLOW;
		p2_color = YELLOW;
	}
	
	//set size based on which player is it.
	float p1_size_mod = 0.02 * adc_channel_info[static_cast<int>(ADC_Channel::P1_IT)];
	float p2_size_mod = 0.02 * adc_channel_info[static_cast<int>(ADC_Channel::P2_IT)];

	BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawCircleV(p1Position, circle_size + p1_size_mod, p1_color);
    
    DrawCircleV(p2Position, circle_size + p2_size_mod, p2_color);
    

    EndDrawing();
}



void read_adc_value_from_UART(uint16_t* adc_val_ptr, uint8_t* adc_channel_ptr, size_t* ms_timeout,
							  char* read_buf_adc_val_ptr, uint8_t* read_buf_adc_val_index_ptr,
							  ReaderState* reader_state_ptr)
{
	// Wait for data to be available at the serial port.
	while(!serial_port.IsDataAvailable()) 
	{
		//make thread sleep for 1000 microseconds instead of whole CPU program
		//read_adc_value_from_UART function is in use in adc buffer update thread
		 std::this_thread::sleep_for(std::chrono::microseconds(1000));
	}


	// Char variable to store data coming from the serial port.
	char data_byte;
	
	
	// Read one byte from the serial port and print it to the terminal.
	try
	{
		// Read a single byte of data from the serial port.
		serial_port.ReadByte(data_byte, *ms_timeout) ;
	}
	catch (const LibSerial::ReadTimeout&)
	{
		std::cerr << "\nThe ReadByte() call has timed out." << std::endl ;
	}
	
	
	// Show the user what is being read from the serial port.
	//std::cout << data_byte;
	
	switch(*reader_state_ptr)
	{
		case ReaderState::STARTING_READ:
		{
			//if space character found, then skip and wait for \n
			if(data_byte == ' ')
			{
				//reset buffer adc val index
				*read_buf_adc_val_index_ptr = 0;
				
				//do nothing
			}
			else if(data_byte == '\n')
			{
				//change state to reading adc channel
				*reader_state_ptr = ReaderState::READING_ADC_CHANNEL;
			}
			break;
		}
		case ReaderState::READING_ADC_CHANNEL:
		{
			switch(data_byte)
			{
				case '0':{*adc_channel_ptr = 0; break;}
				case '1':{*adc_channel_ptr = 1; break;}
				case '2':{*adc_channel_ptr = 2; break;}
				case '3':{*adc_channel_ptr = 3; break;}
				case '4':{*adc_channel_ptr = 4; break;}
				case '5':{*adc_channel_ptr = 5; break;}
				case '6':{*adc_channel_ptr = 6; break;}
				case '7':{*adc_channel_ptr = 7; break;}
			}
			
			//if space character found
			if(data_byte == ' ')
			{
				//reset buffer adc val index
				*read_buf_adc_val_index_ptr = 0;
				
				//change state to reading adc value
				*reader_state_ptr = ReaderState::READING_ADC_VALUE;
			}
			//if \n character found
			else if(data_byte == '\n')
			{
				//do nothing.
			}  
			
			break;
		}
		case ReaderState::READING_ADC_VALUE:
		{
			read_buf_adc_val_ptr[*read_buf_adc_val_index_ptr] = data_byte;
			*read_buf_adc_val_index_ptr = *read_buf_adc_val_index_ptr + 1;
			
			//if space character found, then skip and wait for \n
			if(data_byte == ' ')
			{
				//do nothing
			}
			//if \n character found 
			else if(data_byte == '\n')
			{
				
				//convert chars in read buf to integer and store it
				*adc_val_ptr = atoi(read_buf_adc_val_ptr);
				
				//change state to read done
				*reader_state_ptr = ReaderState::READ_DONE;
			}
			  
			break;
		}
		case ReaderState::READ_DONE:
		{
			//std::cout << "adc channel: " << uint(*adc_channel_ptr) << ", adc value: " << *adc_val_ptr << "\n";
			*reader_state_ptr = ReaderState::READING_ADC_CHANNEL;
			break;
		}
	}
}


bool init_UART_PC_comm()
{
	try
    {
        // Open the Serial Port at the desired hardware port.
        serial_port.Open(SERIAL_PORT_1) ;
    }
    catch (const LibSerial::OpenFailed&)
    {
        std::cerr << "The serial port did not open correctly." << std::endl ;
        return false;
    }

    // Set the baud rate of the serial port.
    serial_port.SetBaudRate(LibSerial::BaudRate::BAUD_115200) ;

    // Set the number of data bits.
    serial_port.SetCharacterSize(LibSerial::CharacterSize::CHAR_SIZE_8) ;

    // Turn off hardware flow control.
    serial_port.SetFlowControl(LibSerial::FlowControl::FLOW_CONTROL_NONE) ;

    // Disable parity.
    serial_port.SetParity(LibSerial::Parity::PARITY_NONE) ;
    
    // Set the number of stop bits.
    serial_port.SetStopBits(LibSerial::StopBits::STOP_BITS_1) ;
    
    return true;
}


bool InitRaylib()
{
	

    InitWindow(screenWidth, screenHeight, "Switched Tag Rendering Window");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    
    return true;
}
