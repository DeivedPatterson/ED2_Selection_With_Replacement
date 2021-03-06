#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/Exception.h"
#include "../include/Ultility.h"
#include "../include/ArrayList.h"
#include "../include/Register.h"

#define MAX_REGISTERS 1000UL	


extern FILE* PartitionCreate(void);
extern FILE* CreateTestFile(void* tupl);
extern void PartitionClose(FILE* file);
static unsigned partitionsCounter = 0;
static void PrintPartitions(void);

static int compare(Object obj1, Object obj2)
{
	int res;

	res = (((Register)obj1)->rField.bit.key - ((Register)obj2)->rField.bit.key);

	return res;
}

static void format(Object obj)
{
	printf("Key:%i freeze: %i\n",((Register)obj)->rField.bit.key,((Register)obj)->rField.bit.freeze);
}

static bool allFrozen(ArrayList list)
{
	Register temp;
	unsigned counter = 0;
	unsigned listSize;

	listSize = getListSize(list);

	foreach_ArrayList(temp, list)
	{
		if(temp->rField.bit.freeze == true)
		{
			counter += 1;
		}
	}
	

	return counter == listSize?true:false;
}

static void unfreezeRecords(ArrayList list)
{
	Register temp;

	
	foreach_ArrayList(temp, list)
	{
		temp->rField.bit.freeze = false;
	}

}

Register GetRegisterUnfreeze(ArrayList list)
{
	Register temp;
	int i = 0;

	foreach_ArrayList(temp, list)
	{
		if(temp->rField.bit.freeze == false)
		{
			
			return (Register)removeElementList(list, temp, compare);
		}
	}

	return NULL;
}

void SubstitutionSelection(FILE* file, int mregisters)
{
	FILE* partition;
	ArrayList registers;
	struct Register reg;
	Register r;
	bool endFile = false;
	int resWrite, resRead = MAX_REGISTERS;

	registers = newArrayList(mregisters);

	ReWind(file);
	partition = PartitionCreate();
	partitionsCounter += 1;

	foreach_mRegisters(reg, file, mregisters)
	{
		insertSorted(registers, newRegister(reg), compare);
	}
	
	
	while(endFile != true)
	{
		r = GetRegisterUnfreeze(registers);

		if(r != NULL)
		{
			FileWrite(r, sizeof(struct Register), 1, partition, resWrite);
			FileRead(&reg, sizeof(struct Register),1, file, resRead);
			if(resRead > 0)
			{
				if(reg.rField.bit.key < r->rField.bit.key)
				{
					reg.rField.bit.freeze = true;
					r->rField.bit.freeze = reg.rField.bit.freeze;
					r->rField.bit.key = reg.rField.bit.key;
					r->info = reg.info;
				}
				else
				{
					reg.rField.bit.freeze = false;
					r->rField.bit.freeze = reg.rField.bit.freeze;
					r->rField.bit.key = reg.rField.bit.key;
					r->info = reg.info;
				}

				insertSorted(registers, r, compare);
			}	
		}
		else if(isEmpty(registers) && resRead == 0)
		{
			endFile = true;
		}
		else if(allFrozen(registers))
		{
			unfreezeRecords(registers);
			PartitionClose(partition);
			partition = PartitionCreate();
			partitionsCounter += 1;
		}
	}


	PartitionClose(partition);
	destroyArrayList(registers);

}

void PrintPartitions(void)
{
	FILE* partition = NULL;
	char bufferStr[64];
	unsigned i;
	struct Register reg;

	for(i = 0; i < partitionsCounter; i++)
	{
		sprintf(bufferStr,"Partitions/Partition %u.bin",i);
		FileOpen(partition, bufferStr, "rb");
		puts(bufferStr);
		PrintFile(partition);
	}	
}

int main(int argc, char const *argv[])
{
	FILE* file;
	int nReg,mregisters;

	unsigned long long sizeReg = sizeof(struct Register);

	scanf("%i",&nReg);
	scanf("%i",&mregisters);
	void* tupl[3] = {&nReg, CreateAleatoryRegister, &sizeReg};

	file = CreateTestFile(&tupl);
	puts("Arquivo Test");
	PrintFile(file);
	
	SubstitutionSelection(file, mregisters);

	PrintPartitions();

	FileClose(file);


	return 0;
}