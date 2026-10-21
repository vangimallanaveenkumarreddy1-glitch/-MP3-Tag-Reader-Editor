#include<stdio.h>
#include"main.h"
#include <string.h>
#include<stdlib.h>

int main(int argc,char* argv[])
{
    // Commmand line arguments validation

    if(argc<2)
    {
        printf("Invalid Argument Count..\n");
        return 0;
    }
    if(strstr(argv[1],".mp3")==NULL)
    {
        printf("Invalid Extension Type..\n");
        return 0;
    }
    FILE *fp=fopen(argv[1],"r+b");
    if(fp==NULL)
    {
        printf("Error to open file %s\n",argv[1]);
        return 0;
    }
    printf("\n================================================================================\n");
    printf("                              MP3 TAG READER \n");
    printf("================================================================================\n");

    // Checking Song version id3v1 or id3v2 
    int ch,exit=0,op=0;
    int res=check_type(fp);
    do{
        printf("\nMENU :-\n---------\n1. VIEW SONG DETILES\n2. EDIT SONG DETILES\n3. EXIT\n");
        printf("\nPlese Select Your Choise : ");
        scanf(" %d",&op);
        getchar();
        rewind(fp);
        if(op)
        switch(op)
        {
            case 1: 
                {
                 
                switch(res)
                {
                    case 1: printf("\nId3v1 found !!..\n");
                            print_idv1(fp);
                        break;
                    case 2: printf("\nId3v2 found !!..\n");
                            print_idv2(fp);
                            {
                                fseek(fp, -128, SEEK_END);
                                unsigned char tag[3];
                                if (fread(tag, 1, 3, fp) == 3 &&
                                tag[0] == 'T' && tag[1] == 'A' && tag[2] == 'G') 
                                {
                                    print_idv1(fp);
                                }
                            }
                        break;
                    default:
                        printf("No ID3 tag found in this file.\n");
                }
            }
            break;
         case 2:
            edit(fp, res, argv[1]);
            if (res == 2) 
            {
                fclose(fp);
                fp = fopen(argv[1], "r+b");
                if (fp == NULL) 
                { 
                    printf("Error reopening %s\n", argv[1]); 
                    return 0; 
                }
                res = check_type(fp);
            }
            else 
            {
                rewind(fp);          
            }
            break;
        case 3: printf("\nConform to Exit (Y/N) : ");
                char ch;
                scanf(" %c",&ch);
                if(ch=='Y'||ch=='y')
                {
                    exit=-1;
                    printf("\nProgram Terminated sucessfullyy..\n");
                    printf("\n********************* Thank You  *************************\n");
                    break;
                }
                else
                printf("Exit cancilled !!...\n");
                break;
        default: printf("Invalid choise....\n");
        }

    }while(exit != -1);
    fclose(fp);
    return 0;
}
