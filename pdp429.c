#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TRUE 1
#define FALSE 0

#define typeBuffer 0xF000

#define upperEight 0xFF00
#define lowerEight 0xFF
#define oneWord 0xFFFF
#define overflowBit 0x10000

//type1
#define subOpcode 0x03FF
#define upperSix 0xFC00
//use 1 for HLT
//use 2 for RET
//use 0 for NOP


//type2
#define lowerEightPage 0x00FF
#define twoRegBuffer 0x0C00
#define typeTwoB 0x0400
#define typeTwoC 0x0800
//use 0 for typeTwoA (A register)
//use twoRegBuffer for typeTwoD (D register)
//use bitEight for Z/C
//use bitNine for D/I


//type3
#define deviceBuffer 0x03F8
#define deviceThree 0x0018
#define deviceFour 0x0020
//use twoRegBuffer for reg


//type4
#define upperSixOpcode 0xFC00
#define typeFourISZ 0xB000
#define typeFourJMP 0xB400
#define typeFourCALL 0xB800
#define typeFourPUSH 0xC000
#define typeFourPOP 0xC400
//use lowerEightPage for page offset
//use bitEight for Z/C
//use bitNine for D/I


//type5
#define regK
#define regJ
#define regI
#define typeFiveSubOpcode 0x0E00
#define typeFiveMOD 0x0000
#define typeFiveADD 0x0200
#define typeFiveSUB 0x0400
#define typeFiveMUL 0x0600
#define typeFiveDIV 0x0800
#define typeFiveAND 0x0A00
#define typeFiveOR 0x0C00
#define typeFiveXOR 0x0E00
//or use typeFiveSubOpcode for XOR
#define typeFiveRegA 0x0000
#define typeFiveRegB 0x0001
#define typeFiveRegC 0x0002
#define typeFiveRegD 0x0003
#define typeFivePC 0x0004
#define typeFivePSW 0x0005
#define typeFiveSP 0x0006
#define typeFiveSPL 0x0007
//shift all regs down to analyize regs correctly because all regs are based on lower three (shift 0 3 or 6)


//type6
#define bitZero 0x0001
#define bitOne 0x0002
#define bitTwo 0x0004
#define bitThree 0x0008
#define bitFour 0x0010
#define bitFive 0x0020
#define bitSix 0x0040
#define bitSeven 0x0080
#define bitEight 0x0100
#define bitNine 0x0200
//use twoRegBuffer for reg
//use 0 for  (A register)
//use twoRegBuffer for  (D register)
//use typeTwoB for B register
//use typeTwoC for C register


#define typeOne 0x0000
#define typeTwoAdd 0x1000
#define typeTwoSub 0x2000
#define typeTwoMul 0x3000
#define typeTwoDiv 0x4000
#define typeTwoAnd 0x5000
#define typeTwoOr 0x6000
#define typeTwoXor 0x7000
#define typeTwoLd 0x8000
#define typeTwoSt 0x9000
#define typeThree 0xA000
#define typeFourA 0xB000
#define typeFourB 0xC000
#define typeFive 0xE000
#define typeSix 0xF000


int reg[4];
int pc;
int linkBit;
int psw;
int sp;
int spl;
int mem[65536];
int verbose;
int time;
char* regStr[300];
char* commandName[300];

void regCode(int regNum, char* ret);

void main( int argc, const char* argv[] ){
    if(argc>=2){
		int indexOfFile=1;
		if(argc>=3 && strcmp("-v",argv[1])==0){
	    	verbose=TRUE;
			indexOfFile++;
	    }
		int check=readObject(argv[indexOfFile]);
		switch(check){
			case -1:
				printf("\nUndefined error\n");
				break;
			case -2:
				printf("\nFile Not Found Error\n");
				break;
			case -3:
				printf("\nStep is not 3 when we get to end of a set\n");
				break;
			case -4:
				printf("\nPremature EOF error\n");
				break;
			case -5:
				printf("\nBytes read exceeds maximum indicated\n");
				break;
			case -6:
				printf("\nOBJG not present at begining\n");
				break;
		}
		
		//edits need to be made below!
		short keepOperating=TRUE;
		int pcBefore;
		int theInst;
		while(psw&bitZero==1 && keepOperating==TRUE){
			commandName[0]='\0';
			regStr[0]='\0';
			pc=pc&oneWord;
			theInst=mem[pc];
			pcBefore=pc;
			keepOperating=operate(mem[pc]);
			if(verbose==TRUE){
				fprintf(stderr,"Time %3lld: PC=0x%04X instruction = 0x%04X (%s)",time,pcBefore,theInst,commandName);     
				if(strlen(regStr)>0){
					fprintf(stderr, "%s", regStr);
				}
				fprintf(stderr, "\n");
			}
		}
    }
}





//RETURNS 0 if success  -1 undefined error   -2 if file not found error    -6 if OBJG isnt at the begining
//-3 step isnt 3 when we get to end of a set      -4 premature EOF           -5 bytes read exceeded max bytes
int readObject(char* fileName){
    FILE* input=fopen(fileName,"r");
	if(input==NULL){
		return -2;
		//File not found error
	}
	char c;
	char objgCheck[4];
	int i;
	for(i=0; i<4; i++){
		if(c = getc(input)) != EOF){
			objgCheck[i]=c;
		}
		else{
			return -4;
		}
	}
	if(objgCheck[0]!='O' || objgCheck[1]!='B' || objgCheck[2]!='J' || objgCheck[3]!='G'){
		return -6;
	}
	//first part of pc
	char tempArr[2];
	c=getc(input);
	if(c == EOF){
		return -4;
	}
	tempArr[0]=c;
	//send part of pc
	c=getc(input);
	if(c == EOF){
		return -4;
	}
	tempArr[1]=c;
	pc=((int)tempArr[0]);
	pc=adding<<8;
	pc=adding & upperEight;
	pc+=((int)tempArr[1])&lowerEight;
	tempArr[0]=NULL;
	tempArr[1]=NULL;
	int step=1;
	int bytesRead=0;
	int currentAddress;
	int maxBytes;
	while((c = getc(input)) != EOF){
		if(bytesRead>=maxBytes){
			//hmm should we do more error checking
			//error checking
			if(step!=3){
				return -3;
			}
			if(bytesRead!=maxBytes){
				return -5;
			}
			step=1;
			bytesRead=-2;
			maxBytes=-1;
		}
		else if(step==1){
			maxBytes=(int)c;
			step++;
			bytesRead=1;
		}
		else if(step==2){
			tempArr[0]=c;
			c=getc(input);
			if(c == EOF){
				return -4;
			}
			tempArr[1]=c;
			//make sure this is correct.  im taking 2 characters from tempArr, combining them as ints
			currentAddress=((int)tempArr[0]);
			currentAddress=currentAddress<<8;
			currentAddress=currentAddress & upperEight;
			currentAddress+=((int)tempArr[1])&lowerEight;
			tempArr[0]=NULL;
			tempArr[1]=NULL;
			step++;
			bytesRead+=2;
		}
		else if(step==3){
			tempArr[0]=c;
			c=getc(input);
			if(c == EOF){
				return -4;
			}
			tempArr[1]=c;
			int adding;
			//same as above.  double check
			adding=((int)tempArr[0]);
			adding=adding<<8;
			adding=adding & upperEight;
			adding+=((int)tempArr[1])&lowerEight;
			mem[currentAddress]=adding;
			currentAddress++;
			bytesRead+=2;
			tempArr[0]=NULL;
			tempArr[1]=NULL;
		}
	}
	psw=1;
	return 0;
}



void printMem(){
	int i;
	printf("Starting printing memory:\n");
	for(i=0; i<65536; i++){
		if(mem[i]!=NULL){
			printf("Address: %03X       Data: %03X\n",i,mem[i]);
		}
	}
	printf("Finished printing memory:\n"); 
}

//return false if an error occurs true otherwise
int operate(int instruction){
    
	
    switch(instruction & typeBuffer){
    	
		//TYPE 1
		case typeOne:
		return doTypeOne(instruction);
		break;
		
		
		//ADD (type 2)
		case typeTwoAdd:
		
		break;
		
		//SUB (type 2)
		case typeTwoSub:
		
		break;
		
		//MUL (type2)
		case typeTwoMul:
		
		break;
		
		//DIV (type 2)
		case typeTwoDiv:
		
		break;
		
		//AND (type 2)
		case typeTwoAnd:
		
		break;
		
		//OR (type 2)
		case typeTwoOr:
		
		break;
		
		//XOR (type 2)
		case typeTwoXor:
		
		break;
		
		//LD (type 2)
		case typeTwoLd:
		
		break;
		
		//ST (type 2)
		case typeTwoSt:
		
		break;
		
		//TYPE 3
		case typeThree:
		doTypeThree(instruction);	
		break;
		
		//TYPE 4
		case typeFourA:
		doTypeFour(instruction);	
		break;
			
		//TYPE 4
		case typeFourB:
		doTypeFour(instruction);
		break;
		
		//TYPE 5
		case typeFive:
		doTypeFive(instruction);
		break;
		
		//TYPE 6
		case typeSix:
		doTypeSix(instruction);
		break;
		
		default:
		//error
		break;
		
    }
}


//NO memory   no register
int doTypeOne(int instruction){
	
	switch(instruction&oneWord){
		//NOP
		case 0:
		strcpy(commandName,"NOP");
		time++;
		pc++;
		break;
		
		//HLT
		case 1:
		printRegs(2,5,3,psw);
		if((psw&bitZero)==bitZero){
			psw--;
		}
		else{
			printf("for some reason we already decided to halt");
			return FALSE;
		}
		printRegs(3,psw,2,5);
		strcpy(commandName,"HLT");
		pc++;
		time++;
		break;
		
		//RET
		case 2:
		strcpy(commandName,"RET");
		printRegs(2,6,3,sp);
		if(sp==0xFFFF){
			printf("stack underflow error");
			return FALSE;
		}
		sp++;
		printRegs(3,sp,2,6);
		printRegs(1,sp,3,mem[sp]);
		pc=mem[sp];
		printRegs(3,pc,2,4);
		time+=2;
		break;
		
		//error
		default:
		printf("\nInvalid type 1 instruction\n");
		return FALSE;
		break;
	}
	return TRUE;
}


int doTypeTwo(int instruction){
    int currentPage=pc/256;
	currentPage=currentPage*256;
    //this turns D/I into a variable:   indirect
    int indirect=((instruction & bitNine)==bitNine);
   
    //this turns Z/C into a vaiable: onCurrPage
    int onCurrPage=((instruction&bitEight)==bitEight);
   
    //this sets the memory address we are dealing with
    int address = instruction&lowerEightPage;

	// Deal with Z/C
	if(onCurrPage==TRUE)
		address+=currentPage;
	
	// Deal with D/I
    if(indirect==TRUE){
        time++;
		printRegs(1, address, 3, mem[address]);
        address=mem[address];
    }

	char* regname[10];
	// Get a reference to the memory
	int& curr_mem = mem[address];

	// Get a reference to the register
	int regno = (instruction && twoRegBuffer)/0x800;
	int& curr_reg = reg[regno];

	regCode(regno, regname);

	int result;
	switch(instruction & typeBuffer){
		
		//ADD (type 2)
		case typeTwoAdd:
			sprintf(commandName, "ADD%s", regname);
			result = curr_reg + curr_mem;
		break;
	
		//SUB (type 2)
		case typeTwoSub:
			sprintf(commandName, "SUB%s", regname);
			int neg = overflowBit - curr_mem;	
			result = curr_reg + neg;
		break;			
		
		//MUL (type2)
		case typeTwoMul:
			sprintf(commandName, "MUL%s", regname);	
			result = curr_reg * curr_mem;
		break;
		
		//DIV (type 2)
		case typeTwoDiv:
			sprintf(commandName, "DIV%s", regname);	
			result = curr_reg / curr_mem;
		break;
		
		//AND (type 2)
		case typeTwoAnd:
			sprintf(commandName, "AND%s", regname);	
			result = curr_reg & curr_mem;
		break;
		
		//OR (type 2)
		case typeTwoOr:
			sprintf(commandName, "OR%s", regname);	
			result = curr_reg | curr_mem;		
		break;
		
		//XOR (type 2)
		case typeTwoXor:
			sprintf(commandName, "AND%s", regname);	
			result = curr_reg ^ curr_mem;
		break;
		
		//LD (type 2)
		case typeTwoLd:
			sprintf(commandName, "LD%s", regname);
			printRegs(1, address, 3, curr_mem);
			curr_reg = curr_mem;
			printRegs(3, curr_mem, 2, regno);
		break;
		
		//ST (type 2)
		case typeTwoSt:
			sprintf(commandName, "ST%s", regname);
			printRegs(2, regno, 3, curr_reg);
			curr_mem = curr_reg;		
			printRegs(2, curr_reg, 3, address);
		break;
	}

	// Extra handling for arithmetic ops
	if((instruction & typeBuffer) < typeTwoLd){

		// Print initial state
		printRegs(2, regno, 3, curr_reg);
		printRegs(1, address, 3, curr_mem);

		int overflow = (result>oneWord) // Check for overflow
		linkBit = linkBit ^ overflow; 	// Complement the link bit
			
		curr_reg = result & oneWord; 	// Truncate the result
			
		// Print final state
		if(overflow)
			printRegs(3, linkBit, 2, 8);
		printRegs(3, curr_reg, 2, regno);
	}		
}



//I/O
int doTypeThree(int instruction){
	
}


//yes memory    yes register
int doTypeFour(int instruction){
	
}


// yes register   yes register
int doTypeFive(int instruction){
	
}


//no memory     yes register
int doTypeSix(int instruction){
	
}

//do i need to malloc?
void regCode(int regNum, char* ret){
	if(regNum==0){
		strcpy(ret, "A");
	}
	else if(regNum==1){
		strcpy(ret, "B");
	}
	else if(regNum==2){
		strcpy(ret, "C");
	}
	else if(regNum==3){
		strcpy(ret, "D");
	}
	else if(regNum==4){
		strcpy(ret, "PC");
	}
	else if(regNum==5){
		strcpy(ret, "PSW");
	}
	else if(regNum==6){
		strcpy(ret, "SP");
	}
	else if(regNum==7){
		strcpy(ret, "SPL");
	}
	else if(regNum==8){
		strcpy(ret, "L");
	}
	else{
		strcpy(ret, "???");
	}
	return ret;
}


//type 1 is memory      //type 2 is register     //type 3 is value
void printRegs(int val1Type, int val1, int val2Type, int val2){
	if(strlen(regStr)>0){
		strcat(regStr,", ");
	}
	else{
		strcpy(regStr,": ");
	}
	char* temp[10];
	if(val1Type==1){
		strcat(regStr,"M[");
		sprintf(temp, "0x%04X", val1);
		strcat(regStr,temp);
		strcat(regStr,"]");
	}
	else if(val1Type==2){
		strcat(regStr,regCode(val1));
	}
	else if(val1Type==3){
		sprintf(temp, "0x%04X", val1);
		strcat(regStr,temp);
	}
	strcat(regStr," -> ");
	temp[0]='\0';
	if(val2Type==1){
		strcat(regStr,"M[");
		sprintf(temp, "0x%04X", val2);
		strcat(regStr,temp);
		strcat(regStr,"]");
	}
	else if(val2Type==2){
		regCode(val2, temp);
		strcat(regStr, temp);
	}
	else if(val2Type==3){
		sprintf(temp, "0x%04X", val2);
		strcat(regStr,temp);
	}
}



