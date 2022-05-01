#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#define MAXBUF 1024
#define PATHSIZ 256
#define BUFSIZE 1024
#define BUFFER_SIZE 1024
#define PATHSIZE 512
int wordappear(const char filnename[], const char *word);

/*
컴파일 방법 : gcc mpws.c -o mpws -lrt
파일 갯수만큼 차일드 프로세스 생성하기 , 생성한 차일드프로세스가 각각 파일 읽기


쉐어드 메모리 생성 (shm_open, ftruncate(shm_fd,size) mmap사용-> ptr에 저장


각각의 차일드가 프로세스에게 워드 카운트 값 넘겨주는건 쉐어메모리에 반환
쉐어메모리는 공백 여덟칸을 주어서 구분

child 프로세스가 각각 파일 하나를 맡아서 파일 읽기(fopen 사용 가능))--> 파일 읽으면서 검색하는 단어 갯수 체크

check한 단어 포인터

포인터에게 ptr

파일 열고 검색하는 과정

parent process 는 shared memory 로 () 집계까지



파일 갯수 만큼 child process 생성하기
반복문 for(파일 갯수 ){
    차일드 프로세스 생성
    파일 읽고 문자 갯수 세기(워드카운트)
    문자 갯수 포인터로 저장
}
*/

int main(int argc, char *argv[])
{
    int someone;
    int pid;
    int Permission;
    struct stat bufOfStat;
    char buf[BUFSIZE];
    char source[PATHSIZE];
    char target[PATHSIZE];
    const int SIZE = 4096;
    const char *name = "Os";
    int shm_fd;
    char *ptr;
    FILE *fptr;
    char path[100];
    char word[50];
    int wcount;

    if (argc < 3)
    {
        fprintf(stderr, "시용법: Directory [file1]... [text]\n");
        exit(1);
    }

    printf("Command Line Arguments!\n");
    printf("argc = %d\n", argc);
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
     ftruncate(shm_fd, SIZE);
    ptr = (char *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    sprintf(ptr, "%d", argc - 2);

    for (int i = 1; i < argc - 1; i++)
    {   //pid 하나당 파일 하나를 count 하기 위해서  파일 갯수 만큼 반복 및 fork
        pid = fork();
        if (pid < 0)
        { // child process가 제대로 생성안된경우 
            printf("Error");
            exit(1);
        }
        else if (pid == 0)
        {    // child process가 제대로 생성된 경우 
            shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
            ftruncate(shm_fd, SIZE);
            ptr = (char *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
            //정수형으로 반환 받기 
            wcount = wordappear(argv[i], argv[argc - 1]);
            printf("Child Process: searching %s ...\n", argv[i]);

            // shared memeory 공백을 8칸씩 주면서 안 덮어지게끔 하기[      4\0        4\0    2\0]
            sprintf(ptr + i * 8, "%d", wcount);
            exit(0);
        }
        else
        {
            wait(NULL);
        }
    }

    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    ptr = (char *)mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    // ptr 에 저장된 파일 갯수를 정수형 변환을 해서 파일을 몇개 받았는지 count 가능 이 값을 반복문에 사용
    int count = atoi(ptr);
    int sumcount = 0;

    for (int i = 0; i < count; i++)
    {   // 총 search된 단어를 세어 줌 
        sumcount = sumcount + atoi(ptr + (i + 1) * 8);
        printf("%s :%s\n", argv[i + 1], ptr + (i + 1) * 8);

        // ptr + (i + 1) * 8 쉐어 메모리에서 통계값을 꺼내주는 작업 
        printf("Result: %s '%s'found in %s  :\n", ptr + (i + 1) * 8, argv[argc - 1], argv[i+1]);
    }
    printf("Total count : %d\n", sumcount);

    return 0;
}

int wordappear(const char filnename[], const char *word)
{

    FILE *fptr;
    FILE *fp;
    char str[BUFFER_SIZE];
    char *pos;
    int linecount = 0, count = 0;
    int index;
        // 파일네임과 검색할 단어를 리턴받아 검색한다.
    fp = fopen(filnename, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "시용법: Directory [file1]... [text]\n");

        return 0;
    }
    //파일이 끝날때까지 반복문을 돌려주고 
    while (fgets(str, 512, fp) != NULL)
    {
        linecount++;
        // 단어를 하나씩 검색하면서 넘어간다 
        char *searcher = strtok(str, " ");
        while (searcher != NULL)
        {
        // 대소문자 구분은 따로 하지 않아서  strcase를 사용한다. 
            if (strcasestr(searcher, word))
            {
                count++;
                // printf("%d line\n %s\n", line, str);
            }
            searcher = strtok(NULL, " ");
        }
    }

    fclose(fp);
    return count;
}

