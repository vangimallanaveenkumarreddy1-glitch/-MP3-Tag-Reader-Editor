# 🎵 MP3 Tag Reader & Editor

A command-line utility written in **pure C** that reads, displays, and edits ID3 metadata embedded inside MP3 files — the tag data that stores title, artist, album, year, genre, comment, and cover art.

**No external libraries.** No `taglib`, no `libid3tag`, no `mutagen`. Just `fopen`, `fread`, `fseek`, `fwrite`, `rename`, `malloc`, and `memcpy` — plus the ID3 specification.

---

## 📑 Table of Contents

- [Features](#-features)
- [Supported Formats](#-supported-formats)
- [Build & Run](#-build--run)
- [Usage](#-usage)
- [Commands Reference](#-commands-reference)
- [How It Works](#-how-it-works)
- [Project Structure](#-project-structure)
- [Error Handling](#-error-handling)
- [Limitations](#-limitations)
- [Author](#-author)

---

## ✨ Features

### View Mode (`-v`)
- Read and display **ID3v1** tags (fixed 128-byte trailer at end of file)
- Read and display **ID3v2** tags (frame-based header at start of file)
- Detect and show **both** tags when a file carries both
- Print every recognized text frame: `TIT2`, `TPE1`, `TALB`, `TYER`/`TDRC`, `TCON`, `TRCK`, `COMM`, `TCOM`, `TPUB`, `TENC`, `TSSE`, `TLEN`, `TCOP`
- Show **APIC** cover-art metadata (MIME type, size, picture type)
- Optional filter to view only v1 or only v2

### Edit Mode (`-e`)
- Edit any single field by name — case-insensitive
- Update **both** v1 and v2 tags in one shot when both exist (keeps tags in sync)
- Enforce v1 field limits (30 chars for text, 4 for year) so tags never diverge
- Numeric genre index (0–147) handled correctly per format — byte in v1, text `TCON` in v2
- **Atomic writes** — rebuild to a temp file, `rename()` over the original, so a crash never corrupts audio
- Encoding-aware: ISO-8859-1, UTF-8 (v2.4), UTF-16 with BOM (v2.3 non-ASCII)

---

## 📦 Supported Formats

| Format | Location | Supported |
|---|---|---|
| **ID3v1** | Last 128 bytes of file | ✅ Read + Edit |
| **ID3v1.1** | Last 128 bytes (track # in comment) | ✅ Read |
| **ID3v2.3** | First N bytes (big-endian sizes) | ✅ Read + Edit |
| **ID3v2.4** | First N bytes (synchsafe sizes) | ✅ Read + Edit |
| **ID3v2.2** | First N bytes (3-byte IDs) | ❌ Not supported |

---

## 🛠 Build & Run

### Prerequisites
- `gcc` (or any C99 compiler)
- GNU Make (optional)

### Build

```bash
gcc main.c print.c edit.c -o mp3tag
```

Or with Make:

```bash
make
```

### Quick Test

```bash
./mp3tag sample.mp3 -v
```

---

## 🚀 Usage

```
./mp3tag <song.mp3> -v [v1|v2]
./mp3tag <song.mp3> -e <field> "<value>"
```

### View All Tags

```bash
./mp3tag song.mp3 -v
```

**Output:**

```
================================================================================
                              MP3 TAG READER
================================================================================

Id3v2 found !!..

-------- ID3v2.3.0 Tag [ size = 1413 bytes ] ---------

  Title    : Bohemian Rhapsody
  Artist   : Queen
  Album    : A Night at the Opera
  Year     : 1975
  Genre    : Rock
  Track    : 11

----------------- ID3v1 Tag -------------------

  Title   : Bohemian Rhapsody
  Artist  : Queen
  Album   : A Night at the Opera
  Year    : 1975
  Comment : Test comment for project
  Genre   : Rock

********************* Thank You  *************************
```

### View Only ID3v1 or ID3v2

```bash
./mp3tag song.mp3 -v v1
./mp3tag song.mp3 -v v2
```

### Edit a Field

```bash
./mp3tag song.mp3 -e title   "Bohemian Rhapsody"
./mp3tag song.mp3 -e artist  "Queen"
./mp3tag song.mp3 -e album   "A Night at the Opera"
./mp3tag song.mp3 -e year    1975
./mp3tag song.mp3 -e comment "My favourite track"
./mp3tag song.mp3 -e genre   17
```

**Output:**

```
Updated 'title' successfully.
```

---

## 📖 Commands Reference

### Flags

| Flag | Argument | Description |
|---|---|---|
| `-v` | *(optional)* `v1` or `v2` | View both tags, or only one |

### Edit Fields

| Field keyword | Edits | Max length |
|---|---|---|
| `title` | Song title | 30 chars |
| `artist` | Artist / band | 30 chars |
| `album` | Album name | 30 chars |
| `year` | Release year | 4 chars |
| `comment` | Comment field | 30 chars |
| `genre` | Genre index (0–147) | numeric |

Field names are **case-insensitive**. Values containing spaces **must be quoted**.

### Common Genre Indices

| Index | Genre | Index | Genre |
|---|---|---|---|
| 0 | Blues | 17 | Rock |
| 1 | Classic Rock | 20 | Alternative |
| 2 | Country | 32 | Classical |
| 8 | Jazz | 52 | Electronic |
| 13 | Pop | 127 | Acapella |
| 14 | R&B | 147 | Dance Hall |

Full 148-entry table lives in `GENRES[]` in `print.c`.

---

## 🔬 How It Works

### File Layout

```
┌──────────────┬──────────────────────┬─────────────────────┬───────────────┐
│ ID3v2 header │  ID3v2 frames        │   MP3 audio frames  │  ID3v1 (128B) │
│  (10 bytes)  │  (variable length)   │  (the actual music) │  at file end  │
└──────────────┴──────────────────────┴─────────────────────┴───────────────┘
```

### ID3v1 — In-Place Patching

Every field has a **fixed offset** from the end of file, so editing is trivial:

```c
fseek(fp, -125, SEEK_END);            /* Title field */
fwrite(padded_value, 1, 30, fp);      /* 30 bytes, space-padded */
```

No file-size change → no rebuild → instantaneous even on huge files.

### ID3v2 — Rebuild & Atomic Replace

Frames are **variable-length**. If a title grows from 8 to 20 bytes, the whole tag shifts and the audio bytes must move. So editing goes through five steps:

1. **Load whole file** into memory (`v2_load`)
2. **Walk frames** — decode each frame's size (big-endian for v2.3, synchsafe for v2.4)
3. **Build replacement frame** with correct encoding
4. **Splice** — `pre + new_frame + post + audio` into a new buffer, then rewrite the header's **synchsafe tag size** field
5. **Atomic save** — write to `song.mp3.tmp`, then `rename()` over the original

A `rename()` is atomic on POSIX — consumers see either the whole old file or the whole new file, never a half-written one.

### Synchsafe Integers

ID3v2.4 stores sizes in a special format where **each byte only uses its lower 7 bits**:

```c
/* 4 bytes × 7 bits = 28 usable bits */
int v2_rd_syncsafe(unsigned char *b)
{
    return ((b[0] & 0x7F) << 21) |
           ((b[1] & 0x7F) << 14) |
           ((b[2] & 0x7F) <<  7) |
            (b[3] & 0x7F);
}
```

Forgetting the `& 0x7F` mask means every size reads as garbage and the file won't open.

---

## 📁 Project Structure

```
.
├── main.c        # CLI parsing, view/edit dispatch
├── print.c       # Tag reading & printing
│                 # check_type, print_idv1, print_idv2,
│                 # syncnum, big_to_int, frame_name,
│                 # id3v2_read_pic, remove_space
├── edit.c        # Tag writing
│                 # edit_v1, edit_v2,
│                 # v2_load, v2_save, v2_build_frame,
│                 # v2_rd_be32, v2_rd_syncsafe,
│                 # v2_wr_be32, v2_wr_syncsafe,
│                 # v2_make_text_body
├── main.h        # Shared prototypes, GENRE_COUNT
└── README.md     # This file
```

### Function Map

| File | Function | Purpose |
|---|---|---|
| `main.c` | `main()` | Parse CLI, dispatch to view/edit |
| `print.c` | `check_type()` | Detect v1 / v2 / both / none |
| `print.c` | `print_idv1()` | Print 128-byte v1 tag |
| `print.c` | `print_idv2()` | Walk and print v2 frames |
| `print.c` | `id3v2_read_pic()` | Parse APIC frame metadata |
| `print.c` | `syncnum()`, `big_to_int()` | Decode size fields |
| `edit.c` | `edit_v1()` | Patch 128-byte v1 trailer in place |
| `edit.c` | `edit_v2()` | Rebuild entire v2 tag + audio |
| `edit.c` | `v2_load()`, `v2_save()` | Whole-file load, atomic save |
| `edit.c` | `v2_build_frame()` | Assemble `[ID][size][flags][body]` |

---

## ⚠️ Error Handling

Every failure exits with code **1** and prints a message on `stderr`:

| Condition | Message |
|---|---|
| Wrong arg count | `Invalid Argument Count.` + usage |
| Not `.mp3` | `Error: '<file>' does not have .mp3 extension.` |
| Cannot open | `Error: cannot open '<file>'` |
| Unknown flag | `Error: unknown flag '<flag>'` |
| Unknown field | `Error: unknown field '<name>'` |
| Genre out of range | `Error: genre must be a number between 0 and 147` |
| Value too long (v1) | `Error: value too long for v1 (max N chars).` |
| No tag on file | `Error: file has no ID3 tag; nothing to edit.` |

Success returns **0**. Useful in shell scripts:

```bash
./mp3tag song.mp3 -e title "X" && echo OK || echo FAILED
```

---

## 🚧 Limitations

- **ID3v2.2** (3-byte frame IDs) — not supported
- **Extended headers** — parsed on read but not rebuilt on write
- **Unsynchronisation** — flag is detected but bytes are not de-unsynchronised
- **Multi-value frames** — reads first value only
- **Very large files** — `edit_v2` loads the whole file into memory (fine for typical MP3s; not ideal for 4 GB files)
- **Windows paths** — `rename()` works but the atomic guarantee is weaker on Windows than POSIX

---

## 🧪 Testing

Create a test file with a known tag, then verify read-back:

```bash
# 1. Copy any MP3 with an existing tag
cp sample.mp3 test.mp3

# 2. Read it
./mp3tag test.mp3 -v

# 3. Edit it
./mp3tag test.mp3 -e title "New Title"
./mp3tag test.mp3 -e artist "New Artist"
./mp3tag test.mp3 -e year 2024

# 4. Read it back
./mp3tag test.mp3 -v
```

Expect both `ID3v1` and `ID3v2` sections to show the updated values.

---

## 👤 Author

**Vangimalla Naveen Kumar Reddy**

Built as a deep-dive into binary file formats, C file I/O, and the ID3 specification.

---

## 📜 License

This project is for educational purposes. No warranty. Use at your own risk.
