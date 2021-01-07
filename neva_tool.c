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

#include <stdio.h> // printf(); fopen(); fseek(); ftell(); fread(); fclose()
#include <math.h> // pow()
#include <string.h> // memcpy()
#include <stdbool.h> // bool, true, false

// Constants
const char bpk0_header[4] = "BPK0";
const char bdl0_header[4] = "BDL0";
const unsigned short header_size = 4;
const unsigned short file_entry_name_size = 64;
const unsigned short temp_file_entry_size = 16;
const unsigned short folder_entry_name_offset = 20;

// Convert little-endian hex array to decimal number
int hex_to_dec (const char input[], const int amount, const int starting_position)
{
	int temp = 0;
	const int stop = starting_position - amount;
	for (int i = starting_position; i > stop; i--)
	{
		// Multiplying charcode by power of 16 and add to temp
		temp += (input[i] < 0 ? input[i] + 256 : input[i]) * pow (16, (i - (stop + 1)) * 2);
	}
	return temp;
}

int hex_to_dec_short (const char input[])
{
	int temp = 0;
	for (int i = strlen (input) - 1; i >= 0; i--)
	{
		// Multiplying charcode by power of 16 and add to temp
		temp += (input[i] < 0 ? input[i] + 256 : input[i]) * pow (16, i * 2);
	}
	return temp;
}

// Print file entries in folder by its address (+ name, address, and number of files in folder)
void file_print (char folder_name[], int address, int number_of_files, FILE *file)
{
	if (address != 0 && number_of_files != 0)
	{
		fseek (file, address, SEEK_SET);
		char get_size[4];
		fread (get_size, 4, 1, file);
		fseek (file, -4, SEEK_CUR); // Return file cursor to starting position
		int number_of_entries = hex_to_dec_short (get_size) / temp_file_entry_size;
		for (int i = 0; i < number_of_entries; i++)
		{
			fseek (file, address + temp_file_entry_size * i, SEEK_SET);
			char temp_file_entry[temp_file_entry_size];
			fread (temp_file_entry, temp_file_entry_size, 1, file);
			int file_entry_name_offset = hex_to_dec (temp_file_entry, 4, 3);
			int file_entry_full_size = hex_to_dec (temp_file_entry, 2, 5);
			int file_entry_size = hex_to_dec (temp_file_entry, 2, 9);
			bool file_entry_is_compressed = file_entry_size > file_entry_full_size ? true : false;
			int file_entry_address = hex_to_dec (temp_file_entry, 4, 15);
			char file_entry_name[file_entry_name_size];
			fseek (file, address + temp_file_entry_size * i + file_entry_name_offset, SEEK_SET);
			fread (file_entry_name, file_entry_name_size, 1, file);
			for (int j = 0; j < strlen (file_entry_name); j++)
			{
				// Shift-JIS -> UTF-8 fullwidth latin small letter hack
				if (memcmp (file_entry_name + j, "\x82", 1) == 0)
				{
					// 0x828C -> 0xEFBD8C (Shift-JIS -> UTF-8)
					file_entry_name[j] = 0xEF;
					for (int k = 12; k > j; k--) // Total length should be 13 bytes
					{
						file_entry_name[k] = file_entry_name[k - 1];
					}
					file_entry_name[j + 1] = 0xBD;
				}
			}
			printf ("%s,0x%x,%d,%s,%d,%d,0x%x\r\n", folder_name, address, number_of_files, file_entry_name, file_entry_is_compressed, file_entry_size, file_entry_address);
		}
	}
	else
	{
		printf ("%s,0x%x,%d,,,,\r\n", folder_name, address, number_of_files);
	}
}

// Exit codes:
// 1 - no file
// 2 - can't read file
// 3 - wrong file

int main (int argc, const char *argv[])
{
	if (argc == 2)
	{
		FILE *file;
		int file_length = 0;
		int scrolled_length = 0; // how much bytes was read from folder entries table
		int folder_table_address = 0;

		file = fopen (argv[1], "rb");

		if (file)
		{
			// Get file size
			fseek (file, 0, SEEK_END);  // Move file cursor to EOF
			file_length = ftell (file); // Save current position of file cursor
			fseek (file, 0, SEEK_SET);  // Move file cursor to beginning of file

			// File signature check
			char test_header[header_size];
			fread (test_header, header_size, 1, file);
			fseek (file, 0, SEEK_SET);
			if (memcmp (test_header, bpk0_header, header_size) != 0)
			{
				printf ("%s %s\n", test_header, bpk0_header);
				printf ("Bad file signature\n");
				return 3;
			}

			fseek (file, 8, SEEK_SET);
			char temp[4];
			fread (temp, 4, 1, file);
			folder_table_address = hex_to_dec_short (temp);
			printf ("Folder name,Folder address,Number of files,File name,Is file compressed?,File size,File address\r\n");

			while (folder_table_address + scrolled_length < file_length)
			{
				fseek (file, folder_table_address + scrolled_length, SEEK_SET);
				fseek (file, 4, SEEK_CUR);
				char folder_entry_size[2];
				fread (folder_entry_size, 2, 1, file);
				int folder_entry_size_int = hex_to_dec_short (folder_entry_size);
				fseek (file, -6, SEEK_CUR); // Return file cursor to starting position
				char temp_folder_entry[folder_entry_size_int];
				fread (temp_folder_entry, folder_entry_size_int, 1, file);
				int folder_entry_number_of_files = hex_to_dec (temp_folder_entry, 2, 7);
				int folder_entry_address = hex_to_dec (temp_folder_entry, 4, 11);
				char folder_entry_name[folder_entry_size_int - folder_entry_name_offset];
				memcpy (folder_entry_name, temp_folder_entry + folder_entry_name_offset, folder_entry_size_int - folder_entry_name_offset);

				file_print (folder_entry_name, folder_entry_address, folder_entry_number_of_files, file);

				scrolled_length += folder_entry_size_int;
			}
			fclose (file);
		}
		else
		{
			return 2;
		}
	}
	else
	{
		printf ("Usage: %s <filename>\n", argv[0]);
		return 1;
	}
	return 0;
}
