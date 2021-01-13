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
#include <stdlib.h> // exit()
#include <math.h> // pow()
#include <string.h> // memcpy()
#include <stdbool.h> // bool, true, false
#include <iconv.h> // Shift-JIS <-> UTF-8

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

int iconv_easy (char* to_code, char* from_code, char* to_buffer, size_t to_size, char *data_for_conversion)
{
	size_t from_size, k;
	char* from_buffer = data_for_conversion; // Pointer for iconv() finction
	iconv_t conversion_descriptor;

	from_size = strlen (data_for_conversion);
	conversion_descriptor = iconv_open (to_code, from_code);
	if (conversion_descriptor == (iconv_t)(-1))
	{
		printf ("iconv_open() error\n");
		exit (1);
	}
	k = iconv (conversion_descriptor, &from_buffer, &from_size, &to_buffer, &to_size);
	iconv_close (conversion_descriptor);

	return 0;
}

// Print file entries in folder by its address (+ name, address, and number of files in folder)
void file_print (char folder_name[], int address, int number_of_files, FILE *file)
{
	char temp_file_entry[temp_file_entry_size], file_entry_name_temp[file_entry_name_size], file_entry_name[file_entry_name_size];
	int number_of_entries, file_entry_name_offset, file_entry_full_size, file_entry_size, file_entry_address = 0;
	bool file_entry_is_compressed;

	if (number_of_files != 0)
	{
		fseek (file, address, SEEK_SET);
		char get_size[4];
		fread (get_size, 4, 1, file);
		fseek (file, -4, SEEK_CUR); // Return file cursor to starting position
		number_of_entries = hex_to_dec_short (get_size) / temp_file_entry_size;
		for (int i = 0; i < number_of_entries; i++)
		{
			// Sanitize array before using with iconv
			memset (&file_entry_name, 0, file_entry_name_size);

			fseek (file, address + temp_file_entry_size * i, SEEK_SET);
			fread (temp_file_entry, temp_file_entry_size, 1, file);

			file_entry_name_offset = hex_to_dec (temp_file_entry, 4, 3);
			file_entry_full_size = hex_to_dec (temp_file_entry, 2, 5);
			file_entry_size = hex_to_dec (temp_file_entry, 2, 9);
			file_entry_is_compressed = file_entry_size > file_entry_full_size ? true : false;
			file_entry_address = hex_to_dec (temp_file_entry, 4, 15);

			fseek (file, address + temp_file_entry_size * i + file_entry_name_offset, SEEK_SET);
			fread (file_entry_name_temp, file_entry_name_size, 1, file);
			iconv_easy ("UTF-8", "SHIFT-JIS", file_entry_name, file_entry_name_size, file_entry_name_temp);
			printf ("%s,0x%x,%d,%s,%d,%d,0x%x\r\n", folder_name, address, number_of_files, file_entry_name, file_entry_is_compressed, file_entry_size, file_entry_address);
		}
	}
	else
	{
		printf ("%s,0x%x,%d,,,,\r\n", folder_name, address, number_of_files);
	}
}

int main (int argc, const char *argv[])
{
	if (argc == 2)
	{
		FILE *file;
		int file_length = 0;
		int scrolled_length = 0; // Counter of bytes read from folder entries table
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
				printf ("Bad file signature\n");
				exit (1);
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
			printf ("Can't read file\n");
			exit (1);
		}
	}
	else
	{
		printf ("Usage: %s <filename>\n", argv[0]);
	}
	exit (0);
}
