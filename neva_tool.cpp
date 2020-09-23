/* Copyright © 2020 Ivan Vatlin <jenrus@riseup.net>
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

#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>

void printh (const char input[], const int amount, const int starting_position)
{
	for (int i = starting_position; i < amount + starting_position; i++)
	{
		printf ("%02x", input[i] & 0xff);
		// Print whitespace after any hex but last
		if (i < amount + starting_position -1)
		{
			printf(" ");
		}
	}
}

int main (int argc, const char *argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <filename>\n", argv[0]);
		return 1;
	}

	const unsigned char bpk0_header[4] = {0x42, 0x50, 0x4B, 0x30};
	const unsigned char bdl0_header[4] = {0x42, 0x44, 0x4C, 0x30};
	const unsigned short buffer_size   = 2048;
	const unsigned short header_size   = 4;

	int file_length = 0;
	int footer_address = 0;
	char buffer[buffer_size];

	std::ifstream file (argv[1], std::ifstream::binary);
	if (file)
	{
		// Get file size and return cursor to beginning
		file.seekg (0, file.end);
		file_length = file.tellg();
		file.seekg (0, file.beg);

		for (int i = 0; i < file_length; i += buffer_size)
		{
			file.seekg (i, file.beg);
			file.read (buffer, buffer_size);
			char tmp[header_size];
			memcpy (tmp, buffer, header_size);

			if (memcmp (tmp, bpk0_header, header_size) == 0)
			{
				printf ("header at 0x0, magic bytes: ");
				printh (buffer, 16, 4);
				printf ("\n");

				for (int j = 19; j > 15; j--)
				{
					// Multiplying charcode by power of 16 and add to footer_address
					footer_address += (buffer[j] < 0 ? buffer[j] + 256 : buffer[j]) * pow (16, (j - 16) * 2);
				}
			}

			if (memcmp (tmp, bdl0_header, header_size) == 0)
			{
				printf ("chunk  at 0x%x, magic bytes: ", i);
				printh (buffer, 28, 4);
				printf ("\n");
			}

			else if (i == footer_address)
			{
				printf ("footer at 0x%x\n", i);
				break;
			}
		}

		file.close();
	}
	return 0;
}
