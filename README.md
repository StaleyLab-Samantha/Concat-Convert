# Concat-Convert

A project that, given a set of `.ACQ` EEG recordings, will create `.DCL` files such that one animal's data is temporally-concatenated into a single file. This is a Visual Studio 2012 project.

The result: One `.DCL` file for every animal present in the set of `.ACQ` files. Each `.DCL` file will contain 2 channels (ie. the left- and right-channel data for each animal). 

# How to Run

**(1) To concatenate the contents of a collection of ACQ files, copy these ACQ files to the following location: `Concat-Convert\Debug`**

This places the ACQ files in the same directory as the Concat-Convert.exe executable (allowing the script to detect the files). 

NOTE: I have attempted to read ACQ files from a user-selectable directory. So far, this hasn’t worked. For now, place the ACQ files in the “Debug” directory of the `Concat-Convert` project as described above. 

**(2) Next, use the command prompt to execute the script which will concatenate and convert these ACQ files to DCL files for each animal.** 

In the command prompt window, navigate to `Concat-Convert\Debug` (the directory containing the executable). 

Run the executable: `Concat-Convert.exe` (no additional arguments necessary). 

**(3) The script will prompt you for the path where the ACQ files are located. Enter the absolute path to these files (eg. C:\Users\...) **

**(4) The script will then prompt you for the path where you would like the DCL files to be created. Select a path where you would like the DCL files to be created. Again, enter an absolute path.**

**(5) Wait for the files to be created in the directory. This will take a long time – about 1 minute per 8-hours of data per animal. It could be hours, depending on how much data you are concatenating.**

Once the files have been created, test them by opening them in dClamp. Note that the old version of dClamp may not be able to handle the sizes of these files. 

My modified version of dClamp should be able to, however, as long as you change the file extension to `.DCLT` or `.DCLU` (`.DCLT` if you want dClamp to automatically iterate through segments, `.DCLU` if you want to interact with the data and the results of the analysis, or if you want to control when dClamp moves from segment to segment). 
