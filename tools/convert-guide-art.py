#!/usr/bin/env python3
"""Convert completed guide illustrations into flash-resident RGB565 LVGL assets."""
from pathlib import Path
from PIL import Image
root = Path(__file__).resolve().parents[1]
directory = root/'assets/guide'
parts = ['#include "lvgl.h"\n']
for park in ('jiuzhai','huanglong'):
    source = Image.open(directory/f'{park}-source.png').convert('RGB')
    for suffix, width, height in (('hero',240,80),('scene',216,72),('thumb',72,24)):
        im = source.resize((width,height),Image.Resampling.NEAREST)
        im.save(directory/f'{park}-{suffix}.png')
        data = bytearray()
        for red,green,blue in im.getdata():
            value=((red>>3)<<11)|((green>>2)<<5)|(blue>>3)
            data.extend((value&255,value>>8))
        name=f'guide_{park}_{suffix}'
        parts.append(f'static const uint8_t {name}_pixels[] = {{\n')
        parts.extend('    '+', '.join(f'0x{x:02x}' for x in data[i:i+20])+',\n' for i in range(0,len(data),20))
        parts.append(f'''}};
const lv_image_dsc_t {name} = {{
    .header = {{.magic=LV_IMAGE_HEADER_MAGIC, .cf=LV_COLOR_FORMAT_RGB565, .w={width}, .h={height}, .stride={width*2}}},
    .data_size=sizeof({name}_pixels), .data={name}_pixels,
}};
''')
(directory/'guide_art.c').write_text(''.join(parts))
print('Six RGB565 assets; 145920 bytes of immutable image pixels; no decompression buffer')
