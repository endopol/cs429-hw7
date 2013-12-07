
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TRUE 1
#define FALSE 0

#define WORD_COUNT_BITS 7               // 7 bits specify word on page
#define PAGE_SIZE (1<<WORD_COUNT_BITS)  // 128=(2^WORD_COUNT_BITS) words per page
#define PAGE_COUNT_BITS 5               // 5 bits specify page
#define PAGE_MASK ((1<<PAGE_COUNT_BITS)-1)		//<<WORD_COUNT_BITS)

#define instructLength 12

#define hexSize 4

#define signMask 0x7FF

#define oneWordBuffer 0xFFF
#define opcodeBuffer 0xE00
#define dIBuffer 0x100
#define zeroCurrentBuffer 0x80
#define addressBuffer 0x7F

#define deviceBuffer 0x3F
#define functionBuffer 0x7


#define opZero 0x000
#define opOne 0x200
#define opTwo 0x400
#define opThree 0x600
#define opFour 0x800
#define opFive 0xA00
#define opSix 0xC00
#define opSeven 0xE00


#define groupBit 0x100
#define bit7 0x80
#define bit6 0x40
#define bit5 0x20
#define bit4 0x10
#define bit3 0x8
#define bit2 0x4
#define bit1 0x2
#define bit0 0x1
#define bit11 0x800


#define overflowBit 0x10000

int rega;
short linkBit;
int pc;
int mem[4096];
short verbose;
int time;






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
				printf("\nSize of string error\n");
				break;
			case -4:
				printf("\nInvalid hex character error\n");
				break;
		}
		short keepOperating=TRUE;
		int pcBefore;
		char* commandName[WORD_COUNT_BITS*3];
		while(keepOperating==TRUE){
			commandName[0]='\0';
			pc=pc&oneWordBuffer;
			pcBefore=pc;
			keepOperating=operate(mem[pc],commandName);
			if(verbose==TRUE){
				fprintf(stderr,"Time %lld: PC=0x%03X instruction = 0x%03X (%s), rA = 0x%03X, rL = %d\n",time,pcBefore,mem[pcBefore],commandName,rega,linkBit);
			}
		}
    }
}



void printMem(){
	int i;
	printf("Starting printing memory:\n");
	for(i=0; i<4096; i++){
		if(mem[i]!=NULL){
			printf("Address: %03X       Data: %03X\n",i,mem[i]);
		}
	}
	printf("Finished printing memory:\n"); 
}


//RETURNS 0 if success     -2 if file not found error    
//-3 is an error of size of string     -4 is error in which we had an invalid hex character
int readObject(char* fileName){
    FILE* input=fopen(fileName,"r");
	if(input==NULL){
		return -2;
		//File not found error
	}
	int step=1;
	int i=0;
	int intAddress;
	int intData;
	short spaceNext;
	char strAddress[5];
	char strData[5];
	char c;
	while((c = getc(input)) != EOF){
		if(c=='\n'){
			if(step==2){
				intData=hexStringInt(strData);
				//error checking
				if(intData==-1){
					return -3;
				}
				if(intData==-2){
					return -4;
				}
				//done error checking
				if(strcmp(strAddress,"EP")==0){
					pc=intData;
				}
				else{
					intAddress=hexStringInt(strAddress);
					//error checking
					if(intAddress==-1){
						return -3;
					}
					if(intAddress==-2){
						return -4;
					}
					//done error checking
					mem[intAddress]=intData;
				}
			}
			strAddress[0]='\0';
			strData[0]='\0';
			intAddress=0;
			intData=0;
			step=1;
			i=0;
			spaceNext=FALSE;
		}
		else if(step==1){
			if(c==':'){
				i=0;
				step++;
				spaceNext=TRUE;
			}
			else{
				strAddress[i]=(char)c;
				i++;
			}
		}
		else if(step==2){
			if(spaceNext==TRUE){
				spaceNext=FALSE;
			}
			else{
				strData[i]=(char)c;
				i++;
			}
		}
	}
	return 0;
}

//-1 is an error of size of string         -2 is error in which we had an invalid hex character
int hexStringInt(char* hex){
	int ret=0;
	if(strlen(hex)!=(instructLength/hexSize)){
		return -1;
	}
	int index=0;
	while(index<strlen(hex)){
		int a=(int)hex[index];
		if(a>=65 && a<=70){
			ret+=(a-55)<<(hexSize*((instructLength/hexSize)-1-index));
			index++;
		}
		else if(a>=48 && a<=57){
			ret+=(a-48)<<(hexSize*((instructLength/hexSize)-1-index));
			index++;
		}
		else{
			return -2;
		}
	}
	return ret;
}




//returns FALSE if its a HALT command        TRUE otherwise
int operate(int instruction, char* commandName){
    int currentPage=pc/128;
	currentPage=currentPage*128;
	int device;
    //this turns D/I into a variable:   indirect
    short indirect=FALSE;
    if((instruction & dIBuffer)==dIBuffer){
        indirect=TRUE;
    }
	
    //this turns Z/C into a vaiable: onCurrPage
    int onCurrPage=FALSE;
    if((instruction&zeroCurrentBuffer)==zeroCurrentBuffer){
        onCurrPage=TRUE;
    }
    
    //this sets the memory address we are dealing with
    int address;
    if(indirect==TRUE && ((instruction & opcodeBuffer)<opSix)){
        time++;
        if(onCurrPage==TRUE){
            //indirect addressing on the current page
            address=mem[currentPage+(instruction&addressBuffer)];
        }
        else{
            //indirect addressing on page zero
            address=mem[(instruction&addressBuffer)];
        }
    }
    else{
        if(onCurrPage==TRUE){
            //direct addressing on the current page
            address=currentPage+(instruction&addressBuffer);
        }
        else{
            //direct addressing on page zero
            address=instruction&addressBuffer;
        }
    }
    switch(instruction & opcodeBuffer ){
        //AND
        case opZero:
		rega=(rega & mem[address]);
		rega=rega&oneWordBuffer;
		strcpy(commandName,"AND");
		pc++;
		time+=2;
		break;

	//TAD    
	case opOne:{
		int regaSign=((rega>>(instructLength-1))&bit0);
		int value=(mem[address])&oneWordBuffer;
		int valSign = (value>>(instructLength-1))&bit0;
		int x=(rega&signMask);
		x+=(value&signMask);
		int resSign=(x>>(instructLength-1))&bit0;
		rega+=value;
		rega=rega&oneWordBuffer;
		int numCarry=regaSign+valSign+resSign;
		if(numCarry>1){
			linkBit=(~linkBit)&bit0;
		}
		strcpy(commandName,"TAD");
		pc++;
		time+=2;
		break;
		}	

	
        case opTwo:
		//weird way of doing this?
		if(mem[address]==oneWordBuffer || mem[address]==signMask){
			pc++;
			mem[address]=0;
		}
		else{
			mem[address]=(mem[address]+1) & oneWordBuffer;
		}
            pc++;
	    strcpy(commandName,"ISZ");
            time+=2;
		break;
        


        //DCA    
        case opThree:
            mem[address]=rega;
            rega=0;
            pc++;
            time+=2;
	    strcpy(commandName,"DCA");
		break;
        
        //JMS    
        case opFour:
            mem[address]=pc+1;
            pc=address+1;
            time+=2;
		strcpy(commandName,"JMS");
		break;
            
        //JMP    
        case opFive:
            pc=address;
            time++;
		strcpy(commandName,"JMP");
		break;
            
        //IOT    
        case opSix:
            device=((instruction>>3)&deviceBuffer);
            if(device==3){
                char myChar=getc(stdin);
                rega=(int)myChar;
		rega=rega&oneWordBuffer;
                pc++;
                time++;
		strcpy(commandName,"IOT 3");
            }
            else if(device==4){
                char out=(char)(rega & 0xFF);
                //putchar(out);
		printf("%c",out);
                pc++;
                time++;
				strcpy(commandName,"IOT 4");
            }
            else{
		strcpy(commandName,"HLT");
		printf("error invalid operation in IOT command");
                return FALSE;
            }
		break;
        
        //Lots of them!    
        case opSeven:
			return groups(instruction,commandName);
			
			
        //error    
        default:
			printf("Error reaching default in switch statement");
			return FALSE;
            
    }
	if(indirect==TRUE && (instruction & opcodeBuffer)<opSix){
		strcat(commandName," I");
	}
	return TRUE;
}


//return FALSE if HLT or fail     TRUE otherwise
int groups(int instruction, char* commandName){
	short skipn=-1;
	short skipz=-1;
	short skipl=-1;
	short flip=FALSE;
	//I feel like I am doing the skip bit functions wrong.  maybe not though
    if((instruction&groupBit)==groupBit){
    	//group2
		//might want to error check by outputting error message instead of HLT
		if((instruction & bit0)==bit0){
			strcpy(commandName,"HLT");
			time++;
			printf("error invalid group 2 set");
			return FALSE;
		}
		
		//SMA
		if((instruction & bit6)==bit6){
			if(strlen(commandName)>0){
				strcat(commandName," ");
			}
			strcat(commandName,"SMA");
			if((rega&bit11)==bit11){
				skipn=TRUE;
			}
			else{
				skipn=FALSE;
			}
		}
		
		//SZA
		if((instruction & bit5)==bit5){
			if(strlen(commandName)>0){
				strcat(commandName," ");
			}
			strcat(commandName,"SZA");
			if(rega==0){
				skipz=TRUE;
			}
			else if(rega!=0){
				skipz=FALSE;
			}
		}
		
		//SNL
		if((instruction & bit4)==bit4){
			if(strlen(commandName)>0){
				strcat(commandName," ");
			}
			strcat(commandName,"SNL");
			if(linkBit==1){
				skipl=TRUE;
			}
			else if(linkBit==0){
				skipl=FALSE;
			}
		}
		
		//RSS
		if((instruction & bit3)==bit3){
			if(strlen(commandName)>0){
				strcat(commandName," ");
			}
			strcat(commandName,"RSS");
			flip=TRUE;
		}
		
		
		//CLA
		if((instruction & bit7)==bit7){
			rega=0;
			if(strlen(commandName)>0){
				strcat(commandName," ");
			}
			strcat(commandName,"CLA");
		}
		
		//OSR  but treat like NOP
		if((instruction & bit2)==bit2){
			if(strlen(commandName)>0){
				strcat(commandName," ");
			}
			strcat(commandName,"OSR");
		}
		
		//HLT
		if((instruction & bit1)==bit1){
			if(strlen(commandName)>0){
				strcat(commandName," ");
			}
			strcat(commandName,"HLT");
			time++;
			return FALSE;
		}
    }
	else{
		//group1
		if(((instruction & bit3)==bit3) & ((instruction & bit2)==bit2)){
			strcat(commandName,"HLT");
			time++;
			printf("error invalid group 1 set");
			return FALSE;
		}
		
		//CLA
		if((instruction & bit7)==bit7){
			rega=0;
			if(strlen(commandName)>0){
				strcat(commandName," ");
			}
			strcat(commandName,"CLA");
		}
		
		//CLL
		if((instruction & bit6)==bit6){
			linkBit=0;
			if(strlen(commandName)>0){
				strcat(commandName," ");
			}
			strcat(commandName,"CLL");
		}
		
		//CMA
		if((instruction & bit5)==bit5){
			rega=~rega;
			rega=rega&oneWordBuffer;
			if(strlen(commandName)>0){
				strcat(commandName," ");
			}
			strcat(commandName,"CMA");
			
		}
		
		//CML
		if((instruction & bit4)==bit4){
			linkBit=(~linkBit);
			linkBit=linkBit&bit0;
			if(strlen(commandName)>0){
				strcat(commandName," ");
			}
			strcat(commandName,"CML");
		}
		
		//IAC
		if((instruction & bit0)==bit0){
			int regaSign = (rega>>(instructLength-1))&bit0;//get rega sign
			int resSign=0;
			if((rega&signMask)==signMask){
				resSign=1;
			}
			rega++;
			rega=rega&oneWordBuffer;
			if(regaSign==1 && resSign==1){
				//right way to compliment link bit?
				linkBit=(~linkBit)&bit0;
			}
			if(strlen(commandName)>0){
				strcat(commandName," ");
			}
			strcat(commandName,"IAC");
		}
		
	//RAR
        if((instruction & bit3)==bit3){
            short rotTwo=FALSE;
            if((instruction & bit1)==bit1){
                short rotTwo=TRUE;
		if(strlen(commandName)>0){
			strcat(commandName," ");
		}
                strcat(commandName,"RTR");
                rotateRight();
                rotateRight();
            }
            else{
		if(strlen(commandName)>0){
			strcat(commandName," ");
		}
                strcat(commandName,"RAR");
                rotateRight();
            }
        }
        
        //RAL
        if((instruction & bit2)==bit2){
            short rotTwo=FALSE;
            if((instruction & bit1)==bit1){
                short rotTwo=TRUE;
		if(strlen(commandName)>0){
			strcat(commandName," ");
		}
                strcat(commandName,"RTL");
                rotateLeft();
                rotateLeft();
            }
            else{
		if(strlen(commandName)>0){
			strcat(commandName," ");
		}
                strcat(commandName,"RAL");
                rotateLeft();
            }
        }
	}
	time++;
	pc++;
	if((flip==TRUE) && (skipn==FALSE || skipz==FALSE || skipl==FALSE)){
		pc++;
	}
	else if((flip==FALSE) && (skipn==TRUE || skipz==TRUE || skipl==TRUE)){
		pc++;
	}
	if(strlen(commandName)==0){
		strcpy(commandName,"NOP");
	}
	return TRUE;
}




	void rotateLeft(){
            short temp=rega&bit11;
            rega=(rega<<1)&oneWordBuffer;
            rega+=linkBit&bit0;
            linkBit=temp;
        }
        
        
       
        
       void rotateRight(){
            short temp=rega&bit0;
	    rega=rega&oneWordBuffer;
            rega=rega>>1;
            rega+=(linkBit&bit0)<<(instructLength-1);
            linkBit=temp;
        }










