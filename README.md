# Bila-ed / CVED — ASCII editor, UTF-8 Vietnamese files

**Author / maintainer:** Long Ngo (Ngô Hoàng Đại Long)  
**Language:** ISO C11 | **License:** MIT | **Reference:** CVNSS4.0 Converter 5.0.0-audit-safe

> ASCII is the interface. Unicode is the data to preserve. CVNSS4.0 is a verified conversion ruleset.

Bila-ed is an `ed`-style line editor for Vietnamese text. On-screen editor output is ASCII-only CVNSS4.0 raw text. Files are written as UTF-8. The decoder was ported into C from the supplied JS v5.0 audit-safe converter. It ships **810 canonical decoding codes** and **867 Unicode-rime encoding entries**, with an explicit ambiguity report and 825 JS-generated parity examples.

## Quick start on Linux or macOS

```sh
make
make test
./cved demo.txt
```

## Windows Command Prompt (CMD)

1. Download/unzip the repository.
2. Install MSYS2 UCRT64 GCC (`pacman -S mingw-w64-ucrt-x86_64-gcc`) and add `C:\msys64\ucrt64\bin` to PATH, unless you already have MinGW-w64 GCC.
3. Run `build-windows.cmd` (creates `dist\cved.exe`).
4. Run `run-demo.cmd`, or `dist\cved.exe demo.txt`.

A ready-built Linux x86-64 binary may be included in release archives. A native Windows EXE must be produced with a Windows C cross-compiler or the included CMD build script. **A Linux executable is not a Windows executable.**

## Interactive commands

```text
cved demo.txt
  a
  toiy iwy
  tizb vidf
  .
  ,p
  V
  P
  w
  q
```

Editor stdout: `toiy iwy` / `tizb vidf` (ASCII). `P` prints Unicode escaped (ASCII). `demo.txt` contains:

```text
tôi yêu
tiếng việt
```

Useful commands: `a` append, `i` insert, `c` change, `d` delete, `p` print raw, `n` numbered raw, `P` escaped UTF-8 diagnostic, `V` verify, `s/old/new/g` raw substitution, `w`, `wq`, `q`, `q!`, `r filename`, `e filename`, `B` Base27 (trits) for current line. `!command` is deliberately **not supported** (safety).

Batch CLI:

```sh
./cved --decode "toiy iwy"         # tôi yêu (explicit UTF-8 stdout command)
./cved --encode "tôi yêu"           # toiy iwy (only if exact round-trip)
./cved --base27 "ABC Z"            # e.g. 000001002222011
./cved --unbase27 "000001002222011"  # ABC Z
```

## What does 'safe round-trip' mean?

For unedited UTF-8 files, the editor preserves UTF-8 bytes (including BOM, CR/LF/CRLF, lack of final newline, empty lines) when re-saving. For Unicode lines without a **provably exact** raw representation, it preserves the original Unicode and prints `[unicode-only]` in raw mode; `P` displays ASCII Unicode escapes. Invalid UTF-8 files are rejected without truncating the original. Writes use a same-directory temporary file + replace-on-success. Windows `MoveFileExA` may have limitations for Unicode file paths; testing is needed on native CMD.

**Many-to-one CVNSS4.0 cannot invert every Unicode word automatically.** Encoding must pass a decode-equality check. Unknown ASCII code words are passed through literally for compatibility with the supplied JS reference; they are not certified as grammatical CVNSS4.0. Therefore this is an **experimental reference implementation**, not a formal proof of linguistic correctness or complete ISO C11 portability.

## Verification

`make test` runs four native C unit programs (buffer, commands, round-trip, UTF-8) and Python integration tests including **825 decode examples** generated from the supplied JS reference. `make strict` compiles with `-Werror` and runs tests. `make asan` builds with AddressSanitizer/UndefinedBehaviorSanitizer (manual runtime testing recommended). `tools/generate_rules.js` regenerates the rule tables and golden set from the audited JS converter. GitHub CI is provided.

**Important:** 100% of the defined test cases passing is not evidence of 100% code-path coverage, full CVNSS4.0 fidelity in all linguistic contexts, or absence of defects. Real Windows executable testing and external fuzz campaigns remain separate release gates.

## Base27 / edge computing

Base27 maps `A–Z + SPACE` to values `0..26` and three ternary digits (`000..222`). It is a research transport representation, **not** ternary CPU execution or guaranteed compression. C11 code can be adapted for embedded serial/ESP32-S3 after file I/O abstractions and resource caps are validated; FPGA support is not claimed.

## Source provenance

The `tools/cvnss4_converter_audit_safe.js` file was supplied by the project owner as a reference, version `5.0.0-audit-safe`, with its internal self-test (23/23). Generated C tables and test fixtures derive from it. Keep upstream CVNSS4.0 authorship and converter attribution distinct from the Bila-ed implementation maintained by Long Ngo. Check copyright and licensing of any third-party CVNSS4.0 data before redistribution.

See [`docs/SPEC.md`](docs/SPEC.md) for the exact invariants and limitations.


## Project website / Trang giới thiệu

**Bila-ed · CVED / GitHub Pages:** https://xulytiengviet.github.io/Bila-ed/

The Vietnamese website (source in [docs/](docs/)) presents the history of Unix ed and CVNSS4.0, software features, CMD/Linux instructions, and a browser-side interactive reference-converter demo. The web demo runs JavaScript, **not** the C executable. For publishing details see [docs/README.md](docs/README.md).
