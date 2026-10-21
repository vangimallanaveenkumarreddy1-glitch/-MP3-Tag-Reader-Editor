# 🎵 MP3 Tag Reader & Editor

A **from-scratch C utility** to **read** and **edit** ID3 metadata inside MP3 files — supporting both **ID3v1** and **ID3v2** (2.3 and 2.4) tag formats.

No external libraries. Pure C. Every byte handled manually.

---

## 📌 Overview

MP3 files store their metadata — Title, Artist, Album, Year, Genre, Comment, Track, Cover Art — inside **ID3 tags**. There are two families:

| Format | Location | Characteristics |
|--------|----------|-----------------|
| **ID3v1** | End of file | Fixed 128-byte block, space-padded fields, single-byte genre index |
| **ID3v2** | Start of file | Variable-size, frame-based, supports UTF-8 / UTF-16, album art |

Real MP3 files often carry **both tags**. Most tools edit only one, leaving the other stale.  
This tool **reads and writes both**, keeping them **synchronized** in a single edit operation.

---

## ✨ Features

### View Mode
- Detects and displays **ID3v1**, **ID3v2**, or **both** (if present)
- ID3v1 → Title, Artist, Album, Year, Comment, Genre
- ID3v2 → All text frames (`TIT2`, `TPE1`, `TALB`, `TYER`/`TDRC`, `TCON`, `TRCK`, `COMM`, `TCOM`, `TPUB`, `TENC`, `TSSE`, `TLEN`, `TCOP`) + `APIC` cover-art metadata
- Correctly decodes **ID3v2.3** (big-endian sizes) and **ID3v2.4** (syncsafe sizes)
- Handles **ISO-8859-1**, **UTF-8**, and **UTF-16** text encodings

### Edit Mode
- Edit **Title, Artist, Album, Year, Comment, Genre**
- **One prompt, both tags updated** — v1 and v2 stay in sync
- **Length validated** against the smaller format (30 chars for text, 4 for year) so neither tag is left inconsistent
- **Genre** stored correctly in each format:
  - ID3v1 → 1-byte numeric index (0-147)
  - ID3v2 → text `TCON` frame
- **ID3v1 edited in place** via `fseek` / `fwrite`
- **ID3v2 fully rebuilt** — tag size changes when text length changes, so the file is reconstructed in memory
- **Atomic writes** via temp file + `rename()` — file never corrupted mid-write

### Safety Features
- 🔒 Atomic v2 rewrite (temp → rename)
- 🔒 Overflow-safe integer parsing for genre IDs
- 🔒 `fflush` after every in-place write
- 🔒 Reopen file after v2 edit (rename invalidates old `FILE*`)
- 🔒 Input validation for genre range and field length
- 🔒 `fread` / `fwrite` / `fseek` return-value checks

---

## 🛠️ Build & Run

### Prerequisites
- A C compiler (`gcc`, `clang`, or MSVC)
- Any POSIX shell (Linux, macOS) or WSL on Windows

### Compile
```bash
gcc main.c print.c edit.c -o mp3tag
```

Or use the provided `Makefile`:
```bash
make
```

### Run
```bash
./mp3tag <song_name.mp3>
```

### Example
```bash
$ ./mp3tag sample-12s.mp3

================================================================================
                              MP3 TAG READER
================================================================================

MENU :-
---------
1. VIEW SONG DETILES
2. EDIT SONG DETILES
3. EXIT

Plese Select Your Choise : 1

Id3v2 found !!..

-------- ID3v2.3.0 Tag [ size = 1407 bytes ] ---------

  Title    : Bohemian Rhapsody
  Artist   : Queen
  Album    : A Night at the Opera
  Year     : 1975
  Genre    : 3
  Track    : 11

----------------- ID3v1 Tag -------------------

  Title   : Bohemian Rhapsody
  Artist  : Queen
  Album   : A Night at the Opera
  Year    : 1975
  Comment : Test comment for project
  Genre   :  Unknown
```

### Edit Example
```
MENU :-
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
```

Both tags now show `2026` when viewed again.

---

## 📁 Project Structure

```
mp3-tag-reader/
├── main.c        Entry point — CLI parsing, menu loop, view/edit dispatch
├── print.c       Tag reading & printing (check_type, print_idv1, print_idv2, frame_name, ...)
├── edit.c        Tag writing (edit, edit_v1, edit_v2 + v2_* encoding helpers)
├── main.h        Shared prototypes, GENRE_COUNT constant
├── Makefile      Build script
└── README.md     This file
```

---

## 🧠 Concepts Applied

| Concept | Where |
|---------|-------|
| Binary file I/O | `fread` / `fwrite` / `fseek` / `ftell` throughout |
| Big-endian ↔ little-endian conversion | `v2_rd_be32`, `v2_wr_be32` |
| Syncsafe integer encoding (v2.4 spec) | `v2_rd_syncsafe`, `v2_wr_syncsafe` |
| Frame walking with padding detection | `edit_v2`, `print_idv2` |
| Encoding handling | ISO-8859-1, UTF-8, UTF-16 BOM |
| Memory management | `malloc` / `free` with full error-path cleanup |
| Atomic file replace | `v2_save` — temp file + `rename()` |
| Overflow-safe integer parsing | Genre index validation |
| `FILE*` state management | Reopen after v2 rewrite to avoid stale inode |

---

## ⚠️ Not Supported

- **ID3v2.2** — old 3-byte frame IDs and 3-byte sizes (rare today)
- **ID3v2 unsynchronisation** — rare; treated as raw bytes
- **Compressed / encrypted frames** — spec allows them; not handled
- **Multiple APIC frames** — only the first is reported
- **Windows native builds** — the code uses POSIX-compatible paths (should still compile under MSVC; not tested)

---

## 🚀 Future Improvements

- [ ] Add **ID3v2.2** support
- [ ] Decode and display **APIC image** to a file (extract cover art)
- [ ] Support **UTF-16 decoding** in `print_idv2` for international text
- [ ] Add **batch editing** (edit tags across a directory)
- [ ] Implement **tag conversion** (v2 → v1, v1 → v2)
- [ ] Add a **GUI** or web front-end
- [ ] Unit tests with a suite of sample MP3 files

---

## 👤 Author

**Vangimalla Naveen Kumar Reddy**

- 📧 Email: [vangimallanaveenkumarreddy1@gmail.com](mailto:vangimallanaveenkumarreddy1@gmail.com)
- 💼 LinkedIn: [naveenkumar-reddy-vangimalla](https://www.linkedin.com/in/naveenkumar-reddy-vangimalla-310184415/)

Feel free to reach out for feedback, collaboration, or questions about the code.
---
 

## 🙏 Acknowledgements

- **ID3.org** — the official tag specification
- All the sample MP3 files used for testing during development

---

⭐ If you found this project useful or interesting, please **star the repo**! ⭐
