

#include "project.h"
#include "drv_fmc.h"

#define FLASH_ADDR 0x08003C00

extern void failloop( int);

// address 0 - 255 ( for current block FLASH_ADDR )
// value 32 bit to write
void writeword( unsigned long address, unsigned long value)
{
	int test = FLASH_ProgramWord( FLASH_ADDR + (address<<2), value);	
	if ( test != FLASH_COMPLETE )
	{
		FLASH_Lock();
		failloop(5);
	}
}

void fmc_write_float(unsigned long address, float float_to_write) {
	writeword(address, *(unsigned long *) &float_to_write);
}

float fmc_read_float(unsigned long address) {
	unsigned long result = fmc_read(address);
	return *(float*)&result;
}

void fmc_unlock() {
	FLASH_Unlock();
}

void fmc_lock() {
	FLASH_Lock();
}

int fmc_erase( void )
{	
	int test = FLASH_ErasePage( FLASH_ADDR );
	if ( test != FLASH_COMPLETE ) FLASH_Lock();
    else return 0;
	return 1;// error occured
}

// reads 32 bit value
unsigned long fmc_read(unsigned long address)
{
	address = address * 4 + FLASH_ADDR;
	unsigned int *addressptr = (unsigned int *)address;
	return (*addressptr);
}

int fmc_write( int data1 , int data2)
{
	FLASH_Unlock();
	FLASH_OB_Unlock();
	FLASH_OB_Erase();
	FLASH_OB_ProgramData( 0x1FFFF804, data1 );
	FLASH_OB_ProgramData( 0x1FFFF806, data2 );
	FLASH_Lock();
	FLASH_OB_Lock();
	return 0;
}


// x = readdata( OB->DATA0 );
// x = readdata( OB->DATA1 );
	
int readdata( unsigned int data )
{
	// checks that data and ~data are valid
	unsigned int userdata = data ;
	int complement = ((userdata &0x0000FF00)>>8 );
	complement |=0xFFFFFF00;

	userdata&=0x000000FF;
	
	if ( userdata!=~complement) 
		return 127;
	
	else return userdata;
}

