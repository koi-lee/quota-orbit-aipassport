#!/usr/bin/env python3
"""Generate the guide's reproducible Simplified Chinese font subsets."""
import argparse
import json
import re
import subprocess
from pathlib import Path
p = argparse.ArgumentParser()
p.add_argument('--font', required=True, type=Path)
p.add_argument('--converter', required=True, type=Path)
a = p.parse_args()
root = Path(__file__).resolve().parents[1]
chars = set(map(chr, range(32, 127)))
for source in [*(root/'main').glob('guide_*.[ch]'), root/'main/main.c']:
    for literal in re.findall(r'"([^"\\]*(?:\\.[^"\\]*)*)"', source.read_text()):
        try:
            chars.update(c for c in json.loads('"'+literal+'"') if ord(c)>127)
        except json.JSONDecodeError:
            pass
inventory = ''.join(sorted(chars, key=ord))
(root/'assets/fonts/guide-characters.txt').write_text(inventory+'\n')
for size in (12,14,16,20):
    subprocess.run([str(a.converter), '--font', str(a.font), '--symbols', inventory,
                    '--size', str(size), '--bpp', '4', '--format', 'lvgl', '--no-compress',
                    '--lv-font-name', f'guide_font_{size}', '--lv-include', 'lvgl.h',
                    '--output', str(root/f'assets/fonts/guide_font_{size}.c')], check=True)
print(f'Guide font subset: {len(chars)} code points, four sizes')
