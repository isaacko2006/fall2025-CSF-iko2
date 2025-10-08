#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>

using namespace std;

int main( int argc, char **argv ) {
  // TODO: implement

  //check if trace file not provided, and print to c error as well as exiting with error
  if (argc < 2) {
    cerr << "No trace file provided";
    return 1;
  }

  //get file name from arguments passed in
  string file_name = argv[1];

  //attempt to open file, and exit with error if file can't be opened
  ifstream tracefile(file_name);
  if (!tracefile.is_open()) {
    cerr << "File was unable to be opened";
    return 1;
  }

  cout << "Reading trace file: " << file_name << endl;

  //variables to hold each line of trace file (load or save, hex address, and third arg that instructions said to ignore)
  string load_or_save;
  string hex_address;
  int extra_arg;

  unsigned long long num_lines = 0;

  //read until end of file reached
  while (tracefile >> load_or_save >> hex_address >> extra_arg) {
    num_lines++;
    
  //simple placeholder print statement for milestone 1 to make sure file read correctly
    cout << "Line: " << num_lines << " - Operation: " << load_or_save << " - Address: " << hex_address << endl;
  }

  cout << "Reading of file successful. " << num_lines << " number of lines read in total";

  //close file at end
  tracefile.close();

  return 0;
}
