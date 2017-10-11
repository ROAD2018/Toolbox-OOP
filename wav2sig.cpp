//
//File: wav2sig.cpp
//Purpose: opens wavfile, maipulates data for use in the Array Toolbox
//Author: Grant Cox, University of Kentucky Distributed Audio Lab, grant.cox@uky.edu
//        (process adapted from
//        https://stackoverflow.com/questions/13660777/c-reading-the-data-part-of-a-wav-file)
//

#include <fstream>
#include <iostream>
#include <string>
#include <cstdint>


/*******************************************************************************
struct: WAV_HEADER
data: allocation for each byte of data in a general .wav file.
*/
typedef struct WAV_HEADER {
     /* RIFF Chunk Descriptor */
    uint8_t         RIFF[4];        // RIFF Header Magic header
    uint32_t        ChunkSize;      // RIFF Chunk Size
    uint8_t         WAVE[4];        // WAVE Header
    /* "fmt" sub-chunk */
    uint8_t         fmt[4];         // FMT header
    uint32_t        Subchunk1Size;  // Size of the fmt chunk
    uint16_t        AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw,
                                    //257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    uint16_t        NumOfChan;      // Number of channels 1=Mono 2=Sterio
    uint32_t        SamplesPerSec;  // Sampling Frequency in Hz
    uint32_t        bytesPerSec;    // bytes per second
    uint16_t        blockAlign;     // 2=16-bit mono, 4=16-bit stereo
    uint16_t        bitsPerSample;  // Number of bits per sample
    /* "data" sub-chunk */
    uint8_t         Subchunk2ID[4]; // "data"  string
    uint32_t        Subchunk2Size;  // Sampled data length
} wav_header;




/*******************************************************************************
Function: getFileSize
Purpose: get the size of the wave file opened in the program
@param: FILE pointer to the wave file
@return: integer, the size of the wavefile
*/
int getFileSize(FILE * inFile) {
     fseek(inFile, 0, SEEK_END);
     unsigned int filesize = ftell(inFile);
     fseek(inFile, 0, SEEK_SET);
     return filesize;
}




/*******************************************************************************
Function: outputFileData
Purpose: prints data about the selected .wav file to the command line
@param: a wav_header struct by reference, a FILE pointer, integer number of samples
@return: void
*/
void outputFileData(wav_header & wavHeader, FILE * wavFile, int numSamples) {
     std::cout << "\n\nComputing/Outputting Metadata\n";
     int filelength = getFileSize(wavFile);

     std::cout << "File is                    :" << filelength << " bytes." << std::endl;
     std::cout << "RIFF header                :" << wavHeader.RIFF[0] << wavHeader.RIFF[1]
          << wavHeader.RIFF[2] << wavHeader.RIFF[3] << std::endl;
     std::cout << "WAVE header                :" << wavHeader.WAVE[0]
          << wavHeader.WAVE[1] << wavHeader.WAVE[2] << wavHeader.WAVE[3] << std::endl;
     std::cout << "FMT                        :" << wavHeader.fmt[0] << wavHeader.fmt[1]
          << wavHeader.fmt[2] << wavHeader.fmt[3] << std::endl;
     std::cout << "Data size                  :" << wavHeader.ChunkSize << std::endl;

     // Display the sampling Rate from the header
     std::cout << "Sampling Rate              :" << wavHeader.SamplesPerSec << std::endl;
     std::cout << "Number of bits used        :" << wavHeader.bitsPerSample << std::endl;
     std::cout << "Number of channels         :" << wavHeader.NumOfChan << std::endl;
     std::cout << "Number of bytes per second :" << wavHeader.bytesPerSec << std::endl;
     std::cout << "Number of samples          :" << numSamples << std::endl;
     std::cout << "Data length                :" << wavHeader.Subchunk2Size << std::endl;
     std::cout << "Audio Format               :" << wavHeader.AudioFormat << std::endl;
     // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM

     std::cout << "Block align                :" << wavHeader.blockAlign << std::endl;
     std::cout << "Data string                :" << wavHeader.Subchunk2ID[0]
          << wavHeader.Subchunk2ID[1] << wavHeader.Subchunk2ID[2]
          << wavHeader.Subchunk2ID[3] << std::endl;
}




/*******************************************************************************
Function: Main
Purpose: Driver of wav2sig.cpp
@param: TODO
@return: integer exit status
*/
int main(int argc, char* argv[]) {
     std::cout << "Attempting to open " << argv[1] << "..." << std::endl;
     wav_header wavHeader;
     int headerSize = sizeof(wav_header);

     const char* filepath = argv[1];

     //attempt to open the wavefile provided by the caller
     FILE * wavFile = fopen(filepath,"r");
     if (wavFile == nullptr) {
          std::cerr << "Unable to open file " << argv[1] << std::endl;
          return 1;
     }

     //read in the wave file by the header
     size_t bytesRead = fread(&wavHeader,1, headerSize,wavFile);
     std::cout << "Header read " << bytesRead << " bytes." << std::endl;

     //check for empty file
     if (bytesRead <= 0) {
          std::cerr << "Empty wav file. Exiting..." << std::endl;
          return 1;
     }

     //read the data
     uint16_t bytesPerSample = wavHeader.bitsPerSample / 8;  //Number of bytes per sample
     uint64_t numSamples = wavHeader.ChunkSize / bytesPerSample; //How many samples are in the wav file
     static const uint16_t BUFFER_SIZE = 4096;
     int8_t * buffer = new int8_t[BUFFER_SIZE];


     double data[numSamples]; //array for sample amplitude
     int sampleCounter = 0;   //counter for array indexing (sampling)
     while ( (bytesRead = fread(buffer, sizeof buffer[0], BUFFER_SIZE / (sizeof buffer[0]) , wavFile) ) > 0) {
          std::cout << "Read\r";

          //increment over bytes read into the buffer
          for(unsigned int i = 0; i < bytesRead; i+=2) { // bytesRead = 256 (frame size)
               int c = buffer[i+1] << 8 | buffer[i]; //combine 2 bytes into 16 bit sample size
               double d = c / 32768.0;               //convert to double floating point
               data[sampleCounter] = d;              //store into array of data
               sampleCounter++;                      //increment counter
          }
     }
     std::cout << "\nNumber of samples: " << sampleCounter << std::endl;

     delete [] buffer; buffer = nullptr; //remove buffer

     outputFileData(wavHeader,wavFile, numSamples);    //report data to command line


     std::cout << "exiting..." << std::endl;
     fclose(wavFile);
     return 0;
}