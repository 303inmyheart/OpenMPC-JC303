#!/usr/bin/env python3
"""Editable 270x110 native Sounds-browser artwork, matching the 303 panel."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
ROOT=Path(__file__).resolve().parents[1]
S=4
im=Image.new('RGB',(270*S,110*S),'#B5BAB1');d=ImageDraw.Draw(im)
def rect(bounds,fill,outline=None,r=0,w=1):
 d.rounded_rectangle(tuple(int(v*S) for v in bounds),radius=r*S,fill=fill,outline=outline,width=w*S)
def txt(x,y,s,px,ink):
 d.text((x*S,y*S),s,font=ImageFont.truetype(str(ROOT/'art/DejaVuSans.ttf'),px*S),fill=ink,anchor='lm')
def line(points,ink,w=1):d.line([(x*S,y*S) for x,y in points],fill=ink,width=w*S)
rect((2,2,267,107),'#B5BAB1','#75816A',5)
rect((7,7,262,102),'#172019','#5B7047',4)
txt(18,39,'JC303',40,'#F0F2E5')
txt(20,72,'ACID BASS',17,'#C6F35B')
line([(20,93),(137,93)],'#EE733C',3)
line([(143,93),(249,93)],'#C6F35B',3)
# The smiley is recognizable at the MPC browser's actual small tile size.
x,y,r=231,38,21
d.ellipse(tuple(v*S for v in (x-r,y-r,x+r,y+r)),fill='#F7D848',outline='#A48E24',width=S)
for dx in (-7,7):d.ellipse(tuple(v*S for v in (x+dx-2,y-9,x+dx+2,y-1)),fill='#172019')
d.arc(tuple(v*S for v in (x-13,y-13,x+13,y+13)),15,165,fill='#172019',width=2*S)
line([(188,78),(199,78),(199,66),(211,66),(211,78),(222,78),(222,66),(234,66),(234,78),(248,78)],'#748C52',1)
im=im.resize((270,110),Image.Resampling.LANCZOS)
im.save(ROOT/'art/browser-tile.png')
print('Native Sounds-browser tile: 270x110, JC303 / ACID BASS')
