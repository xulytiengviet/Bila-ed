/* Bila-ed / CVED GitHub Pages interactive demo — MIT, Long Ngo.
   The web lab uses the supplied CVNSSConverter JS 5.0 reference.
   It does NOT emulate the native C binary, nor a full ed interpreter. */
(() => {
  "use strict";
  const $ = (id) => document.getElementById(id);
  const converter = globalThis.CVNSSConverter;
  const input = $("demo-input");
  const output = $("demo-output");
  const decodeButton = $("mode-decode");
  const encodeButton = $("mode-encode");
  const status = $("engine-status");
  const message = $("demo-message");
  const footer = message.closest(".demo-footer");
  const saveButton = $("save-btn");
  let mode = "decode";
  let currentOutput = "";
  let saveAllowed = true;
  let toastTimer;

  function showToast(text) {
    const el = $("toast");
    el.textContent = text;
    el.hidden = false;
    clearTimeout(toastTimer);
    toastTimer = setTimeout(() => { el.hidden = true; }, 3000);
  }
  async function copyText(text) {
    try {
      if (!navigator.clipboard || !navigator.clipboard.writeText) {
        throw new Error("Clipboard API không sẵn có");
      }
      await navigator.clipboard.writeText(text);
      showToast("Đã sao chép vào clipboard");
    } catch (_err) {
      showToast("Không thể sao chép tự động. Hãy chọn và sao chép thủ công.");
    }
  }
  function byteCount(text) {
    if (typeof TextEncoder !== "undefined") return new TextEncoder().encode(text).length;
    return new Blob([text]).size;
  }
  function updateLab() {
    footer.classList.remove("warning");
    const source = input.value;
    if (!converter) {
      currentOutput = "";
      output.textContent = "Engine CVNSS4.0 chưa sẵn sàng.";
      message.textContent = "Không thể tải bộ chuyển đổi tham chiếu.";
      saveButton.disabled = true;
      return;
    }
    try {
      if (mode === "decode") {
        const result = converter.fromCvss(source);
        currentOutput = result.cqn;
        saveAllowed = true;
        if (source && result.cqn === source) {
          message.textContent = "Không thấy biến đổi. Dữ liệu ASCII có thể là chuỗi literal hoặc mã chưa nhận diện.";
          footer.classList.add("warning");
        } else {
          let ambiguity = 0;
          for (const word of source.match(/[A-Za-z]+/g) || []) {
            const info = converter.inspectWord(word);
            if (info && info.ambiguous) ambiguity++;
          }
          if (ambiguity) {
            message.textContent = "Đã giải mã theo chính sách canonical; có " + ambiguity + " từ thuộc nhóm mã mơ hồ.";
            footer.classList.add("warning");
          } else {
            message.textContent = "Đã chuyển mã bằng engine tham chiếu. Xem file Unicode ở ô bên phải.";
          }
        }
      } else {
        const result = converter.fromCqn(source);
        const raw = result.cvss;
        currentOutput = raw;
        let decoded = "";
        try {
          decoded = converter.fromCvss(raw).cqn;
        } catch (_err) {
          decoded = "";
        }
        const ascii = /^[\x00-\x7F]*$/.test(raw);
        saveAllowed = decoded === source && ascii;
        if (!saveAllowed) {
          footer.classList.add("warning");
          message.textContent = "Cảnh báo: mã hóa không đảo ngược chính xác hoặc chứa ký tự ngoài ASCII. Không lưu bản raw này.";
        } else {
          message.textContent = "Kiểm tra ngược thành công: decode(encode(Unicode)) khớp nguyên văn đầu vào.";
        }
      }
      output.textContent = currentOutput;
      $("count-output").textContent = byteCount(currentOutput) + " bytes";
      $("demo-count").textContent = source.length + " input chars";
      saveButton.disabled = !saveAllowed;
      saveButton.title = saveAllowed ? "Lưu tệp văn bản" : "Không thể lưu raw chưa vượt kiểm tra đảo ngược";
    } catch (err) {
      saveAllowed = false;
      currentOutput = "";
      output.textContent = "[conversion error]";
      message.textContent = "Lỗi chuyển đổi: " + (err && err.message ? err.message : "không xác định");
      footer.classList.add("warning");
      saveButton.disabled = true;
    }
  }
  function setMode(next) {
    mode = next;
    const decode = next === "decode";
    decodeButton.classList.toggle("active", decode);
    encodeButton.classList.toggle("active", !decode);
    decodeButton.setAttribute("aria-pressed", String(decode));
    encodeButton.setAttribute("aria-pressed", String(!decode));
    $("input-label").textContent = decode ? "INPUT / CVNSS4.0 RAW" : "INPUT / UNICODE TIẾNG VIỆT";
    $("output-label").textContent = decode ? "OUTPUT / UNICODE UTF-8" : "OUTPUT / CVNSS4.0 ASCII";
    input.value = decode ? "toiy iwy\ntizb vidf" : "tôi yêu\ntiếng việt";
    input.spellcheck = !decode;
    updateLab();
  }
  decodeButton.addEventListener("click", () => setMode("decode"));
  encodeButton.addEventListener("click", () => setMode("encode"));
  input.addEventListener("input", updateLab);
  $("sample-btn").addEventListener("click", () => {
    input.value = mode === "decode" ? "toiy iwy\ntizb vidf\nses hed" : "tôi yêu\ntiếng việt\nsẽ hề";
    updateLab();
    input.focus();
  });
  $("clear-btn").addEventListener("click", () => { input.value = ""; updateLab(); input.focus(); });
  $("copy-btn").addEventListener("click", () => copyText(currentOutput));
  $("save-btn").addEventListener("click", () => {
    if (!saveAllowed) return;
    const filename = mode === "decode" ? "bila-ed-unicode.txt" : "bila-ed-raw.txt";
    const blob = new Blob([currentOutput], { type: "text/plain;charset=utf-8" });
    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    a.remove();
    setTimeout(() => URL.revokeObjectURL(url), 1500);
    showToast("Đã tạo tệp " + filename);
  });

  if (converter) {
    const audit = converter.audit();
    const test = converter.selfTest();
    status.textContent = test.ok
      ? "ENGINE v" + audit.version + " · " + audit.ambiguityGroups + " mã mơ hồ"
      : "ENGINE: tự kiểm tra chưa đạt";
    status.classList.toggle("warn", !test.ok);
  } else {
    status.textContent = "ENGINE UNAVAILABLE";
  }
  updateLab();

  // Base27: A=0, ... Z=25, SPACE=26, each represented by exactly 3 base-3 digits.
  function updateTrit() {
    const box = $("trit-output");
    const hint = $("trit-summary");
    const source = $("trit-input").value.toUpperCase();
    box.textContent = "";
    if (!/^[A-Z ]*$/.test(source)) {
      hint.textContent = "Chỉ chấp nhận A–Z và dấu cách (27 ký hiệu).";
      hint.style.color = "var(--red)";
      return;
    }
    hint.style.color = "";
    let result = "";
    for (const character of source) {
      const value = character === " " ? 26 : character.charCodeAt(0) - 65;
      const digits = value.toString(3).padStart(3, "0");
      result += digits;
      const token = document.createElement("span");
      token.className = "trit";
      token.textContent = digits;
      token.title = character === " " ? "SPACE = 26" : character + " = " + value;
      box.appendChild(token);
    }
    hint.textContent = source.length + " ký hiệu · " + result.length + " trít · " +
      (source.length ? result : "chưa có dữ liệu");
  }
  $("trit-input").addEventListener("input", updateTrit);
  updateTrit();

  // Responsive navigation
  const toggle = $("menu-toggle");
  const menu = $("site-nav");
  toggle.addEventListener("click", () => {
    const visible = menu.classList.toggle("open");
    toggle.setAttribute("aria-expanded", String(visible));
    toggle.setAttribute("aria-label", visible ? "Đóng menu" : "Mở menu");
  });
  menu.querySelectorAll("a").forEach((a) => {
    a.addEventListener("click", () => {
      menu.classList.remove("open");
      toggle.setAttribute("aria-expanded", "false");
      toggle.setAttribute("aria-label", "Mở menu");
    });
  });

  document.querySelectorAll("[data-copy]").forEach((btn) => {
    btn.addEventListener("click", () => copyText(btn.getAttribute("data-copy") || ""));
  });
})();
