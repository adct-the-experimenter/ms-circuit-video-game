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

//function to read value from string
bool read_adc_value_from_char_buffer(char* read_buf_ptr, uint16_t* adc_val, uint8_t* adc_buf_index);


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
	enum class ReaderState { STARTING_READ=0, READING_ADC_CHANNEL, READING_ADC_VALUE, READ_DONE };
	ReaderState reader_state = ReaderState::STARTING_READ;
	
	
	uint16_t adc_val = 0;
	uint8_t adc_channel = 0;
	
	signal(SIGINT, handle_sigint);
	while (!g_quitProgram)
	{
		// Wait for data to be available at the serial port.
		while(!serial_port.IsDataAvailable()) 
		{
			usleep(1000) ;
		}

		// Specify a timeout value (in milliseconds).
		size_t ms_timeout = 250 ;

		// Char variable to store data coming from the serial port.
		char data_byte;
		
		
		// Read one byte from the serial port and print it to the terminal.
		try
		{
			// Read a single byte of data from the serial port.
			serial_port.ReadByte(data_byte, ms_timeout) ;
		}
		catch (const LibSerial::ReadTimeout&)
		{
			std::cerr << "\nThe ReadByte() call has timed out." << std::endl ;
		}
		
		
		// Show the user what is being read from the serial port.
		std::cout << data_byte;
		
		switch(reader_state)
		{
			case ReaderState::STARTING_READ:
			{
				//if space character found, then skip and wait for \n
				if(data_byte == ' ')
				{
					//reset buffer adc val index
					read_buf_adc_val_index = 0;
					
					//do nothing
				}
				else if(data_byte == '\n')
				{
					//change state to reading adc channel
					reader_state = ReaderState::READING_ADC_CHANNEL;
				}
				break;
			}
			case ReaderState::READING_ADC_CHANNEL:
			{
				switch(data_byte)
				{
					case '0':{adc_channel = 0; break;}
					case '1':{adc_channel = 1; break;}
					case '2':{adc_channel = 2; break;}
					case '3':{adc_channel = 3; break;}
					case '4':{adc_channel = 4; break;}
					case '5':{adc_channel = 5; break;}
					case '6':{adc_channel = 6; break;}
					case '7':{adc_channel = 7; break;}
				}
				
				//if space character found
				if(data_byte == ' ')
				{
					//reset buffer adc val index
					read_buf_adc_val_index = 0;
					
					//change state to reading adc value
					reader_state = ReaderState::READING_ADC_VALUE;
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
				read_buf_adc_val[read_buf_adc_val_index] = data_byte;
				read_buf_adc_val_index++;
				
				//if space character found, then skip and wait for \n
				if(data_byte == ' ')
				{
					//do nothing
				}
				//if \n character found 
				else if(data_byte == '\n')
				{
					
					//convert chars in read buf to integer and store it
					adc_val = atoi(read_buf_adc_val);
					
					//change state to read done
					reader_state = ReaderState::READ_DONE;
				}
				  
				break;
			}
			case ReaderState::READ_DONE:
			{
				std::cout << "adc channel: " << uint(adc_channel) << ", adc value: " << adc_val << "\n";
				reader_state = ReaderState::READING_ADC_CHANNEL;
				break;
			}
		}
	    
	}

	return 0; // success
  
};



bool read_adc_value_from_char_buffer(char* read_buf_ptr,uint16_t* adc_val_ptr, uint8_t* adc_buf_index_ptr)
{
	//bool to indicate if valid adc value received
	bool recv_valid_adc_buf_value = false;

	char* first_char = &read_buf_ptr[0];
	*adc_buf_index_ptr = atoi(first_char);

	bool adc_buf_index_valid = false;

	if(*adc_buf_index_ptr >= 0 && *adc_buf_index_ptr <= 7){adc_buf_index_valid = true;}

	bool space_char_present = false;

	char* second_char = &read_buf_ptr[1];
	if(*second_char == ' '){space_char_present = true;}

	recv_valid_adc_buf_value = space_char_present & adc_buf_index_valid;

	if(recv_valid_adc_buf_value)
	{
		char sub_str_num[7];

		strncpy(sub_str_num,&read_buf_ptr[2],6);
		sub_str_num[6] = '\0';

		*adc_val_ptr = atoi(sub_str_num);

		printf("Original string: %s \n", read_buf_ptr);
		printf("Received adc value: %d , index: %d\n", *adc_val_ptr, *adc_buf_index_ptr);
	}
	
	return recv_valid_adc_buf_value;
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


/**
 * @brief This example demonstrates configuring a serial port and 
 *        reading serial data.
 */
int testLibSerial()
{
    
     
    

    // Successful program completion.
    std::cout << "The example program successfully completed!" << std::endl ;
    return EXIT_SUCCESS ;
}
