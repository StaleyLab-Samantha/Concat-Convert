#include <windows.h>
#include <iostream>
#include <fstream>

#include <sstream>
#include <cctype>
#include <clocale>

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
	Running this script will concatenate all ACQ files in the current folder into DCL files (which are labeled by animal). 

*/



/*
	OLD -- DO NOT USE.
	
	Compiles a list of ACQ files in the current directory (full name), 
	sorted in alphabetical order. Because files are named according to the date of their creation, 
	alphabetical order = chronological order.

	EDIT 3-6-17: Files are not necessarily named in order of creation! 
	See sortACQ Files for sorting by last-modified date. 
	Last-modified date (NOT last-created date) reflects the date of data collection.

	Returns: a vector<string> containing filenames, with the full absolute path. 
*/
//std::vector<std::string> listACQFiles() { //std::string ACQFilePath, std::string currentPath) {
//	//obtaining names of all ACQ files in current folder, piping output to text file.
//
//	system( "dir /s /b /a-d /on *.acq > file_names.txt" );						// /b lists file with extension
//																				// /s provides full absolute path
//																				// /a-d displays only files, no directories
//																				// /on displays files in alphabetical order												
//	std::ifstream filenames("file_names.txt");
//	std::string line;
//	std::vector<std::string> files;
//
//	//reading text file containing filenames, "file_names.txt"
//	while(getline(filenames, line))
//		files.push_back(line);				//filenames to vector of strings
//
//	return files;
//}


/*
	Converts an input string to a wchar_t array.
	Returns: a wchar_t array with the contents of the input string.	
*/
wchar_t* stringToWchar(std::string fname_str) {
	char* fname_char;
	wchar_t fname_w[255];
	size_t length;

	//string to char*
	fname_char = new char[fname_str.size() + 1];					//creating new char array
	std::copy(fname_str.begin(), fname_str.end(), fname_char);		//copying contents of string to char*
	fname_char[fname_str.size()] = '\0';							//null character at end of char*

	//char* to wchar_t
	length = strlen(fname_char);									//find length of char array
	mbstowcs_s(&length, fname_w, fname_char, length);				//convert to wchar_t

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
	Compiles a list of ACQ files in the directory specified by ACQFilePath (provided by user).
	
	See sortACQFiles() for sorting by date (i.e. Windows' last-modified date). 
	Last-modified date (NOT last-created date) reflects the date of data collection.

	Inputs: ACQFilePath, a string containing the full absolute path to the folder containing the ACQ files.
	Returns: a vector<string> containing filenames, with the full absolute path. 
*/
std::vector<std::string> listACQFiles(std::string ACQFilePath) {//, std::string currentPath) {

	WIN32_FIND_DATA data;
	HANDLE hFind;
	std::string line;
	std::vector<std::string> files;
	wchar_t *path_w;
	wchar_t *fname_w;
	std::string fname;

	//create search-string: look for ACQ files in the given directory 
	path_w = stringToWchar(ACQFilePath + "\\*.acq");
	hFind = FindFirstFile(path_w, &data);

	//if path is valid
	if (hFind != INVALID_HANDLE_VALUE) 
	{
		//for all files in directory, add full filenames + paths to vector<string>
		do {
			fname_w = data.cFileName;							
			fname = ACQFilePath + "\\" + wcharToString(fname_w);
			files.push_back(fname);
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	} 


	return files;
}




/*
	Given two <string, FILETIME> Pairs, this will compare the two Pairs by FILETIME. 
	FILETIME: Windows' date-time struct for files

	Inputs: left, right (two <string, FILETIME> Pairs). 
	Returns: true if left FILETIME < right FILETIME, false otherwise
*/

struct sortBySecondFiletime {
    bool operator()(const std::pair<std::string,FILETIME> &left, const std::pair<std::string,FILETIME> &right) {
        return (CompareFileTime(&left.second, &right.second) == -1);
    }
};



/*
	Returns sorted list of ACQ files (sorted by last-modified date).
	Note that this sort involves Windows' last-modified date, NOT date-created!

	Date-created is inaccurate for these ACQ files. 
	Last-modified reflects the date specified in the filenames of the ACQ files.

	Inputs: vector<string> of unsorted ACQ files in the desired directory (ACQFilePath), path included.
	Returns: vector<string> of sorted ACQ files in the desired directory, path included
*/
std::vector<std::string> sortACQFilesFiletime(std::vector<std::string> fnames) { 
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
	std::sort(fnames_times.begin(), fnames_times.end(), sortBySecondFiletime());

	//create vector of sorted filenames from pair-vector
	for(int i=0; i < fnames_times.size(); i++) {
		sorted_fnames.push_back(fnames_times[i].first);
	}

	return sorted_fnames;
}




/*
	Given two <string, string> Pairs, this will compare the two Pairs by alphabetic string ordering. 
	Inputs: left, right (two <string, string> Pairs). 

	Returns: true if left string < right string alphabetically. 

	In sorting files, Pair.first is the filename/path, while Pair.second is the datetime portion of the filename.
	In YYYY-MM-DD format, strings that are sorted alphabetically are also sorted by date.

*/

struct sortBySecondTimestamp {
    bool operator()(const std::pair<std::string,std::string> &left, const std::pair<std::string,std::string> &right) {
        return (left.second < right.second);
    }
};




/*
	Returns sorted list of ACQ files (sorted by datetime in filename).

	Using file-naming timestamp convention:
	[beginning of name]YYYY-MM-DDTHH_MM_SS.acq
	Sorting the last 19 characters (excluding extension) of the filenames should sort by date.

	Inputs: vector<string> of unsorted ACQ files in the desired directory (ACQFilePath), path included.
	Returns: vector<string> of sorted ACQ files in the desired directory, path included
*/

std::vector<std::string> sortACQFilesTimestamp(std::vector<std::string> fnames) { 
	//using list of names, create vector of pairs and sort by date
	std::vector<std::pair<std::string, std::string> > fnames_times;
	std::vector<std::string> sorted_fnames;
	std::pair<std::string, std::string> file;

	//for each filename in list
	for(int i = 0; i < fnames.size(); i++) {
		//first element of pair is filename
		file.first = fnames[i];
		//second element of pair is filename's timestamp
		file.second = fnames[i].substr(fnames[i].length()-23, 19);

		fnames_times.push_back(file);						//add string-filetime pair to pair-vector
	}

	//sort pair-vector by filetime
	std::sort(fnames_times.begin(), fnames_times.end(), sortBySecondTimestamp());

	//create vector of sorted filenames from pair-vector
	for(int i=0; i < fnames_times.size(); i++) {
		sorted_fnames.push_back(fnames_times[i].first);
	}

	return sorted_fnames;
}





//compares strings, case-insensitive
bool icompare_pred(unsigned char a, unsigned char b)
{
    return std::tolower(a) == std::tolower(b);
}

bool icompare(std::string const& a, std::string const& b)
{
    if (a.length()==b.length()) {
        return std::equal(b.begin(), b.end(),
                           a.begin(), icompare_pred);
    }
    else {
        return false;
    }
}



/*
	Compiles a list of animals among all ACQ files in the current directory.
	Also creates a list of broken ACQ files.

	Returns: a vector<string> containing animal names (without L/R suffix or parens).
		Eg: ["BM-40", "BM-41", "BM-36", ...]

	Previously, also created a list of broken ACQ files, and wrote this list to a txt file in
	path specified by DCLFilePath. However, it seems that the list of broken ACQ files varied 
	from time to time (indicating that the ACQ files are not necessarily actually broken???).

	For now, this functionality has been commented out.

		
*/

std::vector<std::string> listAnimals(std::vector<std::string> &fnames, std::string animalsFromFile, std::string rightFormat, 
									 std::string leftFormat) {//, std::string DCLFilePath) {

	std::string fname_str;
	wchar_t *fname_w;
	
	ACQFile acqFile;
	//std::ofstream brokenFilesList;

	CHInfo chInfo;
	wchar_t* wChannelName;
	std::string strChannelName;

	int channelsCount = 1;	

	std::vector<std::string> animals;
	std::string animalName;

	//std::string brokenList = DCLFilePath + "\\broken_files_list.txt";
	//std::ofstream brokenFilesList(brokenList);

	//if user wishes to concatenate all detected animals, detect animals! and provide a list to the user
	if(icompare(animalsFromFile, "all")) {

		//for each file in current folder
		for(int i = 0; i < fnames.size(); i++) {
			fname_str = fnames.at(i);
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

					//find the animal's name (e.g. BM-40, BM-51...)
					std::size_t endPos = strChannelName.find("(");
					animalName = strChannelName.substr(0, endPos);
					
					//if animal isn't already in the animal name list, add it. 	
					if (std::find(animals.begin(), animals.end(), animalName) == animals.end()) {
						animals.push_back(animalName);	
					}
				}
				closeACQFile(&acqFile);
			}
			else {		//if can't be opened, it's a broken file -- add it to the list (filename only, excluding filepath)
				//brokenFilesList << fname_str.substr(fname_str.find_last_of("/\\")+1) << std::endl;
			}
		}
	} 
	else {	//if user instead provides a list of animals, parse the comma-separated list in animalsFromFile string

		//remove any spaces the user may have added to the comma-separated list
		animalsFromFile.erase(std::remove( animalsFromFile.begin(), animalsFromFile.end(), ' '), animalsFromFile.end());

		std::stringstream ss(animalsFromFile);
		std::string animalName;
		std::string ch;

		//while there are still characters in the stringstream from animalsFromFile
		while(std::getline(ss, animalName, ',')) {
		{
				animals.push_back(animalName);	//add animalName to list
				animalName = "";				//reset animalName
			}
		}

		//for (i=0; i< animals.size(); i++)
		//	std::cout << vect.at(i)<<std::endl;



	}
	//remove BM-22 -- this is a typo!
	animals.erase(std::remove(animals.begin(), animals.end(), "BM-22"), animals.end());
	return animals;
}


/*
	A dummy version of listAnimals. Returns a predetermined list of animals. Useful if
	you do not wish all animals' data to be converted to DCL.

	Returns: a vector<string> containing animal names (without L/R suffix or parens).
		Eg: ["BM-40", "BM-41", "BM-36", ...]
*/
//std::vector<std::string> listAnimals(std::vector<std::string> &fnames) {
//	//NOTE: BM-15, BM-22 DON'T EXIST
//
//	//convert array of strings to vector of strings -- vectors cannot be assigned literals
//	//std::string animals_arr[] = {"BM-12", "BM-13", "BM-14", "BM-16", "BM-17", "BM-18", "BM-19", "BM-20", "BM-21", "BM-23"};
//	//std::string animals_arr[] = {"BM-17", "BM-18", "BM-19", "BM-20", "BM-21", "BM-23"};
//	//SKIPPING 21, COME BACK TO IT AFTER FIXING 21/22 ISSUE
//	//std::string animals_arr[] = {"BM-23", "BM-24", "BM-25", "BM-26", "BM-27", "BM-28", "BM-29", "BM-30", "BM-31"};
//	//std::string animals_arr[] = {"BM-32", "BM-33", "BM-34", "BM-35", "BM-36", "BM-37", "BM-38", "BM-39", "BM-40", "BM-41", "BM-42", "BM-43", "BM-44", "BM-45", "BM-46", "BM-47", "BM-48", "BM-49", "BM-50"};
//	//std::string animals_arr[] = {"BM-60"};
//	std::string animals_arr[] = {"BM-21", "BM-22", "BM-33"};
//	std::vector<std::string> animals( animals_arr, animals_arr + ( sizeof ( animals_arr ) /  sizeof ( std::string ) ) );
//	
//	//remove BM-22 -- this is a typo!
//	animals.erase(std::remove(animals.begin(), animals.end(), "BM-22"), animals.end());
//
//	return animals;
//}




//obtains a list of filenames containing data for a given animal

std::vector<std::string> getFnamesAnimal(std::vector<std::string> fnames, std::string currentAnimal, std::string currentAnimal_nodash) {
	ACQFile acqFile;
	CHInfo chInfo;

	std::string fname_str;
	wchar_t *fname_w;

	std::string fname_animal_str;
	wchar_t *fname_animal_w;

	int32_t scanFreq = 0;
	int numSamples = 0;												//number of samples in a channel
	int channelsCount = 1;											//must be at least one channel in file
	uint64_t numDataPoints = 0;	

	wchar_t* wChannelName;
	std::string strChannelName;

	bool bm_flag = false;
	std::vector<std::string> filesToRemove;

	//std::string currentAnimal;
	//std::string currentAnimal_nodash;
	//std::vector<std::string> fnames_animal;

	std::vector<std::string> fnames_animal(fnames);

	for(int i = 0; i < fnames.size(); i++) {
		//convert filename to wchar for ACQ API
		fname_str = fnames.at(i);
		fname_w = stringToWchar(fname_str);					
		
		//open the file
		if(initACQFile(fname_w, &acqFile))	{
			channelsCount = acqFile.numChannels;
			scanFreq = 1000.0/acqFile.sampleRate;		 			//Conversion from msec/sample to samples/sec (Hz).

			//for each channel in that file...
			for(int j = 0; j < channelsCount; j++)				
			{
				//get channel information
				getChannelInfo(j, &acqFile, &chInfo);
				wChannelName = chInfo.label;	
				strChannelName = wcharToString(wChannelName);

				numSamples = chInfo.numSamples;

				//if this file contains the animal we're looking for...
				if((strChannelName.compare(currentAnimal + "(l)") == 0) || (strChannelName.compare(currentAnimal_nodash + "(l)") == 0)  
					|| (strChannelName.compare(currentAnimal + "(r)") == 0) || (strChannelName.compare(currentAnimal_nodash + "(r)") == 0)) {
					bm_flag = true;									//set flag to true
					numDataPoints += numSamples;					//total data points = sum of all channels' data points
				}
			}
			//if this file does not have a channel representing animal BM, remove from list
			if(!bm_flag) {
				filesToRemove.push_back(fname_str);
			}
			bm_flag = false;
			closeACQFile(&acqFile);
		}
	}
	std::string toRemove;

	//removing files that don't have this animal's data from the list of files to scan
	for(int i = 0; i < filesToRemove.size(); i++) {
		toRemove = filesToRemove.at(i);
		fnames_animal.erase(std::remove(fnames_animal.begin(), fnames_animal.end(), toRemove), fnames_animal.end());
	}
	return fnames_animal;
}


//obtains the scan frequency
int32_t getScanFreq(std::vector<std::string> fnames) {
	std::string fname_str;
	wchar_t *fname_w;
	ACQFile acqFile;
	int32_t scanFreq;

	//pick any of the ACQ files (picking the first one for convenience)
	fname_str = fnames.at(0);
	fname_w = stringToWchar(fname_str);		

	if(initACQFile(fname_w, &acqFile))	{
		scanFreq = 1000.0/acqFile.sampleRate;		 			//Conversion from msec/sample to samples/sec (Hz).
		closeACQFile(&acqFile);
	}

	return scanFreq;

}


//obtains the number of data points in a given animal's DCL file
uint64_t getNumDataPoints(std::vector<std::string> fnames, std::string currentAnimal, std::string currentAnimal_nodash) {
	std::string fname_str;
	wchar_t *fname_w;
	ACQFile acqFile;
	uint64_t numDataPoints = 0;
	int numSamples = 0;												//number of samples in a channel
	int channelsCount = 1;	
	CHInfo chInfo;
	wchar_t* wChannelName;
	std::string strChannelName;

	for(int i = 0; i < fnames.size(); i++) {
		//convert filename to wchar for ACQ API
		fname_str = fnames.at(i);
		fname_w = stringToWchar(fname_str);					
		
		//open the file
		if(initACQFile(fname_w, &acqFile))	{
			channelsCount = acqFile.numChannels;

			//for each channel in that file...
			for(int j = 0; j < channelsCount; j++)				
			{
				//get channel information
				getChannelInfo(j, &acqFile, &chInfo);
				wChannelName = chInfo.label;	
				strChannelName = wcharToString(wChannelName);

				numSamples = chInfo.numSamples;
				if((strChannelName.compare(currentAnimal + "(l)") == 0) || (strChannelName.compare(currentAnimal_nodash + "(l)") == 0)  
					|| (strChannelName.compare(currentAnimal + "(r)") == 0) || (strChannelName.compare(currentAnimal_nodash + "(r)") == 0)) {
					numDataPoints += numSamples;					//total data points = sum of all channels' data points
				}
			}
			closeACQFile(&acqFile);
		}
	}

	return numDataPoints;

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

	std::string DCLFileName = DCLFilePath + "\\" + animal + ".dcl";
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

	//channel names and lengths
	std::string leftChannelName;
	std::string rightChannelName;

	leftChannelName = animal + "(l)";
	rightChannelName = animal + "(r)";
	const char* channelNames[2] = {leftChannelName.c_str(), rightChannelName.c_str()};
	int nameLens[2] = {leftChannelName.length() + 1, rightChannelName.length() + 1};		//+1 accounts for null character

	//spacer between header and data
	const int32_t spacer[3] = { 0xffffffff, 0x00000000, 0xffffffff };
	int dataStart;

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
		//writing the lengths of the strings
		file.write((char*) &nameLens[i], sizeof(int8_t));

		//writing channel name char-by-char
		std::string channelName = channelNames[i];
		for(int j=0; j<nameLens[i]; j++) {		
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

	dclFile.write((char*) &writeData[0], sizeof(uint16_t)*segmentSize*channelsCount);	
	//std::cout << "\tWriting: " << std::hex << writeData[0]  << " to " << writeData[segmentSize*channelsCount -1] << std::dec << std::endl; 

	delete lChannelData;
	delete rChannelData;
	delete writeData;

}


//reads paths from input text file
std::vector<std::string>& getPaths(std::ifstream &concatInfoFile) {//, std::string concatInfoFilePath) {

	std::vector<std::string> filePaths;

	std::string line;
	std::string ACQPath;
	std::string DCLPath;

	concatInfoFile.open("C:\\Users\\sk430\\Documents\\Visual Studio 2012\\Projects\\Concat-Convert\\Release\\concat_info.txt", std::ifstream::in);

	std::getline(concatInfoFile, ACQPath);
	std::getline(concatInfoFile, DCLPath);

	//filePaths = {ACQPath, DCLPath};

	filePaths.push_back(ACQPath);
	filePaths.push_back(DCLPath);

	//while(std::getline(concatInfoFile, line))
	//	paths.push_back(line);				//filenames to vector of strings

	return filePaths;
}


int main(int argc, char* argv[]) {

	std::string fname_str;
	wchar_t *fname_w;

	std::string fname_animal_str;
	wchar_t *fname_animal_w;
	
	ACQFile acqFile;
	std::fstream dclFile;
	std::ofstream dclInfoFile;
	//std::fstream brokenFilesList;

	CHInfo chInfo;
	wchar_t* wChannelName;
	std::string strChannelName;

	int32_t scanFreq = 0;
	int numSamples = 0;												//number of samples in a channel
	int channelsCount = 1;											//must be at least one channel in file
	uint64_t numDataPoints = 0;										//total number of data points in DCL file being created

	double hoursInFile;

	bool bm_flag = false;

	std::string ACQFilePath;
	std::string DCLFilePath;

	std::vector<std::string> filesToRemove;
	std::string currentAnimal;
	std::string currentAnimal_nodash;
	std::vector<std::string> fnames_animal;

	std::string concatInfoFilePath;
	std::ifstream concatInfoFile;
	
	std::string animalsFromFile;
	std::string animalFormat;
	std::string rightFormat;
	std::string leftFormat;


	////Prompt user for source and destination paths:
 //	std::cout << "Enter full path where ACQ files are located: " << std::endl;
	//std::getline(std::cin, ACQFilePath);
	//std::cout << "PATH PROVIDED: " << ACQFilePath << "\n" << std::endl;
	//
	//std::cout << "Enter full path where you would like DCL files to be created: " << std::endl;
	//std::getline(std::cin, DCLFilePath);
	//std::cout << "PATH PROVIDED: " << DCLFilePath << "\n" << std::endl;
	//std::cout << "\n\n\n" << std::endl;


	//Prompt user, reminding them to fill out text file. Ask if they've filled it out, exit if no.
	//TODO: create log file after program ends, indicating errors, etc. ? Write to DCLFilePath

	////Prompt user for path where concat_info file is located
 	std::cout << "Enter the full path where your concatenation-info text file is located, including the file name.\nThis path should be of the form C:\\...\\...\\concat_info.txt.\n: " << std::endl;
	std::getline(std::cin, concatInfoFilePath);

	//make sure that the path the user provided is valid! If not, exit.

	//concatInfoFile.open("C:\\Users\\sk430\\Documents\\Visual Studio 2012\\Projects\\Concat-Convert\\Release\\concat_info.txt", std::ifstream::in);
	concatInfoFile.open(concatInfoFilePath, std::ifstream::in);


	std::getline(concatInfoFile, ACQFilePath);
	std::getline(concatInfoFile, DCLFilePath);


	//obtain ACQ files from specified directory
	std::vector<std::string> unsorted = listACQFiles(ACQFilePath);


	//check if there are ACQ files
	//if(unsorted.empty()) {		//if there are no ACQ files in the list
	//	std::cout << "Either the selected ACQ directory is invalid, or there are no ACQ files in the selected directory.\nPlease try again." << std::endl;
	//	//Sleep(3000);
	//	//return 0;
	//}

	//sort filenames by timestamp in filename
	std::vector<std::string> fnames = sortACQFilesTimestamp(unsorted);		//USE THIS to sort by filename's timestamp
	//std::vector<std::string> fnames = sortACQFilesFiletime(unsorted);		//USE THIS to sort by Windows' last-modified time
	
	//print out files in order -- make sure they're in order of date!
	//for(int i = 0; i < fnames.size(); i++) {
	//	std::cout << "FNAMES: " << fnames.at(i) << std::endl;
	//}


	//read next two lines of concat-info file, to determine which animals' data will be concatenated
	std::getline(concatInfoFile, animalsFromFile);		//user-specified animals to concat, or "All"
	std::getline(concatInfoFile, rightFormat);			//format of right-channel designation
	std::getline(concatInfoFile, leftFormat);			//format of left-channel designation

	//preparing to sort through animal data present in files, and separate files by
	//which animals' data they contain.
	std::vector<std::string> animals = listAnimals(fnames, animalsFromFile, rightFormat, leftFormat);//, DCLFilePath);		//obtain a list of animals in the ACQ files



	////TRUNCATING HERE FOR TESTING
	//animals.resize(10);
	////TRUNCATING HERE 



	for(int k = 0; k < animals.size(); k++) {
		currentAnimal = animals.at(k);
		currentAnimal_nodash = animals.at(k);
		currentAnimal_nodash.erase(std::remove(currentAnimal_nodash.begin(), currentAnimal_nodash.end(), '-'), currentAnimal_nodash.end());
		std::cout << "Current animal: " << currentAnimal;
		std::cout << "Current animal, no dash: " << currentAnimal_nodash;

		//obtain all ACQ filenames containing data for the current animal	
		fnames_animal.clear();		//clear old info first!
		fnames_animal = getFnamesAnimal(fnames, currentAnimal, currentAnimal_nodash);

		//get information necessary for writing DCL header
		scanFreq = getScanFreq(fnames_animal);			//get scan frequency (same for all files, usually 500 Hz)
		numDataPoints = getNumDataPoints(fnames_animal, currentAnimal, currentAnimal_nodash);	//get number of data points that will be written to DCL file
																								//i.e. total amount of data present in all ACQ files for this animal
		//write a DCL header for the DCL file
		writeDCLHeader(dclFile, scanFreq, numDataPoints, currentAnimal, DCLFilePath);

		//keeping track of data position
		long int dataCount = 1;											//starting point -- where in the data are we?
		int finalChannels = 2;											//will only be 2 channels in output file -- left and right!
		long int numPointsPerChannel = numDataPoints/finalChannels;
		long int segmentSize;

		//what are the channel indices of the channel we care about?
		int chindex_left;
		int chindex_right;

		std::string DCLInfoFileName = DCLFilePath + "\\" + currentAnimal + "_info.txt";
		std::ofstream dclInfoFile(DCLInfoFileName);

		//for each of the filenames containing BM animal's data, write the relevant animal's data to the new DCL file.
		for(int i = 0; i < fnames_animal.size(); i++) {
			fname_animal_str = fnames_animal.at(i);
			std::cout << "Current file being read and written: " << fname_animal_str << std::endl;
			fname_animal_w = stringToWchar(fname_animal_str);

			//open file
			if(initACQFile(fname_animal_w, &acqFile)) {

				//for each channel in file, search for BM-40 left/right.
				for(int j = 0; j < acqFile.numChannels; j++) {
					if(getChannelInfo(j, &acqFile, &chInfo)) { 
						wChannelName = chInfo.label;	
						strChannelName = wcharToString(wChannelName);						
						
						hoursInFile = chInfo.numSamples/(500*60*60);

						//exception: BM-21(l) is paired with BM-22(r) (typo). 
						//BM-22(r) ALWAYS immediately follows BM-21(l)
						//therefore index of BM-22(r) will always be 1 greater than index of BM-21(l)
						if(strChannelName.compare("BM-21(l)") == 0) {
							chindex_left = j;
							chindex_right = j+1;
						} else if(strChannelName.compare("BM-22(r)") == 0) {
							//do nothing
						} else {     //for all other animals, search for the appropriate channels
							if( (strChannelName.compare(currentAnimal + "(l)") == 0) || (strChannelName.compare(currentAnimal_nodash + "(l)") == 0)  ) chindex_left = j;
							else if( (strChannelName.compare(currentAnimal + "(r)") == 0 ) || (strChannelName.compare(currentAnimal_nodash + "(r)") == 0) ) chindex_right = j;
						}

					}
				}
				//write file info to DCL info file (text file)
				dclInfoFile << fname_animal_str.substr(fname_animal_str.find_last_of("/\\")+1) + ": " + std::to_string(hoursInFile) + " hours. \n";

				//number of points in one of the channels being read
				int numPointsPerChannel = chInfo.numSamples;

				while(dataCount < numPointsPerChannel) {
					if((numPointsPerChannel - dataCount) >= 1000000 ) {     //if there are enough points to large-size
						segmentSize = 1000000;
					} else {
						segmentSize = numPointsPerChannel - dataCount + 1;
						//std::cout << "\tSize of small chunk: " << segmentSize << std::endl;
					}

					writeSegment(dataCount, segmentSize, acqFile, dclFile, chindex_left, chindex_right);
					dataCount += segmentSize;
					//std::cout << "\tPosition: " << dataCount << ", out of " << numPointsPerChannel << std::endl;
				}
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