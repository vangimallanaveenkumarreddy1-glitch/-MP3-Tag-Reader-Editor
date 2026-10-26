
#ifndef MAIN_H
#define MAIN_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define GENRE_COUNT 148

typedef struct Cla
{
  int op;
  char data[31];
  int op6;
}CLA;

// FUNCTION Prototypes

int check_type(FILE *);
void print_idv1(FILE *);
void remove_space(char * ,int  );
void print_idv2(FILE * );
void id3v2_read_pic(FILE *, unsigned int);
int syncnum(unsigned char* );
int big_to_int( unsigned char *);
char *frame_name(char *);
void id3v2_read_pic(FILE *, unsigned int );
void edit(FILE * , int  ,char *,CLA);
int edit_v1(FILE *fp, int op, char *new_value);
int edit_v2(char *fname, int op, char *new_value);

#endif
