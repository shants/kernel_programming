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
#include <string.h>

#define BLOCK_SIZE 2048

int main(int argc, char **argv)
{
    FILE *fout;
    char buffer[BLOCK_SIZE];
    int bytes_read;
    int cid;
    int devfd = open("/dev/fcontainer", O_RDWR);
    cid = atoi(argv[2]);
    fcontainer_create(devfd, cid);
    fout = fopen(argv[1], "w");
    while(!feof(stdin)) {
        bytes_read = fread(buffer, sizeof(char), BLOCK_SIZE, stdin);
        fwrite(buffer, sizeof(char), bytes_read, fout);
    }
    fclose(fout);
    close(devfd);
    return 0;
}