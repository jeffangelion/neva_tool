/* Copyright © 2020,2021 Ivan Vatlin <jenrus@riseup.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iconv.h> // Shift-JIS <-> UTF-8
#include <math.h> // pow()
#include <stdbool.h> // bool, true, false
#include <stdio.h> // printf(); fopen(); fseek(); ftell(); fread(); fclose(); rewind()
#include <stdlib.h> // exit()
#include <string.h> // memcpy(); strlen()

// Constants
const char BPK0_HEADER[] = "BPK0";
const char BDL0_HEADER[] = "BDL0";
const unsigned short HEADER_SIZE = 4;
const unsigned short FILE_ENTRY_NAME_SIZE = 64;
const unsigned short FILE_RECORD_SIZE = 16;
const unsigned short FOLDER_ENTRY_NAME_OFFSET = 20;

// Convert little-endian hex array to decimal number
int convertHexToDec(const unsigned char input[], const size_t size) {
  unsigned output = 0;
  for (int i = size - 1; i >= 0; i--) {
    // Multiplying charcode by power of 16 and add to output
    output += input[i] * pow(16, i * 2);
  }
  return output;
}

int iconvEasy(const char *toCode, const char *fromCode, char *toBuffer,
              size_t toSize, char *dataForConversion) {
  size_t fromSize, k = 0;
  char *fromBuffer = dataForConversion; // Pointer for iconv() finction
  iconv_t conversionDescriptor;

  fromSize = strlen(dataForConversion);
  conversionDescriptor = iconv_open(toCode, fromCode);
  if (conversionDescriptor == (iconv_t)(-1)) {
    printf("iconv_open() error\n");
    exit(1);
  }
  k = iconv(conversionDescriptor, &fromBuffer, &fromSize, &toBuffer, &toSize);
  iconv_close(conversionDescriptor);

  return 0;
}

// Print file entries in folder by its address (+ name, address, and number of
// files in folder)
void printFileEntries(const unsigned char folderName[], const unsigned folderEntryAddress,
                      const unsigned nFileRecords, FILE *file) {
  unsigned char nFileEntriesHex[4], fileEntryNameOffsetHex[4],
      fileEntryFullSizeHex[4], fileEntrySizeHex[4], fileEntryAddressHex[4];
  char fileEntryNameRaw[FILE_ENTRY_NAME_SIZE],
      fileEntryName[FILE_ENTRY_NAME_SIZE];
  unsigned nFileEntries, fileEntryNameOffset, fileEntryFullSize, fileEntrySize,
      fileEntryAddress = 0;
  bool isFileEntryCompressed = false;

  if (nFileRecords != 0) {
    for (int i = 0; i < nFileRecords; i++) {
      // Sanitize array before using with iconv
      memset(fileEntryName, 0, FILE_ENTRY_NAME_SIZE);

      fseek(file, folderEntryAddress + FILE_RECORD_SIZE * i, SEEK_SET);
      fread(fileEntryNameOffsetHex, sizeof fileEntryNameOffsetHex, 1, file);
      fread(fileEntryFullSizeHex, sizeof fileEntryFullSizeHex, 1, file);
      fread(fileEntrySizeHex, sizeof fileEntrySizeHex, 1, file);
      fread(fileEntryAddressHex, sizeof fileEntryAddressHex, 1, file);

      fileEntryNameOffset = convertHexToDec(fileEntryNameOffsetHex,
                                            sizeof fileEntryNameOffsetHex);
      fileEntryFullSize =
          convertHexToDec(fileEntryFullSizeHex, sizeof fileEntryFullSizeHex);
      fileEntrySize =
          convertHexToDec(fileEntrySizeHex, sizeof fileEntrySizeHex);
      isFileEntryCompressed = fileEntrySize > fileEntryFullSize ? true : false;
      fileEntryAddress =
          convertHexToDec(fileEntryAddressHex, sizeof fileEntryAddressHex);

      fseek(file, folderEntryAddress + FILE_RECORD_SIZE * i + fileEntryNameOffset,
            SEEK_SET);
      fread(fileEntryNameRaw, FILE_ENTRY_NAME_SIZE, 1, file);
      iconvEasy("UTF-8", "SHIFT-JIS", fileEntryName, FILE_ENTRY_NAME_SIZE,
                fileEntryNameRaw);
      printf("%s,0x%x,%u,%s,%d,%u,0x%x\n", folderName, folderEntryAddress, nFileRecords,
             fileEntryName, isFileEntryCompressed, fileEntrySize,
             fileEntryAddress);
    }
  } else {
    printf("%s,0x%x,%d,,,,\n", folderName, folderEntryAddress, nFileRecords);
  }
}

int main(int argc, const char *argv[]) {
  if (argc > 1) {
    FILE *file = fopen(argv[1], "rb");
    int fileSize, folderTableSize, folderTableAddress, folderRecordSize,
        folderEntryNumberOfFiles, folderEntryAddress = 0;
    unsigned char folderTableAddressHex[4], folderRecordN1Hex[2],
        folderRecordN2Hex[2], folderRecordSizeHex[2],
        folderEntryNumberOfFilesHex[2], folderEntryAddressHex[4],
        folderEntrySizeHex[4];
    char fileHeader[HEADER_SIZE];

    if (file) {
      // Get file size
      fseek(file, 0, SEEK_END); // Move file cursor to EOF
      fileSize = ftell(file);   // Save current position of file cursor
      rewind(file);             // Move file cursor to beginning of file

      // File signature check
      fread(fileHeader, HEADER_SIZE, 1, file);
      fseek(file, 0, SEEK_SET);
      if (memcmp(fileHeader, BPK0_HEADER, HEADER_SIZE) != 0) {
        printf("Bad file signature\n");
        exit(1);
      }

      fseek(file, 8, SEEK_SET); // Seek to folder table address
      fread(folderTableAddressHex, sizeof folderTableAddressHex, 1, file);
      folderTableAddress =
          convertHexToDec(folderTableAddressHex, sizeof folderTableAddressHex);
      folderTableSize = fileSize - folderTableAddress;
      printf("Folder name,Folder address,Number of files,File name,Is file "
             "compressed?,File size,File address\n");

      while (folderTableSize > 0) {

        fseek(file, fileSize - folderTableSize,
              SEEK_SET); // Seek to next folder record
        fread(folderRecordN1Hex, sizeof folderRecordN1Hex, 1, file);
        fread(folderRecordN2Hex, sizeof folderRecordN2Hex, 1, file);
        fread(folderRecordSizeHex, sizeof folderRecordSizeHex, 1,
              file);
        fread(folderEntryNumberOfFilesHex, sizeof folderEntryNumberOfFilesHex,
              1, file);
        fread(folderEntryAddressHex, sizeof folderEntryAddressHex, 1, file);
        fread(folderEntrySizeHex, sizeof folderEntrySizeHex, 1, file);
        fseek(file, 4, SEEK_CUR); // Skip divider (0x00000000)

        folderRecordSize =
            convertHexToDec(folderRecordSizeHex, sizeof folderRecordSizeHex);
        folderEntryNumberOfFiles = convertHexToDec(
            folderEntryNumberOfFilesHex, sizeof folderEntryNumberOfFilesHex);
        folderEntryAddress = convertHexToDec(folderEntryAddressHex,
                                             sizeof folderEntryAddressHex);

        unsigned char
            folderEntryName[folderRecordSize - FOLDER_ENTRY_NAME_OFFSET];

        fread(folderEntryName, sizeof folderEntryName, 1, file);
        printFileEntries(folderEntryName, folderEntryAddress,
                         folderEntryNumberOfFiles, file);
        folderTableSize -= folderRecordSize;
      }
      fclose(file);
    } else {
      printf("Can't read file\n");
      exit(1);
    }
  } else {
    printf("Usage: %s <filename>\n", argv[0]);
  }
  exit(0);
}
