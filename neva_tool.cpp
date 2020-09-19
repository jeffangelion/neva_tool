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

/* neva.pkg checksums
 * MD5:    26263287db03caeb0d3fc1393534706f
 * SHA1:   59d02466f26f9c8711e74187860aac5ebab7bc6d
 * SHA256: 1ebb2c257abc09f8f61eabcc3f589f62609f21eccedd1f4ba0c3252f149e29d8
 * SHA512: 56365c11ec94b5ac0a469f79474cbbf4008d7db0d95921b9069933ddd19b024147019ca3023efb0ffbb48d2b231b0a172e7163bb95557a89a56d1ecf6e199d51
 */

/* Header: 0x0 -- 0x7FF
 * Body: 0x800 -- 0x121EF7FF
 * Footer: 0x121EF800 -- 0x122451BB
 */

int main(int argc, const char *argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <filename>\n", argv[0]);
		return 1;
	}
	
	const unsigned char bpk0_header[4] = {0x42, 0x50, 0x4B, 0x30};
	const unsigned char bdl0_header[4] = {0x42, 0x44, 0x4C, 0x30};
	const unsigned char bpk0_footer[16] = 
	{0x90, 0x02, 0x00, 0x00,
	 0x00, 0x08, 0x00, 0x00,
	 0x80, 0x01, 0x00, 0x00, 
	 0x00, 0x78, 0xCF, 0x11};
	//const unsigned char zlib_header[2] = {0x78, 0x9C};
	const unsigned short buffer_size = 2048;
	const unsigned short header_size = 4;
	const unsigned short footer_size = 16;
	
	int file_length = 0;
	char *buffer = new char [buffer_size];
	
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
			char tmp[footer_size];
			memcpy (tmp, buffer, footer_size);
			if (memcmp (tmp, bpk0_header, header_size) == 0)
			{
				printf("header at 0x0, magic bytes: ");
				for (short i = 4; i <= 20; i++)
				{
					printf("%x ", (unsigned char) tmp[i]);
				}
				printf("\n");
			}
			if (memcmp (tmp, bdl0_header, header_size) == 0)
			{
				printf("chunk  at 0x%x, magic bytes: ", i);
				// Loop for fancy hex printing
				for (short i = 4; i <= 32; i++)
				{
					if ((unsigned short) tmp[i] <= 15)
					{
						printf("0%x ", (unsigned char) tmp[i]);
					}
					else
					{
						printf("%x ", (unsigned char) tmp[i]);
					}
				}
				printf("\n");
			}
			else if (memcmp (tmp, bpk0_footer, footer_size) == 0)
			{
				printf("footer at 0x%x\n", i);
				break;
			}
		}
		
		file.close();
	}
	return 0;
}
