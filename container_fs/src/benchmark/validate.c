//////////////////////////////////////////////////////////////////////
//                      North Carolina State University
//
//
//
//                             Copyright 2018
//
////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or modify it
// under the terms and conditions of the GNU General Public License,
// version 2, as published by the Free Software Foundation.
//
// This program is distributed in the hope it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////
//
//   Author:  Hung-Wei Tseng, Yu-Chia Liu
//
//   Description:
//     Running Applications on File Container
//
////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <unistd.h>
#include <fcontainer.h>
#include <fcntl.h>
#include <stdlib.h>

#define BLOCK_SIZE 2048

int main(int argc, char **argv)
{
    FILE *fin;
    char buffer[BLOCK_SIZE], buffer2[BLOCK_SIZE];
    int bytes_read;
    int devfd = open("/dev/fcontainer", O_RDWR);
    int i;
    fcontainer_create(devfd, atoi(argv[2]));
    fin = fopen(argv[1], "r");
    while(!feof(stdin)) {
        bytes_read = fread(buffer, sizeof(char), BLOCK_SIZE, stdin);
        fread(buffer2, sizeof(char), bytes_read, fin);
        for(i = 0 ; i < bytes_read; i++) {
            if(buffer[i] != buffer2[i]) {
                fprintf(stderr, "Failed at %d\n",i);
                fclose(fin);
                return 0;
            }
        }
    }
    fprintf(stderr, "Pass\n");
    fclose(fin);
    return 0;
}