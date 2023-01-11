#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>


void map(char*);

int main()
{
    char * filepath = "/dev/mem";
        
    printf("Hello World !\n");
    
    map(filepath);

    return EXIT_SUCCESS;
}

void map(char* filepath)
{
    int fd = open(filepath, O_RDWR);
    if(fd < 0)
    {
        perror("Cannot open file");
        return;
    }

    void* address = mmap (NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0xFF203000);
    
    if(address == MAP_FAILED)
    {
        printf("Shared Mapping Failed\n");
        return;
    }   

    printf("%p\n", address);

    *(int*)address = 4;

    int status = munmap(address, 4);  
    if(status)
    {
        perror("Unmap Failed\n");
        return;
    }

    printf("%p\n", address);

    close(fd);
    
    return;
}