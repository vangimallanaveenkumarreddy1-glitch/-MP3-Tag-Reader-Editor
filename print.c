#include<stdio.h>
#include"main.h"
#include <string.h>
#include<stdlib.h>

// Array of List of Geners
char *GENRES[] = {
    "Blues", "Classic Rock", "Country", "Dance", "Disco",
    "Funk", "Grunge", "Hip-Hop", "Jazz", "Metal",
    "New Age", "Oldies", "Other", "Pop", "R&B",
    "Rap", "Reggae", "Rock", "Techno", "Industrial",
    "Alternative", "Ska", "Death Metal", "Pranks", "Soundtrack",
    "Euro-Techno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk",
    "Fusion", "Trance", "Classical", "Instrumental", "Acid",
    "House", "Game", "Sound Clip", "Gospel", "Noise",
    "AlternRock", "Bass", "Soul", "Punk", "Space",
    "Meditative", "Instrumental Pop", "Instrumental Rock", "Ethnic", "Gothic",
    "Darkwave", "Techno-Industrial", "Electronic", "Pop-Folk", "Eurodance",
    "Dream", "Southern Rock", "Comedy", "Cult", "Gangsta",
    "Top 40", "Christian Rap", "Pop/Funk", "Jungle", "Native American",
    "Cabaret", "New Wave", "Psychadelic", "Rave", "Showtunes",
    "Trailer", "Lo-Fi", "Tribal", "Acid Punk", "Acid Jazz",
    "Polka", "Retro", "Musical", "Rock & Roll", "Hard Rock",
    "Folk", "Folk-Rock", "National Folk", "Swing", "Fast Fusion",
    "Bebob", "Latin", "Revival", "Celtic", "Bluegrass",
    "Avantgarde", "Gothic Rock", "Progressive Rock", "Psychedelic Rock",
    "Symphonic Rock", "Slow Rock", "Big Band", "Chorus", "Easy Listening",
    "Acoustic", "Humour", "Speech", "Chanson", "Opera",
    "Chamber Music", "Sonata", "Symphony", "Booty Bass", "Primus",
    "Porn Groove", "Satire", "Slow Jam", "Club", "Tango",
    "Samba", "Folklore", "Ballad", "Power Ballad", "Rhythmic Soul",
    "Freestyle", "Duet", "Punk Rock", "Drum Solo", "Acapella",
    "Euro-House", "Dance Hall"
};

// Function definition for Checking file type

int check_type(FILE *fp)
{
    char buffer[5];
    rewind(fp);
    if (fread(buffer, 1, 3, fp) == 3) 
    {
        if (buffer[0]=='I' && buffer[1]=='D' && buffer[2]=='3')
            return 2;
    }
    fseek(fp, -128, SEEK_END);
    if (fread(buffer, 1, 3, fp) == 3) 
    {
        if (buffer[0]=='T' && buffer[1]=='A' && buffer[2]=='G')
            return 1;
    }
    return 0;
}

// Function Definition for Printing Idv1 detiles

void print_idv1(FILE *fp)
{
    unsigned char tag[128];
    char title[31], artist[31], album[31], year[5], comment[31];
    unsigned char genre;

// Error check

    fseek(fp, -128, SEEK_END);
    if (fread(tag, 1, 128, fp) != 128) {
        printf("Error: could not read ID3v1 tag\n");
        return;
    }
// Copying data from buffer to related functions using memcpy

    memcpy(title,   &tag[3],  30); title[30]   = '\0';
    memcpy(artist,  &tag[33], 30); artist[30]  = '\0';
    memcpy(album,   &tag[63], 30); album[30]   = '\0';
    memcpy(year,    &tag[93],  4); year[4]     = '\0';
    memcpy(comment, &tag[97], 30); comment[30] = '\0';
    genre = tag[127];

// Trim trailing spaces and nulls 

    remove_space(title,   30);
    remove_space(artist,  30);
    remove_space(album,   30);
    remove_space(year,     4);
    remove_space(comment, 30);

// Printing Detiles

    printf("\n----------------- ID3v1 Tag -------------------\n\n");
    printf("  Title   : %s\n", title[0]   ? title   : "[ empty ]");
    printf("  Artist  : %s\n", artist[0]  ? artist  : "[ empty ]");
    printf("  Album   : %s\n", album[0]   ? album   : "[ empty ]");
    printf("  Year    : %s\n", year[0]    ? year    : "[ empty ]");
    printf("  Comment : %s\n", comment[0] ? comment : "[ empty ]");
    printf("  Genre   : %s\n\n",(genre < GENRE_COUNT) ? GENRES[genre] : " Unknown ");
}

// Function definition for removing trailing spaces

void remove_space(char *str,int len)
{
    int i=len-1;
    while(i>=0 && (str[i]==' '|| str[i]=='\0'))
    {
        str[i]='\0';
        i--;
    }
}

//  Function definition for Printing detiles of id3v2 

void print_idv2(FILE *fp)
{
    unsigned char buffer[10];
    unsigned int tag_size;
    unsigned char vrsn,rev,flag;
    
    // Error Check

    rewind(fp);
    if(fread(buffer,1,10,fp)!=10)
    {
        printf("Error: cannot read ID3v2 header\n");
        return;
    }
    if(!((buffer[0]=='I') && (buffer[1]=='D') && (buffer[2]=='3')))
    {
        printf("Error: not an ID3v2 tag\n");
        return;
    }
    vrsn=buffer[3];
    rev=buffer[4];
    flag=buffer[5];

    // Finding Tag size
    tag_size=syncnum(&buffer[6]);

    printf("\n-------- ID3v2.%u.%u Tag [ size = %u bytes ] ---------\n\n",vrsn, rev, tag_size);

    //  Checking the Flag type

    if (flag & 0x80) 
        printf("  [Unsynchronisation used]\n");
    if (flag & 0x40) 
        printf("  [Extended header present]\n");
    if (flag & 0x20) 
        printf("  [Experimental tag]\n");

    // Finding and Extension and skipping it.
    if(flag & 0x40)
    {
        char extn[4];
        if(fread(extn,1,4,fp)==4)
        {
            int ext_size = big_to_int(extn);
            fseek(fp, ext_size, SEEK_CUR);
        }
    }

    //  Printing the detiles of the song
    long tag_start = ftell(fp);
    long tag_end   = tag_start + (long)tag_size;

    while (ftell(fp) + 10 <= tag_end) 
    {
        unsigned char fh[10];
        if (fread(fh, 1, 10, fp) != 10) 
            break;

        if (fh[0] == 0x00) 
            break;

        char frame_id[5] = {0};
        memcpy(frame_id, fh, 4);

        unsigned int frame_size;
        if (vrsn == 4)
            frame_size = syncnum(&fh[4]);
        else
            frame_size = big_to_int(&fh[4]);

        if (frame_size == 0 || frame_size > tag_size) 
            break;

        char *name = frame_name(frame_id);

        if (name && strcmp(frame_id, "APIC") == 0) 
        {
            id3v2_read_pic(fp, frame_size);
        }
        else if (name && frame_id[0] == 'T') {
           
            unsigned char enc;
            fread(&enc, 1, 1, fp);

            unsigned int text_len = frame_size - 1;
            char *buf = malloc(text_len + 1);
            if(buf==NULL)
            {
                return;
            }
            size_t got = fread(buf, 1, text_len, fp);
                buf[got] = '\0';

            if (enc == 0x00) {
                printf("  %-8s : %s\n", name, buf);
            } else {
                printf("  %-8s : [UTF-16 encoded, %u bytes]\n",name, text_len);
            }
            free(buf);
        }
        else {
            fseek(fp, frame_size, SEEK_CUR);
        }
    }
}

// Funtiondefinition for number reversing to find size

int syncnum(unsigned char s[])
{
    return (((unsigned)(s[0] & 0X7F)<<21)|((unsigned)(s[1]&0X7F)<<14)|((unsigned)(s[2] & 0X7F) << 7)|((unsigned)(s[3]&0X7F)));
}

//  Function definition for converting Big indian number to Intiger

int big_to_int( unsigned char *b)
{
    return ((unsigned int)b[0] << 24) | ((unsigned int)b[1] << 16) | ((unsigned int)b[2] <<  8) |  (unsigned int)b[3];
}

// Function definition for Frame name finding by comparing

char *frame_name(char *id)
{
    if (strcmp(id, "TIT2") == 0) return "Title";
    if (strcmp(id, "TPE1") == 0) return "Artist";
    if (strcmp(id, "TALB") == 0) return "Album";
    if (strcmp(id, "TYER") == 0) return "Year";
    if (strcmp(id, "TCON") == 0) return "Genre";
    if (strcmp(id, "TRCK") == 0) return "Track";
    if (strcmp(id, "COMM") == 0) return "Comment";
    if (strcmp(id, "APIC") == 0) return "Picture";
    if (strcmp(id, "TCOM") == 0) return "Composer";
    if (strcmp(id, "TPUB") == 0) return "Publisher";
    if (strcmp(id, "TENC") == 0) return "Encoded by";
    if (strcmp(id, "TSSE") == 0) return "Encoder settings";
    if (strcmp(id, "TLEN") == 0) return "Length";
    if (strcmp(id, "TCOP") == 0) return "Copyright";
    return NULL;
}

// Finding the Metadata about song image and printing its detiles

void id3v2_read_pic(FILE *fp, unsigned int size)
{
    unsigned char encode;
    char mime[128]={0};
    char pic_type=0;
    int i=0;
    unsigned char c;
    if(size == 0)
    {
        return;
    }
    fread(&encode,1,1,fp);
    size--;
    
    while(size >0 && i< 127 )
    {
        fread(&c,1,1,fp);
        size--;
        if(c == '\0')
            break;
        mime[i]=c;
        i++;
    }
    mime[i]='\0';
    if(size>0)
    {
        fread(&pic_type,1,1,fp);
        size--;
    }
    if(encode == 0x00 || encode == 0x03)
    {
        while(size>0)
        {
            fread(&c,1,1,fp);
            size--;
            if(c=='\0')
            {
                break;
            }
        }
    }
    else{
        while(size>1)
        {
            fread(&c,1,1,fp);
            size--;
            if(c=='\0')
            {
                fread(&c,1,1,fp);
                size--;
                if(c=='\0')
                break;
            }
        }
    }
    if (size > 0)// skip picture data
    fseek(fp, size, SEEK_CUR);   
    
    printf("Picture : MIME=%s, size=%u bytes, type=0x%02X\n", mime[0] ? mime : "[ unknown ]", size, pic_type);
}













