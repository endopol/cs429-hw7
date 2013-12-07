#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TRUE 1
#define FALSE 0

#define typeBuffer 0xF000

#define upperEight 0xFF00
#define lowerEight 0xFF
#define oneWord 0xFFFF
#define theSignBit 0x8000
#define overflowBit 0x10000

#define maxSigned = 0x7FFF
#define minSigned = -0x8000

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
#define regK 0x0007 // i.e. ...000000111
#define regJ 0x0038 // i.e. ...000111000
#define regI 0x01C0 // i.e. ...111000000
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
char regStr[300];
char commandName[300];

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
	if(input==0){
		return -2;
		//File not found error
	}
	char c;
	char objgCheck[4];
	int i;
	for(i=0; i<4; i++){
		if(c = getc(input) != EOF){
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
	pc=pc<<8;
	pc=pc & upperEight;
	pc+=((int)tempArr[1])&lowerEight;
	tempArr[0]=0;
	tempArr[1]=0;
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
			tempArr[0]=0;
			tempArr[1]=0;
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
			tempArr[0]=0;
			tempArr[1]=0;
		}
	}
	psw=1;
	return 0;
}



void printMem(){
	int i;
	printf("Starting printing memory:\n");
	for(i=0; i<65536; i++){
		if(mem[i]!=0){
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
		return doTypeTwo(instruction);
		break;
		
		//SUB (type 2)
		case typeTwoSub:
		return doTypeTwo(instruction);
		break;
		
		//MUL (type2)
		case typeTwoMul:
		return doTypeTwo(instruction);
		break;
		
		//DIV (type 2)
		case typeTwoDiv:
		return doTypeTwo(instruction);
		break;
		
		//AND (type 2)
		case typeTwoAnd:
		return doTypeTwo(instruction);
		break;
		
		//OR (type 2)
		case typeTwoOr:
		return doTypeTwo(instruction);
		break;
		
		//XOR (type 2)
		case typeTwoXor:
		return doTypeTwo(instruction);
		break;
		
		//LD (type 2)
		case typeTwoLd:
		return doTypeTwo(instruction);
		break;
		
		//ST (type 2)
		case typeTwoSt:
		return doTypeTwo(instruction);
		break;
		
		//TYPE 3
		case typeThree:
		return doTypeThree(instruction);	
		break;
		
		//TYPE 4
		case typeFourA:
		return doTypeFour(instruction);	
		break;
			
		//TYPE 4
		case typeFourB:
		return doTypeFour(instruction);
		break;
		
		//TYPE 5
		case typeFive:
		return doTypeFive(instruction);
		break;
		
		//TYPE 6
		case typeSix:
		return doTypeSix(instruction);
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

int cast_up(int x){
	int X = (x & oneWord);	
	X = -(overflowBit-X);
	return X;
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

	// Get a reference to the memory
	int* curr_mem = &(mem[address]);

	// Get a reference to the register
	int regno = (instruction & twoRegBuffer)/0x800;
	int* curr_reg = &(reg[regno]);

	char regname[10];
	regCode(regno, regname);

	int result;
	*curr_mem = cast_up(*curr_mem);
	*curr_reg = cast_up(*curr_reg);

	switch(instruction & typeBuffer){
		
		//ADD (type 2)
		case typeTwoAdd:
			sprintf(commandName, "ADD%s", regname);
			result = *curr_reg + *curr_mem;
		break;
	
		//SUB (type 2)
		case typeTwoSub:
			sprintf(commandName, "SUB%s", regname);
			result = *curr_reg - *curr_mem;	
		break;			
		
		//MUL (type2)
		case typeTwoMul:
			sprintf(commandName, "MUL%s", regname);	
			result = (*curr_reg) * (*curr_mem);
		break;
		
		//DIV (type 2)
		case typeTwoDiv:
			sprintf(commandName, "DIV%s", regname);
			
			if(*curr_mem==0)
				result = overflowBit;
			else
				result = (*curr_reg) / (*curr_mem);
		break;
		
		//AND (type 2)
		case typeTwoAnd:
			sprintf(commandName, "AND%s", regname);	
			result = (*curr_reg) & (*curr_mem);
		break;
		
		//OR (type 2)
		case typeTwoOr:
			sprintf(commandName, "OR%s", regname);	
			result = (*curr_reg) | (*curr_mem);		
		break;
		
		//XOR (type 2)
		case typeTwoXor:
			sprintf(commandName, "XOR%s", regname);	
			result = (*curr_reg) ^ (*curr_mem);
		break;
		
		//LD (type 2)
		case typeTwoLd:
			sprintf(commandName, "LD%s", regname);
			printRegs(1, address, 3, *curr_mem);
			*curr_reg = *curr_mem;
			printRegs(3, *curr_mem, 2, regno);
		break;
		
		//ST (type 2)
		case typeTwoSt:
			sprintf(commandName, "ST%s", regname);
			printRegs(2, regno, 3, *curr_reg);
			*curr_mem = *curr_reg;		
			printRegs(3, *curr_reg, 1, address);
		break;
	}

	// Extra handling for arithmetic ops
	if((instruction & typeBuffer) < typeTwoLd){
		// Print initial state
		printRegs(2, regno, 3, *curr_reg);
		printRegs(1, address, 3, *curr_mem);

		int overflow = ((result > maxSigned) || (result < minSigned));
				
		linkBit = linkBit ^ overflow; 	// Complement the link bit
			
		*curr_reg = result & oneWord; 	// Truncate the result
		*curr_mem &= oneWord;
			
		// Print final state
		if(overflow)
			printRegs(3, linkBit, 2, 8);
		printRegs(3, *curr_reg, 2, regno);
	}
}



//I/O
int doTypeThree(int instruction){
	int regno = (instruction & twoRegBuffer)/0x800;
	int* curr_reg = &(reg[regno]);
    int device=(instruction&deviceBuffer);
	//device is 3
    if(device==deviceThree){
        char myChar=getc(stdin);
		if(myChar==EOF){
			*curr_reg=oneWord;
			printRegs(3,oneWord,2,regno);
		}
		else{
	        *curr_reg=(int)myChar;
			*curr_reg=*curr_reg&oneWord;
			printRegs(3,*curr_reg,2,regno);
		}
        pc++;
        time++;
		strcpy(commandName,"IOT 3");
    }
	//device is 4
    else if(device==deviceFour){
		int myInt=*curr_reg & 0xFF;
        char myOut=(char)(*curr_reg & 0xFF);
        putchar(myOut);
		//printf("%c",out);
		printRegs(2,regno,3,myInt);
        pc++;
        time++;
		strcpy(commandName,"IOT 4");
    }
    else{
		printf("error invalid operation in IOT command");
        return FALSE;
    }
	return TRUE;
}


//yes memory    no register
int doTypeFour(int instruction){	
    //this turns Z/C into a vaiable: onCurrPage
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

	int *curr_mem = &mem[address];

	switch(instruction&upperSixOpcode){
		
		//ISZ
		case typeFourISZ:
			printRegs(1, address, 3, *curr_mem);   
			*curr_mem += 1;
			printRegs(3, *curr_mem, 1, address);
			
			if(((*curr_mem)&oneWord)==0){
				pc += 1;
				printRegs(3, pc, 1, 4);
			}

			time+=3;
		break;
		
		
		//JMP
		case typeFourJMP:
		time++;
		pc=address;
		printRegs(3,address,2,4);
		if(indirect==TRUE){
			strcpy(commandName,"I JMP");
		}
		else{
			strcpy(commandName,"JMP");
		}
		break;
		
		
		//CALL
		case typeFourCALL:
		if(sp<spl){
			printf("stack overflow error\n");
			return FALSE;
		}
		time+=2;
		printRegs(3,(pc+1),1,sp);
		mem[sp]=pc+1;
		sp--;
		printRegs(3,sp,2,6);
		pc=address;
		printRegs(3,address,2,4);
		if(indirect==TRUE){
			strcpy(commandName,"I CALL");
		}
		else{
			strcpy(commandName,"CALL");
		}
		break;
		
		
		//PUSH
		case typeFourPUSH:
		if(sp<spl){
			printf("stack overflow error\n");
			return FALSE;
		}
		time+=3;
		printRegs(1,address,3,mem[address]);
		mem[sp]=mem[address];
		printRegs(3,mem[address],1,sp);
		sp--;
		printRegs(3,sp,2,6);
		pc++;
		if(indirect==TRUE){
			strcpy(commandName,"I PUSH");
		}
		else{
			strcpy(commandName,"PUSH");
		}
		break;
		
		
		//POP
		case typeFourPOP:
		if(sp==oneWord){
			printf("stack underflow error\n");
			return FALSE;
		}
		time+=3;
		printRegs(2,6,3,sp);
		sp++;
		printRegs(3,sp,2,6);
		printRegs(1,sp,3,mem[sp]);
		mem[address]=mem[sp];
		printRegs(3,mem[sp],1,address);
		pc++;
		if(indirect==TRUE){
			strcpy(commandName,"I POP");
		}
		else{
			strcpy(commandName,"POP");
		}
		break;
	}
	return TRUE;
}

int* reg_by_num(int regnum){
	if(regnum<=4) // first four registers
		return &(reg[regnum]);

	if(regnum==4)
		return &pc;

	if(regnum==5)
		return &psw;

	if(regnum==6)
		return &sp;

	if(regnum==7)
		return &spl;
}

// yes register   yes register
int doTypeFive(int instruction){
	int reg_k = instruction & regK;
	int reg_j = (instruction & regJ)/8;
	int reg_i = (instruction & regI)/64;

	int* curr_i = reg_by_num(reg_i);
	int curr_j = *(reg_by_num(reg_j));
	int curr_k = *(reg_by_num(reg_k));

	curr_j = cast_up(curr_j);
	curr_k = cast_up(curr_k);

	int result;
	switch(instruction&typeFiveSubOpcode){
		//MOD (type 5)
		case typeFiveMOD:
			strcpy(commandName, "MOD");

			if(curr_k==0)
				result = overflowBit;
			else
				result = curr_j % curr_k;
		break;

		//ADD (type 5)
		case typeFiveADD:
			strcpy(commandName, "ADD");
			result = curr_j + curr_k;
		break;
	
		//SUB (type 5)
		case typeFiveSUB:
			strcpy(commandName, "SUB");	
			result = curr_j - curr_k;
		break;			
		
		//MUL (type 5)
		case typeFiveMUL:
			strcpy(commandName, "MUL");
			result = curr_j * curr_k;
		break;
		
		//DIV (type 5)
		case typeFiveDIV:
			strcpy(commandName, "DIV");			
			if(curr_k==0)
				result = overflowBit;
			else
				result = curr_j / curr_k;
		break;
		
		//AND (type 5)
		case typeFiveAND:
			strcpy(commandName, "AND");
			result = curr_j & curr_k;
		break;
		
		//OR (type 5)
		case typeFiveOR:
			strcpy(commandName, "OR");
			result = curr_j | curr_k;		
		break;
		
		//XOR (type 5)
		case typeFiveXOR:
			strcpy(commandName, "XOR");
			result = curr_j ^ curr_k;
		break;
	}

	// Print initial state
	printRegs(2, reg_j, 3, curr_j);
	printRegs(2, reg_k, 3, curr_k);

	int overflow = (result>maxSigned || result<minSigned); // Check for overflow
	linkBit = linkBit ^ overflow; 	// Complement the link bit
			
	*curr_i = result & oneWord; 	// Truncate the result
			
	// Print final state
	if(overflow)
		printRegs(3, linkBit, 2, 8);
	printRegs(3, *curr_i, 2, reg_i);
}


//no memory     yes register
int doTypeSix(int instruction){
	int regno = (instruction & twoRegBuffer)/0x800;
	int* curr_reg = &(reg[regno]);
	*curr_reg = cast_up(*curr_reg);

	char* regname[10];
	regCode(regno, regname);
	int skipSM=-1;
	int skipSZ=-1;
	int skipSNL=-1;
	int flip=FALSE;
	
	//sm*
	if(instruction&bitNine==bitNine){
		if(strlen(commandName)>0){
			strcat(commandName," ");
		}
		strcat(commandName,"SM");
		strcat(commandName,regname);
		if((*curr_reg & theSignBit)==theSignBit){
			skipSM=TRUE;
		}
		else{
			skipSM=FALSE;
		}
		printRegs(2,regno,3,*curr_reg);
	}
	
	//sz*
	if(instruction&bitEight==bitEight){
		if(strlen(commandName)>0){
			strcat(commandName," ");
		}
		strcat(commandName,"SZ");
		strcat(commandName,regname);
		if(*curr_reg==0){
			skipSZ=TRUE;
		}
		else{
			skipSZ=FALSE;
		}
		printRegs(2,regno,3,*curr_reg);
	}
	
	//snl
	if(instruction&bitSeven==bitSeven){
		if(strlen(commandName)>0){
			strcat(commandName," ");
		}
		if(linkBit!=0){
			skipSNL=TRUE;
		}
		else{
			skipSNL=FALSE;
		}
		strcat(commandName,"SNL");
		printRegs(2,8,3,linkBit);
	}
	
	//rss
	if(instruction&bitSix==bitSix){
		if(strlen(commandName)>0){
			strcat(commandName," ");
		}
		strcat(commandName,"RSS");
		flip=TRUE;
	}
	
	//cl*
	if(instruction&bitFive==bitFive){
		if(strlen(commandName)>0){
			strcat(commandName," ");
		}
		strcat(commandName,"CL");
		strcat(commandName,regname);
		*curr_reg=0;
		printRegs(3,*curr_reg,2,regno);
	}
	
	//cll
	if(instruction&bitFour==bitFour){
		if(strlen(commandName)>0){
			strcat(commandName," ");
		}
		strcat(commandName,"CLL");
		linkBit=0;
		printRegs(3,linkBit,2,8);
	}
	
	//cm*
	if(instruction&bitThree==bitThree){
		if(strlen(commandName)>0){
			strcat(commandName," ");
		}
		strcat(commandName,"CM");
		strcat(commandName,regname);
		printRegs(2,regno,3,*curr_reg);
		*curr_reg=~*curr_reg;
		*curr_reg=*curr_reg&oneWord;
		printRegs(3,*curr_reg,2,regno);
	}
	
	//cml
	if(instruction&bitTwo==bitTwo){
		if(strlen(commandName)>0){
			strcat(commandName," ");
		}
		strcat(commandName,"CML");
		printRegs(2,8,3,linkBit);
		linkBit^=1;
		printRegs(3,linkBit,2,8);
	}
	
	//dc*
	if(instruction&bitOne==bitOne){
		if(strlen(commandName)>0){
			strcat(commandName," ");
		}
		strcat(commandName,"DC");
		strcat(commandName,regname);
		printRegs(2,regno,3,*curr_reg);
		
		*curr_reg -=1;
		if(curr_reg<minSigned)
			linkBit ^= 1;
		printRegs(3, linkBit, 2, 8)
		
		printRegs(3,*curr_reg,2,regno);
	}
	
	//ic*
	if(instruction&bitZero==bitZero){
		if(strlen(commandName)>0){
			strcat(commandName," ");
		}
		strcat(commandName,"IC");
		strcat(commandName,regname);
		printRegs(2,regno,3,*curr_reg);
	
		*curr_reg +=1;
		if(*curr_reg>maxSigned)
			linkBit ^= 1;
		printRegs(3, linkBit, 2, 8);
		
		printRegs(3,*curr_reg,2,regno);
	}
	
	time++;
	pc++;
	if((flip==TRUE) && (skipSM==FALSE || skipSZ==FALSE || skipSNL==FALSE)){
		pc++;
		printRegs(3,pc,2,4);
	}
	else if((flip==FALSE) && (skipSM==TRUE || skipSZ==TRUE || skipSNL==TRUE)){
		pc++;
		printRegs(3,pc,2,4);
	}
	if(strlen(commandName)==0){
		strcpy(commandName,"NOP");
	}
	return TRUE;
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
	char temp[10];
	if(val1Type==1){
		strcat(regStr,"M[");
		sprintf(temp, "0x%04X", val1);
		strcat(regStr,temp);
		strcat(regStr,"]");
	}
	else if(val1Type==2){
		regCode(val1, temp);
		strcat(regStr,temp);
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



