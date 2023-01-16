#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>


void led_modify_pattern(char* filepath, long pattern);

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        perror("Usage : ./led pattern");
        exit(EXIT_FAILURE);
    }

    char * filepath_pattern = "/proc/ensea/chenille";

    led_modify_pattern(filepath_pattern, atol(argv[1]));
    
    return EXIT_SUCCESS;
}

void led_modify_pattern(char* filepath, long pattern)
{
    unsigned long buf = 0;

    int fd = open(filepath, O_RDWR);
    if(fd < 0)
    {
        perror("Cannot open file");
        return;
    }

    ssize_t r = read(fd, &buf, sizeof(buf));

    printf("Read : %ld, Buf : %ld\n", r, buf);

    ssize_t w = write(fd, &pattern, 4);

    printf("Written : %ld\n", w);

    close(fd);
    
    return;
}