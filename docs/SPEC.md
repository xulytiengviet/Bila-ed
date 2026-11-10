# Bila-ed Specification — v0.1.0 (experimental)

## Architecture

`CLI (ASCII stdin/stdout) -> command interpreter -> line buffer -> canonical CVNSS4.0 decoder -> Unicode UTF-8 file`.

- Screen content: ASCII keystrokes; Unicode-only data is represented by printable ASCII placeholder and diagnostic Unicode escapes.
- Storage: original UTF-8 Unicode bytes per line, with original EOL. BOM is recorded at file level.
- Conversion: deterministic, word-tokenized, onset + canonical rime; 810 decode codes and 867 encode rime entries generated from JS v5.0 audit-safe.
- Ambiguity: 56 explicit canonical policies; five critical short codes `ed/es/od/of/os` are covered by the audit reference.
- Invariants: `(raw, uni)` exact equality after `decode(raw)` for new text; on file load `encode(uni)` is accepted only if `decode(encode(uni)) == uni`. Non-exact lines stay Unicode-only. No unrequested transliteration.
- Unknown input ASCII words: intentionally preserved literally, like JS reference; not proof of lexical validity.

## CLI editing semantics

0-based internal line addresses, 1-based user-facing. `a` after current line (position zero on empty buffer); `i` before current. `p` raw; `n` numbered raw; `P` ASCII escaped Unicode diagnostic. `s/old/new/[g]` is a **literal** substring substitution on raw, not regex. A single `.` ends append input. Empty lines are supported. All substitutions are staged before mutation.

Commands resembling `ed` are a subset, **not complete POSIX ed compatibility**. There is no `!` shell escape, marks, undo, global commands, or regex pattern addresses.

## File model

Read: binary mode with validated UTF-8; empty file = zero lines; preserve CR, LF, CRLF, last EOL absent, UTF-8 BOM; reject NUL/invalid UTF-8; prevent partial buffer replacement on parse failure.

Write: preflight entire buffer; create temporary file `name.cvedtmp.PID.N` exclusively beside target; write all lines with retained original EOL; flush, filesystem sync, close; replace original. On failure retain modified buffer and the previous target file. POSIX implementation does not yet fsync parent directory and cannot guarantee crash durability at every failure boundary. Windows filename conversion is ANSI `MoveFileExA`, not full Win32 Unicode path handling.

`q` refuses unsaved changes unless `q!`. Error statuses are reported from scripts. File-load errors leave current buffer unchanged.

## Conversion and Unicode

CVNSS4.0 is many-to-one; a canonical decoder cannot guarantee all source Unicode variants. The implementation reproduces the reference decoder's behavior for 825 generated examples. It does not implement dictionary-assisted disambiguation. Input is expected precomposed Unicode NFC; decomposed NFD may be marked Unicode-only. UTF-8 output only on explicit `--decode` or to file.

## Base27

`A=0, B=1, ..., Z=25, SPACE=26`, three base-3 digits per symbol. Uppercase canonical decode. Base27 is offered as a study feature, not an encoding for arbitrary punctuation or Unicode and not a security or compression feature.

## Build / verification status

Linux GCC C11 with POSIX feature macro; native CMD compilation via MinGW-w64 GCC. `make test` golden, unit and integration tests; `make strict` warnings-as-errors; sanitizer build (`make asan`). Coverage remains empirical, not formally exhaustive. Reproducibility requires the reference JS converter as shipped in `tools/`.

## Release blockers for a production 1.0

- native Windows CMD end-to-end tests and Unicode filename support
- fuzzing for malformed UTF-8, random editor commands, extreme input and save failures
- independent CVNSS4.0 validator with full semantic token classification
- independent license audit for all imported conversion tables
- optional preservation of source file metadata and stronger crash durability
- documentation of limitations of C standard streams on diverse terminals
