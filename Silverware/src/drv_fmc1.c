

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

int fmc_write2( )
{	
	FLASH_Unlock();
	int test = FLASH_ErasePage( FLASH_ADDR );
	if ( test != FLASH_COMPLETE ) FLASH_Lock();
	unsigned long count = 0;
	writeword(count , 0x12345678);
	count++;
	writeword(count ,0x24681359);	 
	FLASH_Lock();
	return 0;
}


unsigned long test1 , test2;


// reads 32 bit value
unsigned long fmc_read(unsigned long address)
{
	address = address * 4 + FLASH_ADDR;
	unsigned int *addressptr = (unsigned int *)address;
	return (*addressptr);
}

void fmc_read2( )
{	
	test1 =	fmc_read(0);
	test2 = fmc_read(1);
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

