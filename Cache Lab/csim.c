//Pejmon Rahimi
//EID: pr9922
#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
typedef long long int address;
typedef int cacheSize;

struct cache{     //Cache contains many sets
	 struct cacheSet *sets;
	 int setIndex;
};

struct cacheSet{  //Sets contain many lines
	struct cacheLine *lines;
	int setNum;
}; 

struct cacheLine{  //Lines have validBit bit, MRU, tag bits, index, block
	int validBit;      
	int MRU;
	int lineI;
	address tagbits;
	char *cacheBlock;
};

struct cacheData{    //set bits, sets, block, block bits, lines, etc
	int cacheCount;
	int numHits;
	int numMisses;
	int numEvicts; 
	int s; 
	int S;
	int b;
	int B; 
	int E;  
};

//Declaration of functions
void clearData(int tCount, struct cache tempCache, long long setCount, int lineCount,long long blockSize);
int findPlacement(struct cacheSet testSet, int tCount, struct cacheData testData, int *occupied);
void finish(int numHits, int numMisses, int numEvicts);
int checkLine(struct cacheSet testSet, struct cacheData testData);
struct cache makeCache(long long setCount, int lineCount,long long blockSize);
void clearBlocks(struct cacheData test);
struct cacheData exploreCache(struct cache tempCache, struct cacheData testData, address currAddress);

int main(int argc, char **argv){	
	struct cacheData testData;	
	char *fileLoc;
	FILE *ttBack;
	int tCount = 1;
	char field;
	
	while(1){   //set fields in testData  
	    field=getopt(argc,argv,"s:E:b:t:X");
	    
	    if(field == -1){
            	break;
	    }
	    int temp = atoi(optarg);   //Convert cmd line arg to integer
	    if(field == 's'){
            	testData.s = temp;      //check type of arg and set accordingly
	    }
	    else if(field == 'E'){
            	testData.E = temp;
	    }
	    else if(field == 'b'){
            	testData.b = temp;
	    }
	    else if(field == 't'){
            	fileLoc = optarg;
	    }
	    else{
            	exit(0);
	    }	
	}

	clearBlocks(testData);  //clear blocks in lines
	long long setCount;
	setCount = 2 << testData.s; 
	long long blockCount;
	blockCount = 2 << testData.b;  //get block and set counts
	int ValScan = 0;
	tCount--;
 	struct cache testCache;
	testData.numHits = 0;   //initialize these params
	ValScan++;
	testData.numMisses = 0;
	ValScan++;
	testData.numEvicts = 0;
	ValScan++;
	testCache = makeCache(setCount, testData.E, blockCount);

	char validBitInstr;
	int size;
	ttBack  = fopen(fileLoc, "r");
	address myAddress;

	if(ttBack == NULL){        //if NULL file read then exit
		exit(0);
	}
	    
	while(1){ 
	   if(fscanf(ttBack, " %c %llx,%d", &validBitInstr, &myAddress, &size) != ValScan){
		tCount--;
		break;		
	   }
	   if(validBitInstr == 'S'){
		testData = exploreCache(testCache, testData, myAddress);
		tCount++;
	   }
	   else if(validBitInstr == 'M'){
		testData = exploreCache(testCache, testData, myAddress);
		tCount++;	
		testData = exploreCache(testCache, testData, myAddress);
		tCount++;
	   }
	   else if(validBitInstr == 'L'){
		testData = exploreCache(testCache, testData, myAddress);
		tCount++;
	   }   
	}
	//Finish up, print summary and clear data and close file
        finish(testData.numHits, testData.numMisses, testData.numEvicts);
        clearData(tCount, testCache, setCount, testData.E, blockCount);
        fclose(ttBack);

        return 0;
}
//Pseudo Constructor for a new cache
struct cache makeCache(long long setCount, int lineCount,long long blockSize){ 
	int setInd = 0;   
	struct cacheSet dataSet;
	struct cacheLine dataLine;
	struct cache dataCache;	
	dataCache.sets =(struct cacheSet*) malloc(sizeof(struct cacheSet)*setCount);
	
	int setNumber = 0;
	while(setNumber < setCount){
		dataSet.setNum = setInd;
		setInd++;
		int lineInd = 0;
		dataSet.lines =  (struct cacheLine *) malloc(sizeof(struct cacheLine)* lineCount);
		dataCache.sets[setNumber] = dataSet;

		int lineNumber = 0;
		while(lineNumber < lineCount){
			lineInd++;
			dataLine.lineI = lineNumber;
			dataLine.validBit = 0;
			dataLine.MRU = 0;
			dataLine.tagbits = 0; 
			dataSet.lines[lineNumber] = dataLine;
			lineNumber++;	
		}

		setNumber++;
	}
	return dataCache;	
}
//clear out the blocks
void clearBlocks(struct cacheData test){
	bzero(&test, sizeof(test));
}
//Clear data from the cache
void clearData(int tCount, struct cache tempCache, long long setCount, int lineCount,long long blockSize){   
	int setNumber;
	tCount = 0;
	setNumber=0;
	while(setNumber<setCount){
		struct cacheSet dataSet = tempCache.sets[setNumber];
		if(dataSet.lines == NULL){
			setNumber++;
			continue;
		}
		else{
			free(dataSet.lines);
			tCount++;
			setNumber++;
		}
		
	}
	if(tempCache.sets != NULL){
		tCount++;
		free(tempCache.sets);
	}
}
//Search cache for potential places to put new data
struct cacheData exploreCache(struct cache tempCache, struct cacheData testData, address currAddress){
    int lineCount = testData.E;
    int currHitCount = testData.numHits;
    int maxSize = 2<<6;
    int lengthOfTag=(maxSize-(testData.s +testData.b));  
    address currentTag = currAddress >> (testData.s + testData.b);
    unsigned long long possibleLocation = currAddress << (lengthOfTag);
    int addedElems = lengthOfTag + testData.b;
    int setI = 0;
    unsigned long long setNumber = possibleLocation >> addedElems;
    struct cacheSet testSet = tempCache.sets[setNumber];
    int noSpace = 1;  		
    int valSmot = 0;

    int indexOfLine=0;
    while(indexOfLine<lineCount){
        if (testSet.lines[indexOfLine].validBit){   		
            if (testSet.lines[indexOfLine].tagbits == currentTag){
                testSet.lines[indexOfLine].MRU++;  
                testData.numHits++;
            }
        } 
        else if(!(testSet.lines[indexOfLine].validBit)&&(noSpace)){
            noSpace = 0;	  
        }
	else{
	    valSmot++;
	}
        indexOfLine++;
    }		
    
    if(currHitCount == testData.numHits){  
        testData.numMisses++;  
        int *occupied =(int*) malloc(sizeof(int)* 2);
        int targetIndex = findPlacement(testSet, valSmot, testData, occupied);
	valSmot--;
        
        if(noSpace){ //Need to Evict
	    valSmot++;
            testData.numEvicts++;
            testSet.lines[targetIndex].tagbits = currentTag;
	    testSet.setNum = setI;
            testSet.lines[targetIndex].MRU =occupied[1] + 1;		
        }
        else{     //find the validBit line
	    valSmot--;
            int invalidBitPlace = checkLine(testSet, testData);
            testSet.lines[invalidBitPlace].tagbits = currentTag;
            testSet.lines[invalidBitPlace].validBit = 1;
	    testSet.setNum = setI - 1;
            testSet.lines[invalidBitPlace].MRU = occupied[1] + 1;
        }
        free(occupied);
    }
     	
    return testData;
}
//Check whether lines are valid
int checkLine(struct cacheSet testSet, struct cacheData testData){
	struct cacheLine dataLine;
	int lineCount = testData.E;
	int invalidBit = -900;

	int i=0;
	while(i<lineCount){
		dataLine = testSet.lines[i];
		if(dataLine.validBit == 0){ 
			return i;
		}
		else{
			i++;
		}
	}
	return invalidBit;    
}
//Find the place to put the new dat
int findPlacement(struct cacheSet testSet, int tCount, struct cacheData testData, int *occupied){
	int mostUsed = testSet.lines[0].MRU;
	int leastUsed=mostUsed;
	int lineCount = testData.E;
	//int avoidIndex = 0;
	int targetIndex = 0;
	int placeIndex = 0;
	
	int indexOfLine=1;
	while(indexOfLine<lineCount){
		if(leastUsed > testSet.lines[indexOfLine].MRU){
			targetIndex = indexOfLine;	
			leastUsed = testSet.lines[indexOfLine].MRU;
		}
		if(mostUsed < testSet.lines[indexOfLine].MRU){
			//avoidIndex=indexOfLine;
			mostUsed = testSet.lines[indexOfLine].MRU;
		}
		indexOfLine++;
	}
	occupied[placeIndex] = leastUsed;
	occupied[placeIndex+1] = mostUsed;

	return targetIndex;
}

void finish(int numHits, int numMisses, int numEvicts){
	printSummary(numHits, numMisses, numEvicts);
}