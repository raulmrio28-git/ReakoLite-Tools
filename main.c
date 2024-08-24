/*
** ===========================================================================
** File: ExtHS.cpp
** Description: Program main code
** Copyright (c) 2024 raulmrio28-git.
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 07/26/2024	raulmrio28-git	Initial version
** ===========================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "common.h"
#include "convert.h"
#include "decode.h"
int main(int argc, char* argv[])
{
	if (argc >= 2)
	{
		char fn[256];
		if (strcmp(argv[1], "-d") == 0)
		{
			FILE* fp = fopen(argv[2], "rb");
			uint8_t* buff;
			uint16_t* buffd;
			int len;
			int width, height, frms;
			int cfrm;
			if (!fp)
			{
				printf("Failed to open file %s\n", argv[2]);
				return 1;
			}
			fseek(fp, 0, SEEK_END);
			len = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			buff = (uint8_t*)malloc(len);
			if (!buff)
			{
				printf("Failed to allocate memory\n");
				return 1;
			}
			fread(buff, 1, len, fp);
			fclose(fp);
			rls_common_getinfo(buff, &frms, &width, &height, NULL);
			if (frms == 0 || width == 0 || height == 0)
			{
				printf("Failed to get info from file %s\n", argv[2]);
				return 1;
			}
			printf("Width: %d, Height: %d, Frames: %d\n", width, height, frms);
			
			buffd = (uint16_t*)malloc(width * height * sizeof(uint16_t));
			if (!buffd)
			{
				printf("Failed to allocate memory\n");
				return 1;
			}
			for (cfrm = 0; cfrm < frms; cfrm++)
			{
				if (rls_decode(buff, cfrm, buffd) == false)
				{
					printf("Failed to decode frame %d\n", cfrm);
					return 1;
				}
				sprintf(fn, "%s_%d.png", argv[2], cfrm);
				if (rls_convert_5652png(fn, buffd, width, height) == false)
				{
					printf("Failed to convert frame %d\n", cfrm);
					return 1;
				}
			}
			free(buffd);
		}
		else
		{
			printf("Usage: %s -d/e <input>\n", argv[0]);
			return 1;
		}
		return 0;
	}
}