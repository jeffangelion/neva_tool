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

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

void printh (const char input[], const int amount, const int starting_position)
{
	for (int i = starting_position; i < amount + starting_position; i++)
	{
		printf ("%02x", input[i] & 0xff);
		// Print whitespace after any hex but last
		if (i < amount + starting_position - 1)
		{
			printf (" ");
		}
	}
}

// convert little-endian hex array to decimal number
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

int main (int argc, const char *argv[])
{
    if (argc < 2)
	{
		printf ("Usage: %s <filename>\n", argv[0]);
		return 1;
	}

		const unsigned char bpk0_header[4] = {0x42, 0x50, 0x4B, 0x30};
		const unsigned char bdl0_header[4] = {0x42, 0x44, 0x4C, 0x30};
		const unsigned short buffer_size   = 2048;
		const unsigned short header_size   = 4;

    FILE *file;
    int file_length = 0;
		int footer_address = 0;
		int chunk_size = 0;
		char buffer[buffer_size];
		bool first_chunk = true;

		file = fopen (argv[1], "rb");

		if (file)
		{
		// Get file size and return cursor to beginning
		fseek (file, 0, SEEK_END);
        file_length = ftell (file);
        fseek (file, 0, SEEK_SET);

        printf ("{\r\n");

        for (int i = 0; i < file_length; i += buffer_size)
        {
            fseek (file, i, SEEK_SET);
            fread (buffer, buffer_size, 1, file);
            char tmp[header_size];
            memcpy (tmp, buffer, header_size);

            if (memcmp (tmp, bpk0_header, header_size) == 0)
			{
                //{
                //    "header":
                //    {
                //        "address": "",
                //        "magic bytes": ""
                //    },
                //    "chunks":
                //    [
                printf ("\t\"header\":\r\n\t{\r\n\t\t\"address\": \"0x0\",\r\n\t\t\"magic bytes\": \"");
                printh (buffer, 16, 4);
                printf ("\"\r\n\t},\r\n\t\"chunks\":\r\n\t[\r\n");

				footer_address = hex_to_dec (buffer, 4, 19);
			}

            if (memcmp (tmp, bdl0_header, header_size) == 0)
			{
                //    {
                //        "address": "",
                //        "size": int,
                //        "compressed": bool
                //    },
                if (first_chunk)
                {
                    printf ("\t\t{\r\n\t\t\t\"address\": \"0x%x\",\r\n", i);
                    first_chunk = false;
                }
                else
                {
                    printf (",\r\n\t\t{\r\n\t\t\t\"address\": \"0x%x\",\r\n", i);
                }
                chunk_size = hex_to_dec (buffer, 3, 10);
                if (chunk_size == 0)
                {
                    chunk_size = hex_to_dec (buffer, 3, 6);
                    printf ("\t\t\t\"size\": %d,\r\n\t\t\t\"compressed\": false\r\n\t\t}", chunk_size);
                }
                else
                {
                    printf ("\t\t\t\"size\": %d,\r\n\t\t\t\"compressed\": true\r\n\t\t}", chunk_size);
                }
			}

			else if (i == footer_address)
			{
                //    ],
                //    "footer":
                //    {
                //        "address": ""
                //    }
                //}
                printf ("\r\n\t],\r\n\t\"footer\":\r\n\t{\r\n\t\t\"address\": \"0x%x\"\r\n\t}\r\n}\r\n", i);
			}
        }
        fclose (file);
    }
    return 0;
}
