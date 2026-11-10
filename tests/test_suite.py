#!/usr/bin/env python3
"""Golden parity tests + live editor CLI smoke tests. Requires Python 3."""
import pathlib, subprocess, sys, tempfile, unittest, os
ROOT=pathlib.Path(__file__).resolve().parents[1]
EXE=pathlib.Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'cved'
sys.argv=sys.argv[:1]
UNIT=("test_buffer","test_roundtrip","test_commands","test_utf8")
C_SOURCES=[str(p) for p in (ROOT/'src').glob('*.c') if p.name!='main.c']

def invoke(*args, stdin=None, cwd=None):
 return subprocess.run([str(EXE),*map(str,args)],input=stdin,text=True,encoding="utf-8",errors="replace",capture_output=True,cwd=cwd,timeout=15)

class BilaTests(unittest.TestCase):
 def test_001_unit_C(self):
  for name in UNIT:
   src=ROOT/'tests'/f'{name}.c'
   with self.subTest(name=name),tempfile.TemporaryDirectory() as d:
    path=pathlib.Path(d)/name
    cmd=['gcc','-std=c11','-Wall','-Wextra','-D_POSIX_C_SOURCE=200809L','-I'+str(ROOT/'include'),str(src),*C_SOURCES,'-o',str(path)]
    build=subprocess.run(cmd,capture_output=True,text=True)
    self.assertEqual(build.returncode,0,build.stderr)
    run=subprocess.run([str(path)],cwd=d,capture_output=True,text=True,timeout=10)
    self.assertEqual(run.returncode,0,run.stderr+run.stdout)
 def test_002_golden_C_JS_parity_825(self):
  rows=(ROOT/'tests/golden/decode.tsv').read_text(encoding='utf-8').splitlines()
  self.assertEqual(len(rows),825)
  for row in rows:
   raw,expected=row.split('\t')
   with self.subTest(raw=raw):
    p=invoke('--decode',raw)
    self.assertEqual(p.returncode,0,p.stderr)
    self.assertEqual(p.stdout,expected+'\n')
 def test_003_editor_writes_unicode_only_file(self):
  with tempfile.TemporaryDirectory() as d:
   path=pathlib.Path(d)/'demo.txt'
   script='a\ntoiy iwy\ntizb vidf\n.\n,p\nV\nw\nq\n'
   p=invoke(path,stdin=script)
   self.assertEqual(p.returncode,0,p.stderr)
   self.assertEqual(path.read_text(encoding='utf-8'),'tôi yêu\ntiếng việt\n')
   self.assertTrue(p.stdout.isascii(),p.stdout)
 def test_004_untouched_bytes(self):
  samples=[b'',b'a1',b'abc\n',b'a\r\nb',b'\xef\xbb\xbf\xc3\xa1\r\n',
           'Tôi yêu 👋!\n'.encode('utf-8'),b'\n\n',b'a\rb\r']
  with tempfile.TemporaryDirectory() as d:
   for index,data in enumerate(samples):
    with self.subTest(index=index):
     path=pathlib.Path(d)/f'{index}.txt';path.write_bytes(data)
     p=invoke(path,stdin='w\nq\n')
     self.assertEqual(p.returncode,0,p.stderr)
     self.assertEqual(path.read_bytes(),data)
 def test_005_changed_but_unsaved_file(self):
  with tempfile.TemporaryDirectory() as d:
   path=pathlib.Path(d)/'keep.txt';path.write_bytes(b'original\n')
   p=invoke(path,stdin='a\ntoiy\n.\nq\nq!\n')
   self.assertIn('buffer modified',p.stderr)
   self.assertEqual(path.read_bytes(),b'original\n')
 def test_006_stress_lines(self):
  with tempfile.TemporaryDirectory() as d:
   path=pathlib.Path(d)/'big.txt';n=900
   p=invoke(path,stdin='a\n'+('toiy iwy\n'*n)+'.\nwq\n')
   self.assertEqual(p.returncode,0,p.stderr)
   self.assertEqual(path.read_text(encoding="utf-8").count('tôi yêu'),n)
 def test_007_base27_roundtrip(self):
  p=invoke('--base27','ABC XYZ')
  self.assertEqual(p.returncode,0)
  back=invoke('--unbase27',p.stdout.strip())
  self.assertEqual(back.stdout,'ABC XYZ\n')
 def test_008_invalid_utf8_rejected_preserve_file(self):
  with tempfile.TemporaryDirectory() as d:
   path=pathlib.Path(d)/'invalid.txt';data=b'valid\n\xffgarbled\n';path.write_bytes(data)
   p=invoke(path,stdin='w\n')
   self.assertNotEqual(p.returncode,0)
   self.assertEqual(path.read_bytes(),data)
 def test_009_no_shell_execution(self):
  with tempfile.TemporaryDirectory() as d:
   path=pathlib.Path(d)/'pwned'
   p=invoke(stdin=f'!touch {path}\nq\n')
   self.assertFalse(path.exists())
 def test_010_unicode_may_not_roundtrip_exact(self):
  p=invoke('--encode','á')
  if p.returncode==0:
   self.assertEqual(invoke('--decode',p.stdout.strip()).stdout,'á\n')
  p=invoke('--encode','🌿')
  self.assertNotEqual(p.returncode,0)

if __name__=='__main__':
 unittest.main(verbosity=2)
