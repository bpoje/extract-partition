#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

//Extracts one of 4 primary partitions from disk image
//gcc -o extPart extPart.c -Wall

//OFFSETS IN BYTES
const int indexMasterPartitionTable = 446;
const int indexBootSignature = 510;
//Offset of Startin Sector field (in Master Partition Table)
const int indexStartingSector = 8;
const int bytesPerSector = 512;

struct partitionInfo
{
	unsigned int startingSector;
	unsigned int partitionSize;
} partitions[4];

//Verify boot signature
//Returns:
//1 - Boot signature OK
//0 - Wrong boot signature in mbr
int verifyBootSignature( FILE *in )
{
	unsigned int buffer = 0;
	size_t numOfRead = 0;

	//Move to boot signature in Master Partition Table
	fseek( in, indexBootSignature, SEEK_SET );

	//Read one 2-Byte number
	numOfRead = fread( &buffer, 2, 1, in );

	if( numOfRead != 1 )
		return 0;

	//printf("buffer: %u\n", buffer);

	//Is boot signature valid?
	if( buffer == 0xAA55 )
		return 1;
	else
		return 0;
}

//Read data about partitions
//Returns:
//1 - OK
//0 - Fail
int readPartitionsInfo( FILE *in )
{
	int partitionNum = 0;
	size_t readElementsOfData = 2;
	size_t dataSizeInBytes = 4;
	unsigned int buffer[readElementsOfData];
	size_t numOfRead;

	//Move through file to "Starting Sector" field (to its first Byte)
	//("Starting Sector" is in "Master Partition Table")
	fseek( in, indexMasterPartitionTable + indexStartingSector, SEEK_SET );

	//For all 4 partitions store value of "Starting Sector" and "Partition Size"
	//to partitions structure
	for( partitionNum = 0; partitionNum < 4; partitionNum++ )
	{
		//Read two 4-Byte numbers (integers)
		numOfRead = fread( buffer, dataSizeInBytes, readElementsOfData, in );

		if( numOfRead != readElementsOfData )
			return 0;

		partitions[partitionNum].startingSector = buffer[0];
		partitions[partitionNum].partitionSize = buffer[1];

		printf( "Partition %d\n", partitionNum );
		printf( "\tStarting sector: %u\n", buffer[0] );
		printf( "\tPartition size (in sectors): %u\n", buffer[1] );
		printf( "\n" );

		//Move from current position in file to the first Byte of
		//"Starting Sector" (Starting Sector which belongs to NEXT
		//partition)
		fseek( in, 8, SEEK_CUR );
	}

	return 1;
}

//Verify if partition exists
//Returns:
//1 - Partition OK
//0 - Partition doesn't exist
int verifyPartition( int getPartition )
{
	//Info about the partition we are going to extract
	unsigned int startingSector = partitions[getPartition].startingSector;
	unsigned int partitionSize = partitions[getPartition].partitionSize;

	//printf("Starting Sector: %u, Partition Size: %u\n", startingSector, partitionSize);

	//Check if selected partition exists
	if( startingSector == 0 || partitionSize == 0 )
		return 0;

	return 1;
}

//Extracts partition
//Returns:
//1 - Partition transfered OK
//0 - Error
int copyPartition( FILE *in, FILE *out, int getPartition )
{
	//Info about the partition we are going to extract
	unsigned int startingSector = partitions[getPartition].startingSector;
	unsigned int partitionSize = partitions[getPartition].partitionSize;

	printf( "Selected partition: %d\n", getPartition );
	printf( "Starting Sector: %u, Partition Size: %u\n", startingSector, partitionSize );

	//Move to the first Byte of selected partition
	fseek( in, ( long )bytesPerSector * ( long )startingSector, SEEK_SET );

	unsigned char buffer1[bytesPerSector];
	size_t readElementsOfData = bytesPerSector;
	size_t dataSizeInBytes = 1;

	unsigned int sectorsTransfered = 0;
	size_t numOfRead, numOfWrite;

	//WHILE (entire sector is read AND there are still sectors to transfer)
	//fread function will read one sector (= 512 Bytes = number of Bytes per Sector)
	while( ( numOfRead = fread( buffer1, dataSizeInBytes, readElementsOfData, in ) ) ==
	        readElementsOfData && sectorsTransfered < partitionSize )
	{
		//Write sector to output
		numOfWrite = fwrite( buffer1, dataSizeInBytes, numOfRead, out );

		//number of items read != number of items written
		if( numOfWrite != numOfRead )
			return 0;

		//printf("%u: sector %lu transfered\n", sectorsTransfered+1,
		//  (unsigned long)startingSector + sectorsTransfered);

		sectorsTransfered++;
	}

	//Check if entire partition was transfered
	if( sectorsTransfered != partitionSize )
		return 0;

	printf( "Sectors transfered: %u\n", sectorsTransfered );

	return 1;
}

int main( int argc, char **argv )
{
	FILE *in;
	FILE *out;

	if( argc != 4 )
	{
		printf( "Usage: fileIn filePartitionOut "
		        "numberOfPartition(0,1,2 or 3)\n" );
		return 1;
	}

	if( strlen( argv[3] ) != 1 || !isdigit( argv[3][0] ) )
	{
		printf( "Wrong partition number\n" );
		return 1;
	}

	int getPartition = atoi( argv[3] );

	if( getPartition < 0 || getPartition > 3 )
	{
		printf( "Wrong partition number\n" );
		return 1;
	}

	in = fopen( argv[1],"r" );

	if( in == NULL )
	{
		printf( "Can't open fileIn\n" );
		return 1;
	}

	out = fopen( argv[2],"w" );

	if( out == NULL )
	{
		printf( "Can't open filePartitionOut\n" );
		return 1;
	}

	if( verifyBootSignature( in ) )
		printf( "Boot signature OK!\n" );
	else
	{
		printf( "Wrong boot signature in mbr! Is fileIn valid disk image?\n" );
		return 1;
	}

	if( !readPartitionsInfo( in ) )
	{
		printf( "Failed to read data about partitions" );
		return 1;
	}

	if( !verifyPartition( getPartition ) )
	{
		printf( "Selected partition does not exist\n" );
		return 1;
	}

	if( !copyPartition( in, out, getPartition ) )
	{
		printf( "Error while extracting partition\n" );
		return 1;
	}

	printf( "Done!\n" );

	fclose( in );
	fclose( out );

	return 0;
}
