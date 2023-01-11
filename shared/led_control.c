#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>


void map(char*, long);

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        perror("Usage : ./led long pattern");
        exit(EXIT_FAILURE);
    }

    char * filepath = "/proc/ensea/chenille";
        
    printf("Hello World !\n");
    
    map(filepath, atol(argv[1]));

    return EXIT_SUCCESS;
}

void map(char* filepath, long pattern)
{
    long buf;

    int fd = open(filepath, O_RDWR);
    if(fd < 0)
    {
        perror("Cannot open file");
        return;
    }

    ssize_t r = read(fd, &buf, 4);

    printf("Read : %ld, Buf : %ld\n", r, buf);

    ssize_t w = write(fd, &pattern, 4);

    printf("Written : %ld\n", w);

    close(fd);
    
    return;
}