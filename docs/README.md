# Bila-ed website (GitHub Pages)

Published page: https://xulytiengviet.github.io/Bila-ed/

## What this website includes

- History of Unix ed, Chữ Việt Nhanh (CVN), Ký Hiệu Dấu (KHD), CVNSS4.0, and Bila-ed.
- Product introduction and description of the C11 native line editor.
- Live **browser-side JavaScript** CVNSS4.0 converter based on the same audit-safe v5.0 source kept under tools/.
- Base27 (A–Z plus SPACE → 3 trits) teaching demo.
- Windows CMD and Linux/macOS setup instructions.
- Technical distinctions: ASCII editor output vs UTF-8 file output, many-to-one ambiguities, preservation of Unicode, and unverified hardware goals.
- References and links to primary-source history and the repository.

## Files

- index.html: page structure and copy.
- style.css: responsive terminal-inspired UI, no CDN or third-party fonts.
- app.js: interactive browser features, local text export, copy controls and accessibility.
- cvnss4_converter_audit_safe.js: exact copy of tools/cvnss4_converter_audit_safe.js, maintained for browser parity.
- favicon.svg: site icon.
- .nojekyll: static delivery without Jekyll.
- ../.github/workflows/pages.yml: automatic GitHub Pages deployment.

## Publish / repository setup

The GitHub Pages workflow deploys ./docs on pushes to main that affect website files. GitHub must allow Pages deployment through GitHub Actions. If automatic enablement is not permitted, open **Settings → Pages → Build and deployment → Source: GitHub Actions**. Wait for the "Publish Bila-ed Website" workflow to succeed, then visit the URL above.

## Run locally

No package installation needed. Open docs/index.html directly, or serve locally:

    python -m http.server 8000 --directory docs

and visit http://localhost:8000/. All conversion runs in the browser. Downloaded sample files are created locally. The website is not a WASM build of the C editor.

## Verification

    node --check docs/app.js
    node --check docs/cvnss4_converter_audit_safe.js
    node -e "console.log(require('./docs/cvnss4_converter_audit_safe.js').selfTest())"

The JavaScript self-test checks reference conversion; it does not prove that the C port matches in all cases. See the separate C CI workflow and 825 golden examples.

## Attribution

Website and C reference implementation: Long Ngo (Ngô Hoàng Đại Long), MIT.
CVNSS4.0 linguistic system: Kiều Trường Lâm & Trần Tư Bình.
Unix ed origin: Ken Thompson and Dennis Ritchie; QED history from Bell Labs.

The original CVNSS4.0 rules are credited to their authors, and the website does not claim to replace the Vietnamese Quốc ngữ orthography.
