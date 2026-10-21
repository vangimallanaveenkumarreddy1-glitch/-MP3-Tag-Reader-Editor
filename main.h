/* 
   Project      : MP3 Tag Reader & Editor
   Author       : Vangimalla Naveen Kumar Reddy
   Batch ID     : 26015_092
   Language     : C
   File         : main.c
 
   DESCRIPTION
   -----------
        MP3 Tag Reader & Editor is a C-based utility that reads, displays and EDITS the
   metadata embedded inside an MP3 file. This metadata, known as ID3 tags, stores
   details such as Title, Artist, Album, Year, Genre, Track number, Comment, and
   Cover Art.
 
   An MP3 file can contain two kinds of ID3 tags:
       1. ID3v1  - A fixed 128-byte block at the END of the file
       2. ID3v2  - A variable-size frame-based block at the START of the file
 
   ID3v2 has multiple versions:
       2.2  - 3-byte frame IDs, 3-byte sizes  (not supported here)
       2.3  - 4-byte frame IDs, big-endian 4-byte sizes
       2.4  - 4-byte frame IDs, synchsafe 4-byte sizes
 
   ---------------------------------------------------------------------------------------
   FEATURES
   ---------------------------------------------------------------------------------------
 
   VIEW MODE
       - ID3v1 : Title, Artist, Album, Year, Comment, Genre
       - ID3v2 : All text frames (TIT2, TPE1, TALB, TYER/TDRC, TCON, TRCK,
                 COMM, TCOM, TPUB, TENC, TSSE, TLEN, TCOP) + APIC picture info
       - Automatically detects and shows BOTH tags if a file has both.
 
   EDIT MODE
       - Edit one field at a time; the program updates BOTH v1 and v2
         when the file carries both tags.
       - Field limits are enforced against the smallest field size
         (v1: 30 chars for text, 4 chars for Year) so the two tags
         never go out of sync.
       - Genre is entered as a numeric ID (0-147) and stored correctly
         in each format (byte in v1, text TCON in v2).
       - v2 edits rebuild the tag, adjust the 10-byte header's synchsafe
         size field, preserve the audio bytes, then atomically rename the
         temp file over the original.
       - Encoding is preserved/handled: ISO-8859-1, UTF-8 (v2.4) and
         UTF-16 with BOM (v2.3, non-ASCII).
 
   ---------------------------------------------------------------------------------------
   USAGE
   ---------------------------------------------------------------------------------------
 
       $ ./a.out <song_name.mp3>
 
   SAMPLE INPUT :
       $ ./a.out sample-12s.mp3
 
   SAMPLE OUTPUT (View mode) :
       ================================================================================
                                     MP3 TAG READER
       ================================================================================
       Id3v2 found !!..
 
       -------- ID3v2.3.0 Tag [ size = 1413 bytes ] ---------
 
         Title    : Bohemian Rhapsody
         Artist   : Queen
         Album    : A Night at the Opera
         Year     : 1975
         Genre    : Classic
         Track    : 11
 
       ----------------- ID3v1 Tag -------------------
 
         Title   : Bohemian Rhapsody
         Artist  : Queen
         Album   : A Night at the Opera
         Year     : 1975
         Comment : Test comment for project
         Genre   :  Unknown
 
       ********************* Thank You  *************************
 
   SAMPLE EDIT SESSION :
 
       MENU :-
       ---------
       1. VIEW SONG DETILES
       2. EDIT SONG DETILES
       3. EXIT
 
       Plese Select Your Choise : 2
 
       1. Title
       2. Artist
       3. Album
       4. Year
       5. Comment
       6. Genre
       7. Exit
 
        Enter Your Choice: 4
       New value: 2026
       Updated [v1] [v2].
 
   ---------------------------------------------------------------------------------------
   ARGUMENTS
   ---------------------------------------------------------------------------------------
       <song_name.mp3>      Path to the MP3 file to inspect/edit (must end with ".mp3")
 
   ---------------------------------------------------------------------------------------
   FILES
   ---------------------------------------------------------------------------------------
       main.c   - Entry point, CLI parsing, menu loop, view/edit dispatch
       print.c  - Tag reading / printing (check_type, print_idv1, print_idv2,
                  remove_space, frame_name, id3v2_read_pic, syncnum, big_to_int)
       edit.c   - Tag writing (edit, edit_v1, edit_v2, v2_* helpers)
       main.h   - Shared prototypes and GENRE_COUNT
 
*/
#ifndef MAIN_H
#define MAIN_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define GENRE_COUNT 148

int check_type(FILE *);
void print_idv1(FILE *);
void remove_space(char * ,int  );
void print_idv2(FILE * );
void id3v2_read_pic(FILE *, unsigned int);
int syncnum(unsigned char* );
int big_to_int( unsigned char *);
char *frame_name(char *);
void id3v2_read_pic(FILE *, unsigned int );
void edit(FILE * , int  ,char *);
int edit_v1(FILE *fp, int op, char *new_value);
int edit_v2(char *fname, int op, char *new_value);

#endif