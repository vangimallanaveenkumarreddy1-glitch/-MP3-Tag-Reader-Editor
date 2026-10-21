#include<stdio.h>
#include"main.h"
#include <string.h>
#include<stdlib.h>

// Function definition for number conversion from big indian to int
int v2_rd_be32(unsigned char *b)
{
    return ((int)b[0] << 24) | ((int)b[1] << 16) | ((int)b[2] <<  8) |  (int)b[3];
}

// Function definition for read synchsafe (Each byte only uses 7 bits (or) 28 bit values)
int v2_rd_syncsafe(unsigned char *b)
{
    return ((int)(b[0] & 0x7F) << 21) | ((int)(b[1] & 0x7F) << 14) | ((int)(b[2] & 0x7F) <<  7) |  (int)(b[3] & 0x7F);
}

// Function definition to convert int to Big indian
void v2_wr_be32(unsigned char *b, int v)
{
    b[0] = (v >> 24) & 0xFF;
    b[1] = (v >> 16) & 0xFF;
    b[2] = (v >>  8) & 0xFF;
    b[3] =  v        & 0xFF;
}

// Function definition for int to 4 Synchsafe bytes
void v2_wr_syncsafe(unsigned char *b, int v)
{
    b[0] = (v >> 21) & 0x7F;
    b[1] = (v >> 14) & 0x7F;
    b[2] = (v >>  7) & 0x7F;
    b[3] =  v        & 0x7F;
}

// Function definition for Loding the Whole file into memory

unsigned char *v2_load(char *path, long *size)
{
    FILE *f = fopen(path, "rb");
    if (f == NULL) 
        return NULL;

    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    rewind(f);

    unsigned char *buf = malloc((size_t)n);
    if (!buf) { fclose(f); return NULL; }

    if (fread(buf, 1, (size_t)n, f) != (size_t)n) 
    {
        free(buf); fclose(f); 
        return NULL;
    }
    fclose(f);
    *size = n;
    return buf;
}

// Function definition to Write rebuilt file into .mp3 file

int v2_save(char *path, unsigned char *buf, long n)
{
    char tmp[600];
    snprintf(tmp, sizeof tmp, "%s.tmp", path);

    FILE *f = fopen(tmp, "wb");
    if (f == NULL) 
        return -1;

    if (fwrite(buf, 1, (size_t)n, f) != (size_t)n) {
        fclose(f); remove(tmp); return -1;
    }
    fclose(f);

    if (remove(path) != 0 || rename(tmp, path) != 0) {
        remove(tmp); return -1;
    }
    return 0;
}

// Function definition for building the body of a text frame

unsigned char *v2_make_text_body(char *txt, int ver, int *len)
{
    size_t n = strlen(txt);
    int ascii = 1;
    for (size_t i = 0; i < n; ++i)
        if ((unsigned char)txt[i] > 0x7F) { ascii = 0; break; }

    if (ver == 4 || ascii) {
        unsigned char *b = malloc(1 + n);
        if (!b) return NULL;
        b[0] = (ver == 4) ? 0x03 : 0x00;
        memcpy(b + 1, txt, n);
        *len = (int)(1 + n);
        return b;
    }
    unsigned char *b = malloc(3 + n * 2);
    if (!b) return NULL;
    size_t k = 0;
    b[k++] = 0x01;
    b[k++] = 0xFF;
    b[k++] = 0xFE;
    for (size_t i = 0; i < n; ++i) {
        b[k++] = (unsigned char)txt[i];
        b[k++] = 0x00;
    }
    *len = (int)k;
    return b;
}

// Function definition for Assemble a complete frame

unsigned char *v2_build_frame(char *id, int ver, unsigned char f0, unsigned char f1,char *txt, int *out_size)
{
    int body_len = 0;
    unsigned char *body = v2_make_text_body(txt, ver, &body_len);
    if (!body) return NULL;

    int total = 10 + body_len;
    unsigned char *f = malloc((size_t)total);
    if (!f) { free(body); return NULL; }

    memcpy(f, id, 4);
    if (ver == 4) v2_wr_syncsafe(f + 4, body_len);
    else          v2_wr_be32    (f + 4, body_len);
    f[8] = f0;
    f[9] = f1;
    memcpy(f + 10, body, (size_t)body_len);
    free(body);

    *out_size = total;
    return f;
}

// Function definition for rewrite the whole file in version 2 (or) .v2 

int edit_v2(char *fname, int op, char *new_value)
{
    long fsize = 0;
    unsigned char *data = v2_load(fname, &fsize);
    if (data== NULL) 
        return -1;

    if (fsize < 10 || memcmp(data, "ID3", 3) != 0) 
    {
        free(data);
        return -1;
    }

    int ver = data[3];
    if (ver != 3 && ver != 4) 
    {
        free(data);
        return -1;
    }

    long tag_end = 10 + (long)v2_rd_syncsafe(data + 6);

    char *id;
    switch (op) {
        case 1: id = "TIT2"; break;
        case 2: id = "TPE1"; break;
        case 3: id = "TALB"; break;
        case 4: id = (ver == 4) ? "TDRC" : "TYER"; break;
        case 5: id = "COMM"; break;
        case 6: id = "TCON"; break;
        default: free(data); return -1;
    }

    // find existing frame 
    long frame_off = -1;
    int  frame_size = 0;
    unsigned char fflags[2] = { 0, 0 };

    long p = 10;
    while (p + 10 <= tag_end) 
    {
        unsigned char *fh = data + p;
        if (fh[0] == 0) 
            break;

        int sz = (ver == 4) ? v2_rd_syncsafe(fh + 4) : v2_rd_be32(fh + 4);
        if (sz == 0 || p + 10 + (long)sz > tag_end) break;

        if (memcmp(fh, id, 4) == 0) {
            frame_off  = p;
            frame_size = sz;
            fflags[0]  = fh[8];
            fflags[1]  = fh[9];
            break;
        }
        p += 10 + (long)sz;
    }

    int nf_size = 0;
    unsigned char *nf = v2_build_frame(id, ver, fflags[0], fflags[1], new_value, &nf_size);
    if (!nf) { free(data); return -1; }

    // boundaries 
    long pre_start, pre_end, post_start, post_end;
    if (frame_off >= 0) {
        pre_start  = 10;
        pre_end    = frame_off;
        post_start = frame_off + 10 + (long)frame_size;
        post_end   = tag_end;
    } else {
        long last = 10, q = 10;
        while (q + 10 <= tag_end) {
            unsigned char *fh = data + q;
            if (fh[0] == 0) break;
            int sz = (ver == 4) ? v2_rd_syncsafe(fh + 4) : v2_rd_be32(fh + 4);
            if (sz == 0 || q + 10 + (long)sz > tag_end) break;
            q += 10 + (long)sz;
            last = q;
        }
        pre_start  = 10;
        pre_end    = last;
        post_start = last;
        post_end   = tag_end;
    }

    long pre_len  = pre_end  - pre_start;
    long post_len = post_end - post_start;
    long audio_len = fsize - tag_end;
    long new_tag_body = pre_len + (long)nf_size + post_len;
    long new_total    = 10 + new_tag_body + audio_len;

    unsigned char *out = malloc((size_t)new_total);
    if (!out) { free(nf); free(data); return -1; }

    long o = 0;
    memcpy(out, data, 10);
    v2_wr_syncsafe(out + 6, (int)new_tag_body);
    o = 10;
    memcpy(out + o, data + pre_start,  (size_t)pre_len);  o += pre_len;
    memcpy(out + o, nf,                 (size_t)nf_size); o += nf_size;
    memcpy(out + o, data + post_start, (size_t)post_len); o += post_len;
    memcpy(out + o, data + tag_end,    (size_t)audio_len);o += audio_len;

    int rc = v2_save(fname, out, o);

    free(out);
    free(nf);
    free(data);
    return rc;
}

// Function definition for rewrite the whole file in version 1 (or) .v1.

int edit_v1(FILE *fp, int op, char *new_value)
{
    unsigned char tag[3];
    if (fseek(fp, -128, SEEK_END) != 0 ||   fread(tag, 1, 3, fp) != 3 || memcmp(tag, "TAG", 3) != 0) 
        return -1;

    if (op == 6) 
    {
        int g = atoi(new_value);
        if (g < 0 || g > 147) 
            return -1;
        fseek(fp, -1, SEEK_END);
        if (fputc(g, fp) == EOF) 
            return -1;
        fflush(fp);
        return 0;
    }

    long off[6] = { 0, -125, -95, -65, -35, -31 };
    int  len[6] = { 0,   30,  30,  30,   4,  30 };
    if (op < 1 || op > 5) 
        return -1;

    char out[64];
    memset(out, ' ', (size_t)len[op]);
    size_t n = strlen(new_value);
    if ((int)n > len[op]) n = (size_t)len[op];
        memcpy(out, new_value, n);

    fseek(fp, off[op], SEEK_END);
    if (fwrite(out, 1, (size_t)len[op], fp) != (size_t)len[op]) 
        return -1;
    fflush(fp);
    return 0;
}

// Main function definition to do edit operation in .mp3 files.
void edit(FILE *fp, int type, char *fname)
{
    if (fp == NULL)
        return;
    if (type != 1 && type != 2)
        return;

    // detect which tags exist 
    int has_v1 = 0, has_v2 = 0;
    if (type == 2)
        has_v2 = 1;
    {
        unsigned char t[3];
        if (fseek(fp, -128, SEEK_END) == 0 && fread(t, 1, 3, fp) == 3 && memcmp(t, "TAG", 3) == 0)
            has_v1 = 1;
    }

    printf("\n1. Title\n2. Artist\n3. Album\n4. Year\n5. Comment\n6. Genre\n7. Exit\n");
    printf("\n Enter Your Choice: ");

    int op=0;
    scanf(" %d", &op);
    if ( op < 1 || op > 7) 
    {
        printf("Invalid choice.\n");
        return;
    }
    else if(op == 7)
    {
        printf("\nEdit Cancilled....\n");
        return;
    }

    char new_value[1024] = { 0 };
    if (op == 6) {
        int g;
        printf("New Genre index (0-147): ");
        if (scanf("%d", &g) != 1 || g < 0 || g > 147) 
        {
            printf("Invalid genre.\n");
            int c; 
            while ((c = getchar()) != '\n' && c != EOF) {}
            return;
        }
        snprintf(new_value, sizeof new_value, "%d", g);
    } else {
        printf("New value: ");
        if (scanf(" %1023[^\n]", new_value) != 1) {
            printf("Cancelled.\n");
            return;
        }
    }

    /* length check against v1 limits */
    if (has_v1) {
        int maxlen = (op == 4) ? 4 : 30;
        if ((int)strlen(new_value) > maxlen) {
            printf("Too long for v1 (max %d). Aborted.\n", maxlen);
            return;
        }
    }

    /* BOTH writers, in that order */
    int v1_ok = 0, v2_ok = 0;
    if (has_v1)
        v1_ok = edit_v1(fp, op, new_value);         
    if (has_v2)
        v2_ok = edit_v2(fname, op, new_value);      

    printf("Updated%s%s.\n",v1_ok ? " [v1]" : "",v2_ok ? " [v2]" : "");
}