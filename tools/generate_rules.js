/* Deterministic table generator from the user-supplied audit-safe JS source.
   The source is only executed locally for generating comparison data. */
const fs=require('fs'),vm=require('vm'),path=require('path');
const root=path.resolve(__dirname,'..');
const source=process.argv[2]||path.join(root,'tools','cvnss4_converter_audit_safe.js');
let js=fs.readFileSync(source,'utf8');
const marker='  const PUBLIC_AUDIT = audit();';
if(!js.includes(marker))throw Error('unknown converter source');
js=js.replace(marker, '  globalThis.__TABLES = { BASE_ROWS, PATCH_54, PATCH_56, CANONICAL_PREFERRED, INITIAL_PAIRS, LEGACY_FORWARD_ALIASES };\n'+marker);
let context={module:{exports:{}},globalThis:{}};
vm.runInNewContext(js,context,{timeout:15000,filename:source});
let api=context.module.exports,tab=context.globalThis.__TABLES;
if(!api.selfTest().ok)throw Error('JS reference selfTest failed');
let rows=[...tab.BASE_ROWS.map(r=>[r[0],r[2]]),...tab.PATCH_54,...tab.PATCH_56];
let codes=[...new Set(rows.map(r=>r[1]))].sort();
let fwd=new Map();for(let [u,c] of rows)if(!fwd.has(u))fwd.set(u,c);
for(let [u,c] of tab.LEGACY_FORWARD_ALIASES)if(!fwd.has(u))fwd.set(u,c);
const cand=new Map();for(let [u,c] of rows){if(!cand.has(c))cand.set(c,[]);if(!cand.get(c).includes(u))cand.get(c).push(u);}
const preferred=new Map(codes.map(code=>[code,tab.CANONICAL_PREFERRED[code] || cand.get(code)[0]]));
// Some short codes are intentionally ambiguous; selected values must follow JS policy.
const esc=s=>JSON.stringify(s).replace(/\\u2028|\\u2029/g,' ');
let header='/* Auto-generated from v5.0.0-audit-safe reference. Do not edit by hand. */\n';
header+='typedef struct { const char *key; const char *value; } CvedRule;\n';
header+='static const CvedRule CVED_DEC_RULES[] = {\n'+codes.map(c=>`  {${esc(c)}, ${esc(preferred.get(c))}},`).join('\n')+'\n};\n';
header+='static const CvedRule CVED_ENC_RULES[] = {\n'+[...fwd].sort((a,b)=>a[0].localeCompare(b[0],'vi')).map(([u,c])=>`  {${esc(u)}, ${esc(c)}},`).join('\n')+'\n};\n';
fs.writeFileSync(path.join(root,'src','rules_generated.h'),header);
fs.writeFileSync(path.join(root,'rules','cvnss4.rules'),'# CVNSS4.0 v5.0 audit-safe: code<TAB>canonical Unicode; UTF-8 NFC\n'+codes.map(c=>`${c}\t${preferred.get(c)}`).join('\n')+'\n');
fs.writeFileSync(path.join(root,'rules','exceptions.tsv'),'# code<TAB>canonical<TAB>candidates (pipe-separated)\n'+[...cand].filter(([c,l])=>l.length>1).sort((a,b)=>a[0].localeCompare(b[0])).map(([c,l])=>[c,preferred.get(c),l.join('|')].join('\t')).join('\n')+'\n');
let test=[...codes.map(c=>[c,api.fromCvss(c).cqn]),...['toiy','iwy','vidf','ily','idb','tizb','wizy','ses','hed','mod','mof','vos','tizb vidf','TOIY IWY','Tizb'].map(c=>[c,api.fromCvss(c).cqn])];
fs.writeFileSync(path.join(root,'tests','golden','decode.tsv'),test.map(r=>r.join('\t')).join('\n')+'\n');
fs.writeFileSync(path.join(root,'tests','golden','reference_audit.json'),JSON.stringify(api.audit(),null,2)+'\n');
console.log(JSON.stringify({reference:api.audit(),selfTest:api.selfTest(),decoding:codes.length,forward:fwd.size,golden:test.length}));
