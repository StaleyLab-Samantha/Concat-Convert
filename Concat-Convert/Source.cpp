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
	To run this script:
		1. Open the Visual Studio 2012 project file (Concat-Convert.sln) in Visual Studio 2012.
		2. Build the project. 
		3. Create a concatenation information TXT file. This file needs to be of a very specific format. 
			 - Line 1: Absolute path to ACQ files
			 - Line 2: Absolute path to DCL files
			 - Line 3: A list of animal names you wish to concatenate, or "all" if you wish to concatenate all animals.
			 - Line 4: The right-channel designation for all animals.
			 - Line 5: The left-channel designation for all animals. 
		4. In the Concat-Convert\Release folder, run Concat-Convert.exe. 
		5. Follow the instructions in the script. 
	For more detailed instructions, as well as an example with screenshots, see TODO CREATE DOCUMENT

	CONCAT-INFO TEXT FILE -- FURTHER INFO
	 - The file should be a simple Notepad text file, with extension ".txt"
	 - In Line 3: it is safer to specify "All" to search for all animals of the format you specified. 
		Otherwise, if you make a mistake while specifying animal names, the script may not be able to find any 
		of the animals you specified.
	 - In Line 3: when specifying animal-names:
			Be sure to include all whitespace at the end of the animal name! For example, if you have channel-names
				"EEG-028 Right"
				"EEG-028 Left"
				"EEG-030 Right"
				"EEG-030 Left"
			and you wish to concatenate both EEG-028's and EEG-030's data, lines 3, 4, and 5 should look like the following:
				EEG-028[SPACE],EEG-030[SPACE]
				Right
				Left
			where spaces are included ONLY where indicated by [SPACE]! Do NOT place the space at the beginning of the R/L
			designation instead.
	 - In Lines 4/5: when specifying R/L designation
			Note that R/L designation is CASE-SENSITIVE! Write the R/L designation EXACTLY as it appears in the channel-
			names of the files you wish to concatenate.


	PREREQUISITES:
	 - ACQ filenames must contain a timestamp at the end. Filenames must therefore be of the form 
		[beginning of name]YYYY-MM-DDTHH_MM_SS.acq
	 - Each animal may only have 2 channels of data -- a right and a left.
	 - Channel names must contain the name of the animal, followed by a designation indicating whether that
		channel contains right- or left-hemisphere data.
			Eg. if you have animal BM-54's right channel, and that animal's right-designation is "(r)", 
			the name of the channel should be BM-54(r). 
	 - The input ACQ files may contain up to 16 channels (the ACQ maximum). However, if a file contains a particular
		animal's data, that file should contain BOTH channels for that animal. There should not be a file with just
		ONE of the animal's data channels.

		TODO OTHER PREREQUISITES?

	OUTPUTS:
	 - One DCL file for each animal will be created in the output path provided by the user. 
		Each DCL file will be named according to the animal. For example, a file containing BM-54's data 
		will be named BM-54.dcl.
	 - Each DCL file will contain only two channels: a right-channel and a left-channel. These channels will be
		named according to the naming convention specified by the ACQ files. A channel labeled BM-54(r) in the 
		original ACQ files will also be labeled BM-54(r) in the resulting DCL file. 
	 - One TXT file for each animal will be created in the output path provided by the user. This file will contain further
		information about the file-concatenation -- which ACQ files were used to create the file, and which parts of the data
		in the DCL file correspond to the data in the ACQ files. 
	 - This TXT file will be named according to the animal as well. A TXTfile containing information about BM-54 will be named
		BM-54_info.txt. 

		TODO DOCUMENT OUTPUT LOG FILE FOR CONCATENATION PROCESS



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


//returns fnames sorted in alphabetical order!
std::vector<std::string> sortACQFilesAlphabetical(std::vector<std::string> fnames) { 
	//using list of names, create vector of pairs and sort by date
	std::vector<std::string> sorted_fnames(fnames);
	std::pair<std::string, std::string> file;

	//sort pair-vector by filetime
	std::sort(sorted_fnames.begin(), sorted_fnames.end());

	return sorted_fnames;
}




//TODO DOCUMENTATION
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

	std::size_t rEndPos;		//finding last index of r-designation
	std::size_t lEndPos;

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

					//find the animal's name (e.g. BM-40, BM-51...) minus it's R/L designation
					//std::size_t endPos = strChannelName.find("(");			//REMOVING 5/9/17
					rEndPos = strChannelName.find(rightFormat);		//finding last index of r-designation
					lEndPos = strChannelName.find(leftFormat);		//finding last index of l-designation

					//removing R/L portion, to obtain just the animal name (e.g. BM-50)
					if(rEndPos != std::string::npos)
						animalName = strChannelName.substr(0, rEndPos);			
					else if(lEndPos != std::string::npos)
						animalName = strChannelName.substr(0, lEndPos);	
					//if neither can be found, then keep animalName the way it is, including whatever R/L designation it has
					
					//if animal isn't already in the animal name list, add it. 	
					if (std::find(animals.begin(), animals.end(), animalName) == animals.end()) {
						if(animalName != "")	//make sure the string isn't empty!
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
		//what happens if there is whitespace in the animal name itself??
		//REMOVING in case there is whitespace in animal name
		//animalsFromFile.erase(std::remove( animalsFromFile.begin(), animalsFromFile.end(), ' '), animalsFromFile.end());

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

	}
	//remove BM-22 -- this is a typo! //REMOVING REMOVAL OF BM-22!
	//animals.erase(std::remove(animals.begin(), animals.end(), "BM-22"), animals.end());
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



//TODO DOCUMENTATION
//obtains a list of filenames containing data for a given animal

std::vector<std::string> getFnamesAnimal(std::vector<std::string> fnames, std::string currentAnimal, std::string currentAnimal_nodash, 
										 std::pair<std::string, std::string> currentAnimal_correction, std::string leftFormat, std::string rightFormat) {

//std::vector<std::string> getFnamesAnimal(std::vector<std::string> fnames, std::string currentAnimal, std::pair<std::string, std::string> currentAnimal_correction, 
//										 std::string leftFormat, std::string rightFormat) {
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
				//if((strChannelName.compare(currentAnimal + leftFormat) == 0) || (strChannelName.compare(currentAnimal_nodash + leftFormat) == 0)  
				//	|| (strChannelName.compare(currentAnimal + rightFormat) == 0) || (strChannelName.compare(currentAnimal_nodash + rightFormat) == 0)) {
				//	bm_flag = true;									//set flag to true
				//	//numDataPoints += numSamples;					//total data points = sum of all channels' data points
				//}

				//else if((strChannelName.compare(currentAnimal_correction.first + leftFormat) == 0) || (strChannelName.compare(currentAnimal_correction.first + rightFormat) == 0)) {
				//	bm_flag = true;									//set flag to true
				//	//numDataPoints += numSamples;					//total data points = sum of all channels' data points
				//	//return fnames_animal;
				//}

				//if((strChannelName.compare(currentAnimal + leftFormat) == 0) || (strChannelName.compare(currentAnimal_correction.first + leftFormat) == 0)  
				//	|| (strChannelName.compare(currentAnimal + rightFormat) == 0) || (strChannelName.compare(currentAnimal_correction.first + rightFormat) == 0)) {
				//	bm_flag = true;									//set flag to true
				//	//numDataPoints += numSamples;					//total data points = sum of all channels' data points
				//}

				if((strChannelName.compare(currentAnimal + leftFormat) == 0) || (strChannelName.compare(currentAnimal_correction.first + leftFormat) == 0) 
					|| (strChannelName.compare(currentAnimal + rightFormat) == 0) || (strChannelName.compare(currentAnimal_correction.first + rightFormat) == 0)) {					
						bm_flag = true;	
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


//TODO DOCUMENTATION
//obtains the scan frequency

//TODO: need to fix this! If file is broken, this will not work! Look through all files, break out of loop once a
//valid scanFreq value is found
int32_t getScanFreq(std::vector<std::string> fnames) {
	std::string fname_str;
	wchar_t *fname_w;
	ACQFile acqFile;
	int32_t scanFreq;

	////pick any of the ACQ files (picking the first one for convenience)
	//fname_str = fnames.at(0);
	//fname_w = stringToWchar(fname_str);		

	//if(initACQFile(fname_w, &acqFile))	{
	//	scanFreq = 1000.0/acqFile.sampleRate;		 			//Conversion from msec/sample to samples/sec (Hz).
	//	closeACQFile(&acqFile);
	//}

	for(int i=0; i < fnames.size(); i++) {
		//pick any of the ACQ files (picking the first one for convenience)
		fname_str = fnames.at(i);
		fname_w = stringToWchar(fname_str);		

		if(initACQFile(fname_w, &acqFile))	{
			scanFreq = 1000.0/acqFile.sampleRate;		 			//Conversion from msec/sample to samples/sec (Hz).
			closeACQFile(&acqFile);

			if(scanFreq > 0)	//if we have a valid scan frequency, break out of loop, return
				break;
		}
	}


	return scanFreq;

}


//TODO DOCUMENTATION
//obtains the number of data points in a given animal's DCL file
uint64_t getNumDataPoints(std::vector<std::string> fnames, std::string currentAnimal, std::string currentAnimal_nodash, 
						  std::pair<std::string, std::string> currentAnimal_correction, std::string leftFormat, std::string rightFormat) {
//uint64_t getNumDataPoints(std::vector<std::string> fnames, std::string currentAnimal, std::pair<std::string, std::string> currentAnimal_correction, 
//						  std::string leftFormat, std::string rightFormat) {
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

				//if((strChannelName.compare(currentAnimal + leftFormat) == 0) || (strChannelName.compare(currentAnimal_nodash + leftFormat) == 0)  
				//	|| (strChannelName.compare(currentAnimal + rightFormat) == 0) || (strChannelName.compare(currentAnimal_nodash + rightFormat) == 0)) {					
				//	numDataPoints += numSamples;					//total data points = sum of all channels' data points
				//}
				//else if((strChannelName.compare(currentAnimal_correction.first + leftFormat) == 0) || (strChannelName.compare(currentAnimal_correction.first + rightFormat) == 0)) {
				//	numDataPoints += numSamples;					//total data points = sum of all channels' data points
				//}

				if((strChannelName.compare(currentAnimal + leftFormat) == 0) || (strChannelName.compare(currentAnimal_correction.first + leftFormat) == 0) 
					|| (strChannelName.compare(currentAnimal + rightFormat) == 0) || (strChannelName.compare(currentAnimal_correction.first + rightFormat) == 0)) {					
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
void writeDCLHeader(std::fstream &file, int32_t &scanFreq, uint64_t &numDataPoints, std::string &animal, std::string &leftFormat, std::string &rightFormat, std::string DCLFilePath) {

	std::string animal_nospace(animal);
	animal_nospace.erase(std::remove(animal_nospace.begin(), animal_nospace.end(), ' '), animal_nospace.end());

	std::string DCLFileName = DCLFilePath + "\\" + animal_nospace + ".dcl";
	//std::string DCLFileName = DCLFilePath + "\\" + animal + ".dcl";
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

	//OLD -- specific to BM dataset only!
	//leftChannelName = animal + "(l)";
	//rightChannelName = animal + "(r)";

	//For a channel-name identical to the channel-names in the ACQ input files. 
	//leftChannelName = animal + leftFormat;
	//rightChannelName = animal + rightFormat;

	//For a channel name consisting of just leftFormat/rightFormat
	//Necessary if you want to blind the files afterward!
	leftChannelName = leftFormat;
	rightChannelName = rightFormat;
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


//TODO DOCUMENTATION
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

//TODO only allows one correction per animal, be sure to document this!
std::vector<std::pair<std::string, std::string>> getAnimalCorrections(std::ifstream &concatInfoFile) {
	std::vector<std::pair<std::string, std::string>> corrections;
	std::string line;

	std::string wrongAnimalName;
	std::string nameCorrection;

	size_t pos;

	//for each remaining line of concatInfoFile
	while(std::getline(concatInfoFile, line)) {
		//"||" will separate old from new!
		pos = line.find("||");
		wrongAnimalName = line.substr(0, pos);
		nameCorrection = line.substr(pos+2);	//pos+1 if single character, pos+2 b/c 2 characters in "||"

		//add pair to list
		corrections.push_back(std::make_pair(wrongAnimalName, nameCorrection));
	}

	return corrections;
}



//TODO organize and document all variable definitions, move all to top
int main(int argc, char* argv[]) {

	//filename variables for conversion between std::string and wchar_t
	//BioPAC ACQ API requires wchar_t
	std::string fname_str, fname_animal_str;
	wchar_t *fname_w, *fname_animal_w;
	
	//filestream objects and ACQ file handles
	ACQFile acqFile;
	std::fstream dclFile;
	std::ofstream dclInfoFile, logFile;
	std::ifstream concatInfoFile;
	//std::fstream brokenFilesList;

	//ACQ channel-related variables
	CHInfo chInfo;
	wchar_t* wChannelName;
	std::string strChannelName;

	//relevant variables obtained from ACQ header
	//Important for writing DCL header
	int32_t scanFreq = 0;
	int numSamples = 0;								//number of samples in a channel
	int channelsCount = 1;							//must be at least one channel in file
	uint64_t numDataPoints = 0;						//total number of data points in DCL file being created

	double hoursInACQFile;							//number of hours of a particular animal's data in a given file.
													//Stored in text file accompanying each DCL, so user knows how much
													//of DCL data comes from each ACQ file
	
	bool bm_flag = false;							//flag indicates if a particular ACQ file contains data for the
													//current animal

	//filepath variables for filestreams
	std::string concatInfoFilePath, ACQFilePath, DCLFilePath;

	//current-animal variables
	std::vector<std::string> filesToRemove;									//ACQ files that do NOT contain data for the current animal
																			//These filenames are to be removed from fnames_animal
	
	std::string currentAnimal, currentAnimal_nodash, currentAnimal_nospace;	//variants on currentAnimal, with dashes/spaces removed
	std::pair<std::string, std::string> currentAnimal_correction;			//user-supplied corrected version of a given animal name
	std::vector<std::string> fnames_animal;									//list of filenames containing data for the current animal
	
	
	//information obtained from the user
	std::string animalsFromFile, animalFormat, rightFormat, leftFormat;		//provided via the concatenation-info text file the user provides
																			//specify the animals the user wants to concat, the format of 
																			//right/left channel designation, etc.
	//user's response to prompts
	std::string response;


	//Prompt user, reminding them to fill out text file. Ask if they've filled it out, exit if no.
	std::cout << "This script will concatenate ACQ files, and convert them to DCL files.\nBe sure to complete a concatenation-info text file before proceeding.";
	std::cout << "Press Ctrl-C at any point to stop this script.\n" << std::endl;

	//Prompt user for path where concat_info file is located
 	std::cout << "Enter the full path where your concatenation-info text file is located, including the file name.\nThis path should be of the form C:\\...\\...\\concat_info.txt: \n" << std::endl;
	std::getline(std::cin, concatInfoFilePath);

	//make sure that the path the user provided is valid! If not, exit.
	concatInfoFile.open(concatInfoFilePath, std::ifstream::in);
	if(!concatInfoFile) {	//if stream has failed, notify user that they provided an incorrect path.
		std::cout << "Failed to open file at: " << concatInfoFilePath << ".\nCheck that the path provided is correct. Enter \"Q\" to quit, and try again." << std::endl;
		std::getline(std::cin, response);

		//if user answers that list is not correct, end concatenation and notify the user.
		if(icompare(response, "q") || icompare(response, "quit")) {	
			std::cout << "\nQuitting program..." << std::endl;
			Sleep(3000);	//give user time to see message
			return 0;
		}
	}

	//ifstream opened successfully -- obtain first two lines of concat-info text file 
	//(path to ACQ folder and DCL folder respectively)
	std::getline(concatInfoFile, ACQFilePath);
	std::getline(concatInfoFile, DCLFilePath);


	//show user the paths they entered -- are these paths correct?
	std::cout << "\nACQ-file and DCL-file paths obtained from concatenation-info file." << std::endl;
	std::cout << "\tACQ File Path: " << ACQFilePath << std::endl;
	std::cout << "\tDCL File Path: " << DCLFilePath << std::endl;
	std::cout << "Are these paths correct? (Y/N)\n" << std::endl;
	std::getline(std::cin, response);

	//if user answers that paths are not correct, end concatenation and notify the user.
	if(icompare(response, "n") || icompare(response, "no")) {	//if response is N/n/no/NO/No/nO
		std::cout << "You indicated this information is not correct. Stopping concatenation." << std::endl;
		Sleep(3000);	//give user time to see message
		return 0;
	}	
	
	//warn user if there are already DCL files in the concat-output folder
	WIN32_FIND_DATA data;
	HANDLE hFind;
	wchar_t *path_w;

	//create search-string: look for ACQ files in the given directory 
	path_w = stringToWchar(DCLFilePath + "\\*.dcl");
	hFind = FindFirstFile(path_w, &data);

	//if at least one DCL file is found in the specified DCL output path, notify the user!
	//they may be overwriting data with this concat.
	if (hFind != INVALID_HANDLE_VALUE) 
	{
		std::cout << "\nExisting DCL files were found in the specified DCL file path." << std::endl;
		std::cout << "Concatenation may overwrite existing DCL files and log files, and will do so without warning. \nAre you sure you wish to proceed? (Y/N)\n" << std::endl;
		std::getline(std::cin, response);

		//if user answers that paths are not correct, end concatenation and notify the user.
		if(icompare(response, "n") || icompare(response, "no")) {	//if response is N/n/no/NO/No/nO
			std::cout << "\nYou indicated you do not wish to proceed. Stopping concatenation." << std::endl;
			Sleep(3000);	//give user time to see message
			return 0;
		}
		FindClose(hFind);
	} 


	//open log-file at DCLFilePath -- same destination as DCL files and DCL-info text files
	logFile.open(DCLFilePath + "\\concat-log-file.txt", std::ofstream::out);
	logFile << "CONCATENATION STARTED\n\n" << std::endl;
	logFile << "INPUTS AND SETTINGS" << std::endl;
	logFile << "User provided the following concat-info file path: \n" << concatInfoFilePath << std::endl;
	logFile << "\tACQ file path: \n" << ACQFilePath << std::endl;
	logFile << "\tDCL file path: \n" << DCLFilePath << std::endl;

	//Notify user that a list of animals is being obtained. If there are many files, this may take a couple of minutes.
	std::cout << "\nObtaining a list of animals (without corrections). If you have a large number of ACQ files, this may take a couple of minutes...\n" << std::endl;
	
	//obtain ACQ files from specified directory
	std::vector<std::string> unsorted = listACQFiles(ACQFilePath);

	//check if there are ACQ files
	if(unsorted.empty()) {		//if there are no ACQ files in the list
		std::cout << "Either the selected ACQ directory is invalid, or there are no ACQ files in the selected directory.\nEnter \"Q\" to quit, and try again." << std::endl;
		logFile << "Either the selected ACQ directory is invalid, or there are no ACQ files in the selected directory.\nEnter \"Q\" to quit, and try again." << std::endl;
		std::getline(std::cin, response);

		//if user answers that list is not correct, end concatenation and notify the user.
		if(icompare(response, "q") || icompare(response, "quit")) {	//if response is N/n/no/NO/No/nO
			std::cout << "\nQuitting program..." << std::endl;
			logFile << "\n\nQuitting. Concatenation Unsuccessful. " << std::endl;
			Sleep(3000);	//give user time to see message
			return 0;
		}		
	}

	//read next two lines of concat-info file, to determine which animals' data will be concatenated
	std::getline(concatInfoFile, animalsFromFile);		//user-specified animals to concat, or "All"
	std::getline(concatInfoFile, rightFormat);			//format of right-channel designation
	std::getline(concatInfoFile, leftFormat);			//format of left-channel designation

	//preparing to sort through animal data present in files, and separate files by
	//which animals' data they contain.
	std::vector<std::string> animals = listAnimals(unsorted, animalsFromFile, rightFormat, leftFormat);//, DCLFilePath);		//obtain a list of animals in the ACQ files

	//obtaining animal corrections
	std::vector<std::pair<std::string, std::string>> animal_corrections;
	animal_corrections = getAnimalCorrections(concatInfoFile);

	//if list is empty, notify user and end program
	if(animals.empty()) {		//if there are no ACQ files in the list
		std::cout << "No animals were found in the provided ACQ files with the specified format.\nEnter \"Q\" to quit, and try again." << std::endl;
		std::getline(std::cin, response);

		//if user answers that list is not correct, end concatenation and notify the user.
		if(icompare(response, "q") || icompare(response, "quit")) {	//if response is N/n/no/NO/No/nO
			std::cout << "\nQuitting program..." << std::endl;
			logFile << "\n\nQuitting. Concatenation Unsuccessful. " << std::endl;
			Sleep(3000);	//give user time to see message
			return 0;
		}
	}

	//provide the user with the list of animals to be concatenated, confirm that this is correct.
	std::cout << "\n---------------------\n" << std::endl;
	std::cout << "Animal-finding complete! The following is a list of the animals you wish to concatenate (without corrections):\n" << std::endl;
	logFile << "Animals found in ACQ files: " << std::endl;
	std::sort(animals.begin(), animals.end());		//sorting list of animals before displaying
	for(int i = 0; i < animals.size(); i++) {
		std::cout << animals.at(i) << " ";
		logFile << animals.at(i) << " ";
	}
	//getting corrections, show them to user
	std::cout << "\n\nThe following is a list of corrections to make to the above animal-list:\n" << std::endl;
	logFile << "\n\nThe following is a list of corrections to make to the above animal-list:\n" << std::endl;
	
	if(animal_corrections.size() == 0) {  //if no corrections, print "no corrections"
		std::cout << "\tNo corrections found." << std::endl;
		logFile << "\tNo corrections found." << std::endl;
	}
	else {	//print corrections if there are any
		for(int i = 0; i < animal_corrections.size(); i++) {
			std::cout << "\t" << i+1 << ". " << animal_corrections.at(i).first << " will be corrected to: " << animal_corrections.at(i).second << std::endl;
		}
	}
	//remove animal-names that represent typos
	//for(int i = 0; i < animal_corrections.size(); i++) {
	//	animals.erase(std::remove(animals.begin(), animals.end(), animal_corrections.at(i).first), animals.end());
	//}

	//removing mistaken animal names from list, adding correct animal names (if needed)
	std::pair<std::string, std::string> corr;
	for(int i = 0; i < animal_corrections.size(); i++) {
		//get a correction from the list
		corr = animal_corrections.at(i);

		//erase corr.first (ie. an animal-name typo) from the animal-list
		animals.erase(std::remove(animals.begin(), animals.end(), corr.first), animals.end());

		//if the correction for an animal isn't present in the list, add it
		if (std::find(animals.begin(), animals.end(), corr.second) == animals.end()) {
			if(corr.second != "")	//make sure the string isn't empty!
				animals.push_back(corr.second);	
		}
	}

	//sort after removal-replacing of incorrect animal names
	std::sort(animals.begin(), animals.end());

	//show corrected list to user, ask if it's correct
	std::cout << "\n\nThe following is the corrected animal-list:\n" << std::endl;
	logFile << "The following is the corrected animal-list:" << std::endl;
	for(int i = 0; i < animals.size(); i++) {
		std::cout << animals.at(i) << " ";
		logFile << animals.at(i) << " ";
	}
	std::cout << "\n\nIs this correct? (Y/N)\n" << std::endl;
	logFile << "\nIs this correct? (Y/N)\n" << std::endl;
	std::getline(std::cin, response);
	logFile << "\tUser Response: " << response << std::endl;

	//if user answers that list is not correct, end concatenation and notify the user.
	if(icompare(response, "n") || icompare(response, "no")) {	//if response is N/n/no/NO/No/nO
		std::cout << "You indicated this information is not correct. Stopping concatenation.\nEnter \"Q\" to quit, and try again." << std::endl;
		logFile << "\tYou indicated this information is not correct. Stopping concatenation.\nEnter \"Q\" to quit, and try again." << std::endl;
		std::getline(std::cin, response);

		//if user answers that list is not correct, end concatenation and notify the user.
		if(icompare(response, "q") || icompare(response, "quit")) {	//if response is N/n/no/NO/No/nO
			std::cout << "\nQuitting program..." << std::endl;
			logFile << "\n\nQuitting. Concatenation Unsuccessful. " << std::endl;
			Sleep(3000);	//give user time to see message
			return 0;
		}
	}
	//if user gives any other response besides some variant of "No"/"N", proceed

	//allow user to choose file-sorting method
	logFile << "User choosing file-sorting method..." << std::endl;
	std::cout << "\nNext, choose a sorting method to sort your ACQ files by date. This is the order in which your files will be concatenated:\n";
	std::cout << "\tOption 1: By timestamp in filename, formatted YYYY_MM_DDT_HH_MM_SS (recommended)" << std::endl;
	std::cout << "\tOption 2: By Windows' \"Last-Modified\" time" << std::endl;
	std::cout << "\tOption 3: In alphabetical order of filename (not recommended)" << std::endl;
	std::cout << "Pick one of Options 1, 2, or 3 by entering \"1\", \"2\", or \"3\" now.\n" << std::endl;
	std::getline(std::cin, response);

	std::vector<std::string> fnames;	//sorted filenames
	//sort filenames
	if(response == "1") {
		fnames = sortACQFilesTimestamp(unsorted);		//USE THIS to sort by filename's timestamp
		logFile << "\tUser selected Option 1: Sorting by Filename Timestamp (recommended)" << std::endl;
	}
	else if(response == "2") {
		fnames = sortACQFilesFiletime(unsorted);		//USE THIS to sort by Windows' last-modified time
		logFile << "\tUser selected Option 2: Sorting by Windows Last-Modified Time" << std::endl;
	}
	else if(response == "3") {
		fnames = sortACQFilesAlphabetical(unsorted);	//USE THIS to sort alphabetically. TODO TEST
		logFile << "\tUser selected Option 3: Alphabetical Filename Sort (not recommended)" << std::endl;
	}
	else
	{
		std::cout << "Invalid input for sorting method.\nEnter \"Q\" to quit, and try again." << std::endl;
		logFile << "Invalid input for sorting method.\nEnter \"Q\" to quit, and try again." << std::endl;
		std::getline(std::cin, response);

		//if user answers that list is not correct, end concatenation and notify the user.
		if(icompare(response, "q") || icompare(response, "quit")) {	//if response is N/n/no/NO/No/nO
			std::cout << "\nQuitting program..." << std::endl;
			logFile << "\n\nQuitting. Concatenation Unsuccessful. " << std::endl;
			Sleep(3000);	//give user time to see message
			return 0;
		}
	}

	
	//tell user they can leave, since all user-input has finished
	std::cout << "\nConcatenation started -- this process may take a few hours. No further input is required. ";
	std::cout << "This window will close when concatenation is complete. \nA log file is being written to " << DCLFilePath  << "\n" << std::endl;

	////TRUNCATING HERE FOR TESTING
	//animals.resize(10);
	////TRUNCATING HERE 


	//TODO CONSISTENT NAMING SCHEME: all to camelcase, no underscores!

	logFile << "\n\nREADING AND WRITING" << std::endl;

	//beginning concatenation
	for(int k = 0; k < animals.size(); k++) {
		currentAnimal = animals.at(k);
		currentAnimal_nodash = animals.at(k);
		currentAnimal_nospace = animals.at(k);
		currentAnimal_correction = std::make_pair("", "");

		//obtain user-supplied correction for current animal, if there is one
		for(int m = 0; m < animal_corrections.size(); m++) {
			//if the current animal name is an error, or is associated with an error
			if((currentAnimal == animal_corrections.at(m).first) || (currentAnimal == animal_corrections.at(m).second))
				currentAnimal_correction = animal_corrections.at(m);
		}

		//create versions of currentAnimal without dashes, spaces
		currentAnimal_nodash.erase(std::remove(currentAnimal_nodash.begin(), currentAnimal_nodash.end(), '-'), currentAnimal_nodash.end());
		currentAnimal_nospace.erase(std::remove(currentAnimal_nospace.begin(), currentAnimal_nospace.end(), ' '), currentAnimal_nospace.end());
		
		//print current animal so that user can watch progress of program
		std::cout << "\n---------------------\n\nCurrent animal's file being created: " << currentAnimal_nospace << ".dcl" << std::endl;
		logFile << "\n---------------------\n\nCurrent animal's file being created: " << currentAnimal_nospace << ".dcl" << std::endl;
		//std::cout << "Current animal, no dash: " << currentAnimal_nodash;

		//obtain all ACQ filenames containing data for the current animal	
		fnames_animal.clear();		//clear old info first!
		fnames_animal = getFnamesAnimal(fnames, currentAnimal, currentAnimal_nodash, currentAnimal_correction, leftFormat, rightFormat);
		//fnames_animal = getFnamesAnimal(fnames, currentAnimal, currentAnimal_correction, leftFormat, rightFormat);

		//notify user that animal's data was not found, and skip to next animal
		if(fnames_animal.empty()) {		//if there are no ACQ files in the list for this animal
			std::cout << "No ACQ files containing " << currentAnimal << "'s data were found. Please check that the format specified is correct.\n"<< std::endl;
			logFile << "No ACQ files containing " << currentAnimal << "'s data were found. Please check that the format specified is correct.\n"<< std::endl;
			continue;					//do not execute the rest, skip to next animal
		}

		//get information necessary for writing DCL header
		scanFreq = getScanFreq(fnames_animal);			//get scan frequency (same for all files, usually 500 Hz)
		numDataPoints = getNumDataPoints(fnames_animal, currentAnimal, currentAnimal_nodash, currentAnimal_correction, leftFormat, rightFormat);	//get number of data points that will be written to DCL file																												
		//numDataPoints = getNumDataPoints(fnames_animal, currentAnimal, currentAnimal_correction, leftFormat, rightFormat);	//get number of data points that will be written to DCL file
																															//i.e. total amount of data present in all ACQ files for this animal	
		//write a DCL header for the DCL file
		writeDCLHeader(dclFile, scanFreq, numDataPoints, currentAnimal, leftFormat, rightFormat, DCLFilePath);

		//keeping track of data position
		long int dataCount = 1;											//starting point -- where in the data are we?
		int finalChannels = 2;											//will only be 2 channels in output file -- left and right!
		long int numPointsPerChannel = numDataPoints/finalChannels;
		long int segmentSize;

		//what are the channel indices of the channel we care about?
		int chindex_left;
		int chindex_right;

		//creating stream for DCL-info text file. Contains information about which hours of data came from which ACQ file.
		std::string DCLInfoFileName = DCLFilePath + "\\" + currentAnimal_nospace + "_info.txt";
		std::ofstream dclInfoFile(DCLInfoFileName);
		//totalHoursInDCL = 0;

		//for each of the filenames containing BM animal's data, write the relevant animal's data to the new DCL file.
		for(int i = 0; i < fnames_animal.size(); i++) {
			//conversion from str to wchar_t for BioPAC ACQ API
			fname_animal_str = fnames_animal.at(i);
			//so user can see progress of program
			std::cout << "Current ACQ file being read: " << fname_animal_str << std::endl;		//TODO PRINT STATEMENT NEEDS TO BE HERE??? FOR ACQ FILES TO OPEN
			logFile <<"Current ACQ file being read: " << fname_animal_str << std::endl;			//MAY 24 SAMANTHA -- WATCH OUT, MESSING WITH PRINT STATEMENT

			fname_animal_w = stringToWchar(fname_animal_str);
			

			//open ACQ file
			if(initACQFile(fname_animal_w, &acqFile)) {

				//so user can see progress of program
				std::cout << "\tFile-open successful." << std::endl;
				logFile << "\tFile-open successful." << std::endl;

				//for each channel in file, search for the animal's left/right channels.
				for(int j = 0; j < acqFile.numChannels; j++) {
					if(getChannelInfo(j, &acqFile, &chInfo)) { 
						wChannelName = chInfo.label;	
						strChannelName = wcharToString(wChannelName);						
						
						hoursInACQFile = chInfo.numSamples/(scanFreq*60.0*60.0);
						//totalHoursInDCL += hoursInACQFile;


						/*
						//TODO REMOVE BM-21/BM-22 EXCEPTION

						//exception: BM-21(l) is paired with BM-22(r) (typo). 
						//BM-22(r) ALWAYS immediately follows BM-21(l)
						//therefore index of BM-22(r) will always be 1 greater than index of BM-21(l)
						*/
						//if(strChannelName.compare("BM-21(l)") == 0) {
						//	chindex_left = j;
						//	chindex_right = j+1;
						//} else if(strChannelName.compare("BM-22(r)") == 0) {
						//	//do nothing
						//} else {     //for all other animals, search for the appropriate channels
						//	//if( (strChannelName.compare(currentAnimal + "(l)") == 0) || (strChannelName.compare(currentAnimal_nodash + "(l)") == 0)  ) chindex_left = j;
						//	//else if( (strChannelName.compare(currentAnimal + "(r)") == 0 ) || (strChannelName.compare(currentAnimal_nodash + "(r)") == 0) ) chindex_right = j;
						//	if( (strChannelName.compare(currentAnimal + leftFormat) == 0) || (strChannelName.compare(currentAnimal_nodash + leftFormat) == 0) || (strChannelName.compare(currentAnimal_correction.first + leftFormat) == 0) ) chindex_left = j;
						//	else if( (strChannelName.compare(currentAnimal + rightFormat) == 0 ) || (strChannelName.compare(currentAnimal_nodash + rightFormat) == 0) || (strChannelName.compare(currentAnimal_correction.first + rightFormat) == 0) ) chindex_right = j;
						//}

						//if( (strChannelName.compare(currentAnimal + "(l)") == 0) || (strChannelName.compare(currentAnimal_nodash + "(l)") == 0)  ) chindex_left = j;
						//else if( (strChannelName.compare(currentAnimal + "(r)") == 0 ) || (strChannelName.compare(currentAnimal_nodash + "(r)") == 0) ) chindex_right = j;
						if( (strChannelName.compare(currentAnimal + leftFormat) == 0) || (strChannelName.compare(currentAnimal_nodash + leftFormat) == 0) || (strChannelName.compare(currentAnimal_correction.first + leftFormat) == 0) ) chindex_left = j;
						else if( (strChannelName.compare(currentAnimal + rightFormat) == 0 ) || (strChannelName.compare(currentAnimal_nodash + rightFormat) == 0) || (strChannelName.compare(currentAnimal_correction.first + rightFormat) == 0) ) chindex_right = j;
						


					}
				}
				//write file info to DCL info file (text file)
				dclInfoFile << fname_animal_str.substr(fname_animal_str.find_last_of("/\\")+1) + ": " + std::to_string(hoursInACQFile) + " hours. \n";

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
				//so user can see progress of program
				std::cout << "\tData-write from file is complete." << std::endl;
				logFile << "\tData-write from file is complete." << std::endl;
				dataCount = 1;
			}
			else {	//if opening ACQ file failed with BioPAC API
				//so user can see progress of program
				std::cout << "\tFile-open failed." << std::endl;
				logFile << "\tFile-open failed." << std::endl;
			}
		}

		//write to info file about 
		dclInfoFile << "\n\nTOTAL AMOUNT OF DATA IN FILE: " + std::to_string(numDataPoints/(500.0*60*60*2)) + " hours. \n";

		const int32_t timestampCount = 0;	//timestamps appear to be made up of 2 numbers
		dclFile.write((char*)&timestampCount, sizeof(int32_t));

		const int32_t ttlstampCount = 0;	//timestamps appear to be made up of 2 numbers
		dclFile.write((char*) &ttlstampCount, sizeof(int32_t));

		dclFile.close();
		numDataPoints = 0;
	}

	Sleep(5000);


	//TODO concatenation-is-complete message
	//TODO user presses "quit" or something, then return, so that user can see output...?

	std::cout << "\n\nPROCESS COMPLETE. Enter \"Q\" to quit.\n" << std::endl;
	logFile <<"\n\nPROCESS COMPLETE. Enter \"Q\" to quit.\n" << std::endl;
	std::getline(std::cin, response);

	//if user answers that list is not correct, end concatenation and notify the user.
	if(icompare(response, "q") || icompare(response, "quit")) {	//if response is N/n/no/NO/No/nO
		std::cout << "\nQuitting program..." << std::endl;
		logFile <<"\n\nQuitting program. Concatenation Successful." << std::endl;
		Sleep(3000);	//give user time to see message
		return 0;
	}
	//return 0;

}