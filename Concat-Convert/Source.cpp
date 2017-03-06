#include <windows.h>
#include <iostream>
#include <fstream>

#include <locale>
#include <cstdlib>

#include <stdlib.h>
#include <stdio.h>
#include <cstdint>

#include <string>
#include <vector>
#include <algorithm>

#include "acqfile.h"

#include <wchar.h>


/*
	README:

	Running this script will concatenate all ACQ files in the current folder into DCL files (which are labeled by animal). 

*/







/*
	Obtains the current path of the executable, minus the executable name. 

	NOTE: This function is not complete -- still needs to accept input 
	(ie. argv[0], which GENERALLY contains the full path to the executable. Seems this is not a guarantee!) 
	Will not be using this function because of the uncertainty surrounding argv[0]

	Returns:
		a string containing the path to the currently-executing program. 
*/
//std::string ExePath() {
//    char buffer[MAX_PATH];
//    GetModuleFileNameA( NULL, buffer, MAX_PATH );
//    std::string::size_type pos = std::string( buffer ).find_last_of( "\\/" );
//    return std::string( buffer ).substr( 0, pos);
//}




/*
	Compiles a list of ACQ files in the current directory (full name), 
	sorted in alphabetical order. 
	
	Because files are named according to the date of their creation, 
	alphabetical order = chronological order.
	EDIT 3-6-2017: NOT ALWAYS TRUE. Work with Windows' "Last Modified" time. 

	NOTE: When using this function, error occurs at return! Not sure why...
	Both listACQFilesDir() and listACQFiles() do (and should) have the same output for a given path. 
	
	It is unclear why the program fails well after the function call when listACQFilesDir() is used, 
		but not when listACQFiles() is used. 

	Returns: a vector<string> containing filenames, with the full absolute path. 
*/
//std::vector<std::string> listACQFilesDir(std::string ACQFilePath) {
//	HANDLE hFind;
//	WIN32_FIND_DATAA data;
//
//	std::string line;
//	std::vector<std::string> files;
//
//	wchar_t search_w[255];
//	size_t length;
//
//	std::string searchFor = ACQFilePath + "\\*.acq";
//
//	//string to char*
//	char* charSearch = new char;
//	std::strcpy(charSearch, searchFor.c_str());
//
//	////char* to wchar
//	//length = strlen(charSearch);									//find length of char array
//	//mbstowcs_s(&length, search_w, charSearch, length);
//
//	hFind = FindFirstFileA(charSearch, &data);
//	if (hFind != INVALID_HANDLE_VALUE) {
//		do {
//			line = ACQFilePath + "\\" + data.cFileName;
//			//printf("%s\n", line);
//			files.push_back(line);
//		} while (FindNextFileA(hFind, &data));
//		FindClose(hFind);
//	}
//
//	return files;
//
//}




/*
	Compiles a list of ACQ files in the current directory (full name), 
	sorted in alphabetical order. Because files are named according to the date of their creation, 
	alphabetical order = chronological order.

	EDIT 3-6-17: Files are not necessarily named in order of creation! 
	See sortACQ Files for sorting by last-modified date. 
	Last-modified date (NOT last-created date) reflects the date of data collection.

	Returns: a vector<string> containing filenames, with the full absolute path. 
*/
std::vector<std::string> listACQFiles() { //std::string ACQFilePath, std::string currentPath) {
	//obtaining names of all ACQ files in current folder, piping output to text file.

	system( "dir /s /b /a-d /on *.acq > file_names.txt" );						// /b lists file with extension
																				// /s provides full absolute path
																				// /a-d displays only files, no directories
																				// /on displays files in alphabetical order												
	std::ifstream filenames("file_names.txt");
	std::string line;
	std::vector<std::string> files;

	//reading text file containing filenames, "file_names.txt"
	while(getline(filenames, line))
		files.push_back(line);				//filenames to vector of strings

	return files;
}




/*
	Sort two pairs by second element of pair.
*/
//bool sortBySecond(const std::pair<std::string,FILETIME> &a, const std::pair<std::string,FILETIME> &b)
//{
//    return (CompareFileTime(&a.second, &b.second) == -1);
//}

struct sortBySecond {
    bool operator()(const std::pair<std::string,FILETIME> &left, const std::pair<std::string,FILETIME> &right) {
        return (CompareFileTime(&left.second, &right.second) == -1);
    }
};

/*
	Returns sorted list of ACQ files (sorted by last-modified date).
*/
std::vector<std::string> sortACQFiles(std::vector<std::string> fnames) { 
	//using list of names, create vector of pairs and sort by date
	std::vector<std::pair<std::string, FILETIME> > fnames_times;

	std::vector<std::string> sorted_fnames;
	HANDLE hFind;
	WIN32_FIND_DATAA data;
	std::pair<std::string, FILETIME> file;

	//for each filename in list
	for(int i = 0; i < fnames.size(); i++) {
		//first element of pair is filename
		file.first = fnames[i];

		//second element of pair is last-modified time
		hFind = FindFirstFileA(fnames[i].c_str(), &data);	//obtain file data, find last-write-time
		file.second = data.ftLastWriteTime;
		FindClose(hFind);									//close file

		fnames_times.push_back(file);						//add string-filetime pair to pair-vector
	}

	//sort pair-vector by filetime
	std::sort(fnames_times.begin(), fnames_times.end(), sortBySecond());

	//create vector of sorted filenames from pair-vector
	for(int i=0; i < fnames_times.size(); i++) {
		sorted_fnames.push_back(fnames_times[i].first);
	}

	return sorted_fnames;
}







/*
	Converts an input string to a wchar_t array.
	Returns: a wchar_t array with the contents of the input string.	
*/
wchar_t* stringToWchar(std::string fname_str) {
	char* fname_char;
	wchar_t fname_w[255];
	size_t length;

	//std::cout << "\tString version: " << fname_str << std::endl;

	//string to char*
	fname_char = new char[fname_str.size() + 1];					//creating new char array
	std::copy(fname_str.begin(), fname_str.end(), fname_char);		//copying contents of string to char*
	fname_char[fname_str.size()] = '\0';							//null character at end of char*

	//std::cout << "\tChar version: " << fname_char << std::endl;

	//char* to wchar_t
	length = strlen(fname_char);									//find length of char array
	mbstowcs_s(&length, fname_w, fname_char, length);				//convert to wchar_t

	//wprintf(L"\tWchar version: %ls\n", fname_w);

	return fname_w;

}




/*
	Converts an input wchar_t array to a string.
	Returns: a wchar_t array with the contents of the input string.	
*/
std::string wcharToString(wchar_t *fname_w) {
	//initializing char channel name
	size_t nameLen = wcslen(fname_w) + 1;							//account for null character when converting
	char* fname_char = new char[nameLen];	

	//converting wchar_t to char
	size_t charsConverted = 0;
	wcstombs_s(&charsConverted, fname_char, nameLen, fname_w, nameLen);

	//converting char to string for equality-checking
	std::string fname_str = std::string(fname_char);

	return fname_str;
}




/*
	Compiles a list of animals among all ACQ files in the current directory.

	Returns: a vector<string> containing animal names (without L/R suffix or parens).
		Eg: ["BM-40", "BM-41", "BM-36", ...]
*/

std::vector<std::string> listAnimals(std::vector<std::string> &fnames) {

	std::string fname_str;
	wchar_t *fname_w;
	
	ACQFile acqFile;

	CHInfo chInfo;
	wchar_t* wChannelName;
	std::string strChannelName;

	int channelsCount = 1;	

	std::vector<std::string> animals;
	std::string animalName;

	//for each file in current folder
	for(int i = 0; i < fnames.size(); i++) {
		fname_str = fnames.at(i);
		//std::cout << "Filename: " << fname_str << std::endl;
		fname_w = stringToWchar(fname_str);					

		//open the file
		if(initACQFile(fname_w, &acqFile))	{
			channelsCount = acqFile.numChannels;

			//for each channel in that file...
			for(int j = 0; j < channelsCount; j++)	{
				//get channel information
				getChannelInfo(j, &acqFile, &chInfo);
				wChannelName = chInfo.label;	
				strChannelName = wcharToString(wChannelName);

				std::size_t endPos = strChannelName.find("(");
				animalName = strChannelName.substr(0, endPos);
					
				//if animal isn't already in the animal name list, add it. 	
				if (std::find(animals.begin(), animals.end(), animalName) == animals.end()) {
					//std::cout << "Animal being added: " << animalName << std::endl;
					animals.push_back(animalName);	
					std::cout << "ANIMAL NAMES: " << animalName << std::endl;

				}
			}

			closeACQFile(&acqFile);
		}

	}

	return animals;

}




/*
	Opens a DCL file for writing, and then writes header information to that file.
	Writes the beginning portion of the header ONLY, up to the 3B 0xffffffff, 0x00000000, 0xffffffff spacer. 

	Inputs: 
		file: a filestream object, the DCL file to be written
		scanFreq: the data's scan frequency, obtained from the original ACQ files
		numDataPoints: the total number of data points the DCL file will contain. 	
*/
void writeDCLHeader(std::fstream &file, int32_t &scanFreq, uint64_t &numDataPoints, std::string &animal, std::string DCLFilePath) {
	//std::cout << "Scan Frequency Inside Function: " << scanFreq << std::endl;
	//std::cout << "Num Data Points Outside Loop: " << numDataPoints << std::endl;

	//std::string DCLFilePath = "C:\\Users\\sk430\\Documents\\Visual Studio 2012\\Projects\\ConcatBM40\\Debug\\dcl_files";
	//std::cout << "-----DCL File path: " << DCLFilePath << std::endl;

	std::string DCLFileName = DCLFilePath + "\\" + animal + ".dcl";

	//file.open("testconcat.dcl", std::fstream::binary | std::fstream::out | std::fstream::in | std::fstream::trunc);
	file.open(DCLFileName, std::fstream::binary | std::fstream::out | std::fstream::in | std::fstream::trunc);

	const float fileVersion = 2.6f;
	const int32_t channelsCount = 2;								//only two channels -- left and right
	const double scanRange = 20;									//underestimating scan range leads to overflow! be careful!
	const int8_t bipolar = 1;
	const int32_t triggerDelayMs = 0;
	const int32_t dateTime[6] = { 2010, 2, 26, 14, 32, 52 };
	
	//these actually don't matter, and aren't used when dClamp reads in DCL
	const int32_t buttonColor = 1;
	const double displayGain = 1;
	const int32_t displayOffset = 1;

	//this does matter! its value is floored in dClamp, so can't have 0 < analogGain < 1. 
	const int32_t analogGain = 1;	

	//std::cout << "Animal name input to header HERE: " << animal << std::endl;

	//channel names and lengths
	//const char* channelNames[2] = {"BM-40(l)", "BM-40(r)"};			//new char*[channelsCount];
	//int nameLens[2] = {9, 9};	
	std::string leftChannelName;
	std::string rightChannelName;

	leftChannelName = animal + "(l)";
	rightChannelName = animal + "(r)";
	
	const char* channelNames[2] = {leftChannelName.c_str(), rightChannelName.c_str()};
	int nameLens[2] = {leftChannelName.length() + 1, rightChannelName.length() + 1};		//+1 accounts for null character

	//spacer between header and data
	const int32_t spacer[3] = { 0xffffffff, 0x00000000, 0xffffffff };
	int dataStart;

	//std::cout << "Scan Frequency Inside Function: " << scanFreq << std::endl;
	//std::cout << "Num Data Points Outside Loop: " << numDataPoints << std::endl;
	//writing out items for whole-file header
	file.write((char*) &fileVersion, sizeof(float));
	file.write((char*) &numDataPoints, sizeof(uint64_t));
	file.write((char*) &channelsCount, sizeof(int32_t));
	file.write((char*) &scanFreq, sizeof(int32_t));

	//underestimating scan range leads to overflow! be careful!
	file.write((char*) &scanRange, sizeof(double));
	file.write((char*) &bipolar, sizeof(int8_t));
	file.write((char*) &triggerDelayMs, sizeof(int32_t));
	file.write((char*) &dateTime, sizeof(int32_t)*6);


	//writing out channel-wise headers for DCL
	for(int i=0; i<channelsCount; i++) {
		//std::cout << "Channel " << i << std::endl;
		//std::cout << "\t Channel Name Length: 0x" << std::hex << (int16_t) nameLens[i] << std::dec << std::endl;
		//std::cout << "\t Channel Name: " << channelNames[i] << std::endl;

		//writing the lengths of the strings
		file.write((char*) &nameLens[i], sizeof(int8_t));

		//writing channel name char-by-char (and printing out so can confirm with hex reader)
		std::string channelName = channelNames[i];
		//std::cout << "\t Channel Name: 0x" << std::hex;
		for(int j=0; j<nameLens[i]; j++) {		
		//	std::cout << (int16_t) channelName[j];
			file.write((char*) &channelName[j], sizeof(char));
		}
		std::cout << std::dec << std::endl;

		//writing the useless values described above -- not needed for DCL read.
		file.write((char*) &buttonColor, sizeof(int32_t));
		file.write((char*) &displayGain, sizeof(double));
		file.write((char*) &displayOffset, sizeof(int32_t));
		file.write((char*) &analogGain, sizeof(int32_t));
	}

	//writing out spacer between header and data
	file.write((char*) &spacer, sizeof(int32_t)*3);
	//for(int i=0; i<3; i++) {	//printing spacer contents to console
	//	std::cout << "Spacer written: " << std::hex << spacer[i] << std::dec << std::endl;
	//} 

	//report current position: header-end, data-start
	//dataStart = file.tellg();
	//std::cout << "Data start position: " << dataStart << std::endl;
}




/*
	Writes an interleaved, converted segment of a given size to a DCL file.  
	Inputs:
		dataCount: an index representing where we currently are in the channel -- how many points have been written out so far?
		segmentSize: size of the segment to read in from the ACQ file (and subsequently write out to the DCL file)
		acqFile: ACQ file being read from
		dclFile: DCL file being written to
		chindex_left: index of the channel in the ACQ file containing the animal's left-side data
		chindex_right: index of the channel in the ACQ file containing the animal's right-side data. 

	Using this information, this function:
		(1) Looks at two channels in the ACQ file: the channels whose indices are chindex_left and chindex_right, 
		which are the animal's left- and right-channels, respectively. 
		(2) Loads data from both channels. The number of points read is determined by segmentSize, and where this segment starts
		depends upon dataCount. 
		(3) Interleaves and converts the data from both channels. This creates a segment that can be written directly to the DCL file.
		(4) Writes the segment to the DCL file. 

*/
void writeSegment(long int dataCount, long int segmentSize, ACQFile acqFile, std::fstream &dclFile, int chindex_left, int chindex_right) {
	CHInfo chInfo;

	//preparing data for writing
	float lFloatData;
	float rFloatData;
	double* lChannelData;
	double* rChannelData;
	uint16_t* writeData;

	//number of channels being read is always 2 -- left and right.
	int channelsCount = 2;

	lChannelData = new double[segmentSize];
	rChannelData = new double[segmentSize];
	writeData = new uint16_t[(channelsCount)*(segmentSize)]; 

	long startIndex = dataCount - 1; //index = # points -1
	long endIndex = dataCount + segmentSize - 2; //index = # points - 1

	if(getChannelInfo(chindex_left, &acqFile, &chInfo)) {
		if(getSampleSegment(&acqFile, &chInfo, lChannelData, startIndex, endIndex)) {
			//std::cout << "Reading Channel " << chindex_left << " sample segment: " << startIndex << " to " << endIndex << std::endl;
		}
	}

	if(getChannelInfo(chindex_right, &acqFile, &chInfo)) {
		if(getSampleSegment(&acqFile, &chInfo, rChannelData, startIndex, endIndex)) {
			//std::cout << "Reading Channel " << chindex_right << " sample segment: " << startIndex << " to " << endIndex << std::endl;
		}
	}	

	//std::cout << "we're getting here: " << std::endl;

	for(int j = 0; j < segmentSize; j++) { //for each data point in channel segment

		//double-to-float conversion
		lFloatData = static_cast<float>(lChannelData[j]); 
		rFloatData = static_cast<float>(rChannelData[j]); 		

		//float-to-uint16 conversion
		int rangeMin = -32768; //SIGNED int16 min value
		double scanRange = 20;
		int analogGain = 1;

		//issue with THIS LINE
		double conv = analogGain/(2*scanRange/UINT16_MAX);

		uint16_t lConvData = lFloatData*conv - rangeMin;
		uint16_t rConvData = rFloatData*conv - rangeMin;

		//computing location for converted point
		int lDataIndex = (channelsCount) * j;		// + i; 
		int rDataIndex = channelsCount * j + 1;		// + i;

		writeData[lDataIndex] =  (uint16_t) (lConvData);
		writeData[rDataIndex] =  (uint16_t) (rConvData);
	}

	//not making it to this point
	//std::cout << "we're getting here, second time: " << std::endl;

	//writing interleaved segment out as one block
	//cout << "\tBefore write: " << file.tellg() << endl;
	dclFile.write((char*) &writeData[0], sizeof(uint16_t)*segmentSize*channelsCount);
	//cout << "\tAfter write: " << file.tellg() << endl;
			
	std::cout << "\tWriting: " << std::hex << writeData[0]  << " to " << writeData[segmentSize*channelsCount -1] << std::dec << std::endl; 


	delete lChannelData;
	delete rChannelData;
	delete writeData;

}


int main(int argc, char* argv[]) {

	std::string fname_str;
	wchar_t *fname_w;

	std::string fname_animal_str;
	wchar_t *fname_animal_w;
	
	ACQFile acqFile;
	std::fstream dclFile;

	CHInfo chInfo;
	wchar_t* wChannelName;
	std::string strChannelName;

	int32_t scanFreq = 0;
	int numSamples = 0;												//number of samples in a channel
	int channelsCount = 1;											//must be at least one channel in file
	uint64_t numDataPoints = 0;										//total number of data points in DCL file being created

	bool bm_flag = false;

	std::string ACQFilePath;
	std::string DCLFilePath;

	//std::string currentPath = ExePath(argv[0]);
	//std::cout << "\n CURRENT PATH OF PROGRAM: " << currentPath << "\n" << std::endl;

	//Prompt user for source and destination paths:
	std::cout << "Enter full path where ACQ files are located: " << std::endl;
	std::getline(std::cin, ACQFilePath);
	std::cout << "PATH PROVIDED: " << ACQFilePath << "\n" << std::endl;

	std::cout << "Enter full path where you would like DCL files to be created: " << std::endl;
	std::getline(std::cin, DCLFilePath);
	std::cout << "PATH PROVIDED: " << DCLFilePath << "\n" << std::endl;

		//THIS VERSION DOESN'T WORK
		//std::vector<std::string> fnames = listACQFilesDir(ACQFilePath);
		//for(int i = 0; i < fnames.size(); i++) {
		//	std::cout << "FILE NAMES: " << fnames.at(i) << std::endl;
		//}

	std::cout << "\n\n\n" << std::endl;

	////THIS VERSION WORKS
	std::vector<std::string> unsorted = listACQFiles();//listACQFilesDir(ACQFilePath);//ACQFilePath, currentPath);
	std::vector<std::string> fnames = sortACQFiles(unsorted);
	for(int i = 0; i < fnames.size(); i++) {
		std::cout << "FNAMES: " << fnames.at(i) << std::endl;
	}

	std::vector<std::string> filesToRemove;
	std::vector<std::string> animals = listAnimals(fnames);		//obtain a list of animals in the ACQ files
	std::string currentAnimal;

	std::vector<std::string> fnames_animal;


	//for each animal in the ACQ files
	for(int k = 0; k < animals.size(); k++) {
		currentAnimal = animals.at(k);

		//create a copy of fnames specific to that animal, fnames_animal
		std::vector<std::string> fnames_animal(fnames);
		filesToRemove.clear();

		for(int i = 0; i < fnames_animal.size(); i++) {
			std::cout << "FNAMES_ANIMAL: " << fnames_animal.at(i) << std::endl;
		}


		//for each filename...
		for(int i = 0; i < fnames.size(); i++) {
			fname_str = fnames.at(i);
			//std::cout << "Filename: " << fname_str << std::endl;
			fname_w = stringToWchar(fname_str);					
			//const wchar_t* fname_w = L"IctalLikeExample.acq";

/*			fname_animal_str = fnames_animal.at(i);
			fname_animal_w = stringToWchar(fname_animal_str);*/					

			//open the file
			if(initACQFile(fname_w, &acqFile))	{
				//std::cout << "\tAre we getting here?" << std::endl;

				//std::cout << "\tCurrent ACQ file: " << fname_str << std::endl;	

				channelsCount = acqFile.numChannels;
				scanFreq = 1000.0/acqFile.sampleRate;		 			//Conversion from msec/sample to samples/sec (Hz).
				//std::cout << "Scan Frequency Within Loop: " << scanFreq << std::endl;

				//for each channel in that file...
				for(int j = 0; j < channelsCount; j++)				
				{
					//get channel information
					getChannelInfo(j, &acqFile, &chInfo);
					wChannelName = chInfo.label;	
					strChannelName = wcharToString(wChannelName);

					numSamples = chInfo.numSamples;

					//if this file contains the animal we're looking for...
					if((strChannelName.compare(currentAnimal + "(l)") == 0) || (strChannelName.compare(currentAnimal + "(r)") == 0)) {
						bm_flag = true;									//set flag to true
						numDataPoints += numSamples;					//total data points = sum of all channels' data points
						//std::cout << "Adding to list: " << numDataPoints << std::endl;
					}
					//std::cout << "\t" << strChannelName << std::endl;
				}

				//if this file does not have a channel representing animal BM, remove from list
				if(!bm_flag) {
					//what file are we erasing?
					//std::cout << "Erasing: " << fname_str << std::endl;
					filesToRemove.push_back(fname_str);
					//fnames.erase(std::remove(fnames.begin(), fnames.end(), fname_str), fnames.end());
				}
			
				bm_flag = false;


				closeACQFile(&acqFile);		//CLOSING FILE
			}

		}

		std::string toRemove;
		//removing files that don't have BM-whatever
		for(int i = 0; i < filesToRemove.size(); i++) {
			toRemove = filesToRemove.at(i);
			fnames_animal.erase(std::remove(fnames_animal.begin(), fnames_animal.end(), toRemove), fnames_animal.end());

		}


		std::cout << "FNAMES_ANIMAL AFTER REMOVAL: " << std::endl;
		for(int i = 0; i < fnames_animal.size(); i++) {
			fname_animal_str = fnames_animal.at(i);
			std::cout << "\tFNAMES_ANIMAL Remaining: " << fname_animal_str << std::endl;
			fname_animal_w = stringToWchar(fname_animal_str);	
		}
		//std::cout << "DONE PRINTING: " << std::endl;

	
		for(int i = 0; i < fnames.size(); i++) {
			std::cout << "FNAMES AFTER REMOVAL: " << fnames.at(i) << std::endl;
		}
		std::cout << "---------------\n\n " << std::endl;

		//std::cout << "Scan Frequency Outside Loop: " << scanFreq << std::endl;
		//std::cout << "Num Data Points Outside Loop: " << numDataPoints << std::endl;
	
		//write a DCL header for the file
		writeDCLHeader(dclFile, scanFreq, numDataPoints, currentAnimal, DCLFilePath);


		//--------------

		//keeping track of data position
		long int dataCount = 1;											//starting point -- where in the data are we?
		int finalChannels = 2;											//will only be 2 channels in output file -- left and right!
		long int numPointsPerChannel = numDataPoints/finalChannels;
		long int segmentSize;

		//what are the channel indices of the channel we care about?
		int chindex_left;
		int chindex_right;

		//for each of the filenames containing BM animal data...
		for(int i = 0; i < fnames_animal.size(); i++) {
			fname_animal_str = fnames_animal.at(i);
			std::cout << "Current filename: " << fname_animal_str << std::endl;
			fname_animal_w = stringToWchar(fname_animal_str);

			//open file
			if(initACQFile(fname_animal_w, &acqFile)) {

				//for each channel in file, search for BM-40 left/right.
				for(int j = 0; j < acqFile.numChannels; j++) {
					if(getChannelInfo(j, &acqFile, &chInfo)) { 
						wChannelName = chInfo.label;	
						strChannelName = wcharToString(wChannelName);

						//what if there is more than one channel of the same name in a file?
						//would be extremely unlikely... but it's good to know if that happens.
						if(strChannelName.compare(currentAnimal + "(l)") == 0) chindex_left = j;
						else if(strChannelName.compare(currentAnimal + "(r)") == 0) chindex_right = j;
					}
				}

				//number of points in one of the channels being read
				int numPointsPerChannel = chInfo.numSamples;

				while(dataCount < numPointsPerChannel) {
			
					if((numPointsPerChannel - dataCount) >= 1000000 ) {     //if there are enough points to large-size
						segmentSize = 1000000;	//1.0e7; 
					} else {
						segmentSize = numPointsPerChannel - dataCount + 1;
						std::cout << "\tSize of small chunk: " << segmentSize << std::endl;
					}

					//need to write this function
					writeSegment(dataCount, segmentSize, acqFile, dclFile, chindex_left, chindex_right);
					dataCount += segmentSize;
					std::cout << "\tPosition: " << dataCount << ", out of " << numPointsPerChannel << std::endl;
				}

				//std::cout << "Are we moving past the loop?" << std::endl;
				closeACQFile(&acqFile);											///CLOSING FILE

				dataCount = 1;
			}
		}



		const int32_t timestampCount = 0;	//timestamps appear to be made up of 2 numbers
		dclFile.write((char*)&timestampCount, sizeof(int32_t));

		const int32_t ttlstampCount = 0;	//timestamps appear to be made up of 2 numbers
		dclFile.write((char*) &ttlstampCount, sizeof(int32_t));

		dclFile.close();

		numDataPoints = 0;
	
	}

	return 0;

}