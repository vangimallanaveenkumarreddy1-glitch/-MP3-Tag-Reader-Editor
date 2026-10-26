#include<stdio.h>
#include"main.h"
#include <string.h>
#include<stdlib.h>
#include<ctype.h>
int main(int argc,char* argv[])
{
    // Commmand line arguments validation

    if(argc<3 || argc > 5)
    {
        printf("Invalid Argument Count..\n");
        printf("The Arguments Must Be : \n");
        printf("\tVIEW : ./a.out <file_name> -v\n");
        printf("\tEDIT : ./a.out <file_name> -e <field> ''data''\n");
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
    int op=0;
    
    char data[6][20]={"title","artist","album","year","comment","genre"};
    CLA c1;
    c1.op6=-1;
    if(strcmp(argv[2],"-v")==0)
    {
        op=1;
    }
    else if(strcmp(argv[2],"-e")==0)
    {
        if(argc !=5)
        {
            printf("Invalid Argument Count..\n");
            printf("Syntax : ./a.out <file_name> -e <data_to_edit> ""NEW_DATA"" \n");
            return 0;
        }
        op=2;
        for(int i=0;i<6;i++)
        {
            if(strcasecmp(argv[3],data[i]) == 0)
            {
                c1.op=i+1;
                break;
            }
        }
        if (c1.op == 0) 
        {
            printf("Unknown field '%s'\n", argv[3]);
            fclose(fp);
            return 0;
        }
        if (c1.op == 6) 
        {
            c1.op6 = atoi(argv[4]);
            if (c1.op6 < 0 || c1.op6 > 147) {
                printf("Error: genre must be 0-147\n");
                fclose(fp);
                return 0;
            }
        } else {
            snprintf(c1.data, sizeof c1.data, "%s", argv[4]);
        }
 
    }
    printf("\n================================================================================\n");
    printf("                              MP3 TAG READER \n");
    printf("================================================================================\n");

    // Checking Song version id3v1 or id3v2 
    int ch,exit=0;
    int res=check_type(fp);
        rewind(fp);

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
            printf("\n********************* Thank You  *************************\n");
            break;
         case 2:
         {
            edit(fp, res, argv[1],c1);
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
            printf("\n********************* Thank You  *************************\n");
        }
            break;
            default: printf("Invalid choise....\n");
        }

    
    fclose(fp);
    return 0;
} 
