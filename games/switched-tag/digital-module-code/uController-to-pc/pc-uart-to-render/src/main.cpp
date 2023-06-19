// C library headers
#include <cstdio>
#include <cstring>
#include <string>


#include <libserial/SerialPort.h>

#include <cstdlib>
#include <iostream>
#include <unistd.h>


#include <signal.h> //for signal interrupt

//Tutorial code from 
//https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/


//variable used to indicate that program needs to quit
int g_quitProgram = 0;

// Handler for SIGINT, caused by
// Ctrl-C at keyboard
void handle_sigint(int sig)
{
	g_quitProgram = 1;
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


int main() {
  
  
	//if initialization of UART PC communication fails, exit program
	if(!init_UART_PC_comm())
	{
	  printf("Failed to initialize PC UART communication!\n");
	  return -1;
	}

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
	
	signal(SIGINT, handle_sigint);
	while (!g_quitProgram)
	{
		read_adc_value_from_UART(&adc_val,&adc_channel,&ms_timeout,
								&read_buf_adc_val[0], &read_buf_adc_val_index,
							    &reader_state);
	    
	}

	return 0; // success
  
};



void read_adc_value_from_UART(uint16_t* adc_val_ptr, uint8_t* adc_channel_ptr, size_t* ms_timeout,
							  char* read_buf_adc_val_ptr, uint8_t* read_buf_adc_val_index_ptr,
							  ReaderState* reader_state_ptr)
{
	// Wait for data to be available at the serial port.
	while(!serial_port.IsDataAvailable()) 
	{
		usleep(1000) ;
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
	std::cout << data_byte;
	
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
			std::cout << "adc channel: " << uint(*adc_channel_ptr) << ", adc value: " << *adc_val_ptr << "\n";
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
