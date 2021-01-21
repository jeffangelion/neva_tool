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

#include <iconv.h>   // Shift-JIS <-> UTF-8
#include <math.h>    // pow()
#include <stdbool.h> // bool, true, false
#include <stdio.h>   // printf(); fopen(); fseek(); ftell(); fread(); fclose()
#include <stdlib.h>  // exit()
#include <string.h>  // memcpy(); strlen()

// Constants
const char BPK0_HEADER[] = "BPK0";
const char BDL0_HEADER[] = "BDL0";
const unsigned short HEADER_SIZE = 4;
const unsigned short FILE_ENTRY_NAME_SIZE = 64;
const unsigned short FILE_RECORD_SIZE = 16;
const unsigned short FOLDER_ENTRY_NAME_OFFSET = 20;

// Convert little-endian hex array to decimal number
int convertHexToDec(const unsigned char input[], const unsigned amount,
                    const unsigned startingPosition) {
  unsigned output = 0;
  const int stop = startingPosition - amount;
  for (int i = startingPosition; i > stop; i--) {
    // Multiplying charcode by power of 16 and add to output
    output += (input[i] < 0 ? input[i] + 256 : input[i]) *
              pow(16, (i - (stop + 1)) * 2);
  }
  return output;
}

int convertHexToDecShort(const char input[]) {
  unsigned output = 0;
  for (int i = strlen(input) - 1; i >= 0; i--) {
    // Multiplying charcode by power of 16 and add to output
    output += (input[i] < 0 ? input[i] + 256 : input[i]) * pow(16, i * 2);
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
void printFileEntries(const unsigned char folderName[], const unsigned address,
                      const unsigned nFiles, FILE *file) {
  unsigned char fileRecord[FILE_RECORD_SIZE];
  char nFileEntriesHex[4], fileEntryNameRaw[FILE_ENTRY_NAME_SIZE],
      fileEntryName[FILE_ENTRY_NAME_SIZE];
  unsigned nFileEntries, fileEntryNameOffset, fileEntryFullSize, fileEntrySize,
      fileEntryAddress = 0;
  bool isFileEntryCompressed = false;

  if (nFiles != 0) {
    fseek(file, address, SEEK_SET);
    fread(nFileEntriesHex, 4, 1, file);
    fseek(file, -4, SEEK_CUR); // Return file cursor to starting position
    nFileEntries = convertHexToDecShort(nFileEntriesHex) / FILE_RECORD_SIZE;
    for (int i = 0; i < nFileEntries; i++) {
      memset(&fileEntryName, 0,
             FILE_ENTRY_NAME_SIZE); // Sanitize array before using with iconv

      fseek(file, address + FILE_RECORD_SIZE * i, SEEK_SET);
      fread(fileRecord, FILE_RECORD_SIZE, 1, file);

      fileEntryNameOffset = convertHexToDec(fileRecord, 4, 3);
      fileEntryFullSize = convertHexToDec(fileRecord, 2, 5);
      fileEntrySize = convertHexToDec(fileRecord, 2, 9);
      isFileEntryCompressed = fileEntrySize > fileEntryFullSize ? true : false;
      fileEntryAddress = convertHexToDec(fileRecord, 4, 15);

      fseek(file, address + FILE_RECORD_SIZE * i + fileEntryNameOffset,
            SEEK_SET);
      fread(fileEntryNameRaw, FILE_ENTRY_NAME_SIZE, 1, file);
      iconvEasy("UTF-8", "SHIFT-JIS", fileEntryName, FILE_ENTRY_NAME_SIZE,
                fileEntryNameRaw);
      printf("%s,0x%x,%d,%s,%d,%d,0x%x\r\n", folderName, address, nFiles,
             fileEntryName, isFileEntryCompressed, fileEntrySize,
             fileEntryAddress);
    }
  } else {
    printf("%s,0x%x,%d,,,,\r\n", folderName, address, nFiles);
  }
}

int main(int argc, const char *argv[]) {
  if (argc == 2) {
    FILE *file;
    int fileSize, folderTableSize, folderTableAddress, folderRecordSize,
        folderEntryNumberOfFiles, folderEntryAddress = 0;
    char folderRecordSizeHex[2], folderTableAddressHex[4],
        fileHeader[HEADER_SIZE];

    file = fopen(argv[1], "rb");

    if (file) {
      // Get file size
      fseek(file, 0, SEEK_END); // Move file cursor to EOF
      fileSize = ftell(file);   // Save current position of file cursor
      fseek(file, 0, SEEK_SET); // Move file cursor to beginning of file

      // File signature check
      fread(fileHeader, HEADER_SIZE, 1, file);
      fseek(file, 0, SEEK_SET);
      if (memcmp(fileHeader, BPK0_HEADER, HEADER_SIZE) != 0) {
        printf("Bad file signature\n");
        exit(1);
      }

      fseek(file, 8, SEEK_SET); // Seek to folder table address
      fread(folderTableAddressHex, 4, 1, file);
      folderTableAddress = convertHexToDecShort(folderTableAddressHex);
      folderTableSize = fileSize - folderTableAddress;
      printf("Folder name,Folder address,Number of files,File name,Is file "
             "compressed?,File size,File address\n");

      while (folderTableSize > 0) {
        fseek(file, fileSize - folderTableSize,
              SEEK_SET); // Seek to next folder record

        fseek(file, 4, SEEK_CUR); // Seek to folder record size
        fread(folderRecordSizeHex, 2, 1,
              file); // Read folder record size from file
        folderRecordSize = convertHexToDecShort(folderRecordSizeHex);
        fseek(file, -6, SEEK_CUR); // Return file cursor to starting position

        unsigned char folderRecord[folderRecordSize],
            folderEntryName[folderRecordSize - FOLDER_ENTRY_NAME_OFFSET];

        fread(folderRecord, folderRecordSize, 1, file);

        folderEntryNumberOfFiles = convertHexToDec(folderRecord, 2, 7);
        folderEntryAddress = convertHexToDec(folderRecord, 4, 11);
        memcpy(folderEntryName, folderRecord + FOLDER_ENTRY_NAME_OFFSET,
               folderRecordSize - FOLDER_ENTRY_NAME_OFFSET);

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
