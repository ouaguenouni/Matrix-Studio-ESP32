#!/usr/bin/env python3
"""Send a still image to MatrixControl. Install Pillow: python3 -m pip install Pillow"""
import argparse
import struct
import urllib.request
import uuid
from PIL import Image, ImageOps

parser = argparse.ArgumentParser()
parser.add_argument('address', help='ESP32 address, e.g. http://192.168.1.42')
parser.add_argument('image', help='Image file; animated images use their first frame')
args = parser.parse_args()
with Image.open(args.image) as src:
    image = ImageOps.exif_transpose(src).convert('RGBA')
    image = ImageOps.contain(image, (64, 32), Image.Resampling.LANCZOS)
    canvas = Image.new('RGBA', (64, 32), (0, 0, 0, 255))
    canvas.alpha_composite(image, ((64-image.width)//2, (32-image.height)//2))
    frame = bytearray()
    for r, g, b in canvas.convert('RGB').getdata():
        frame.extend(struct.pack('<H', ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)))
boundary = 'matrix-' + uuid.uuid4().hex
body = (f'--{boundary}\r\nContent-Disposition: form-data; name="frame"; '
        'filename="frame.rgb565"\r\nContent-Type: application/octet-stream\r\n\r\n').encode()
body += frame + f'\r\n--{boundary}--\r\n'.encode()
request = urllib.request.Request(args.address.rstrip('/') + '/api/frame', data=body, headers={
    'Content-Type': f'multipart/form-data; boundary={boundary}',
    'X-Matrix-Control': '1',
}, method='POST')
with urllib.request.urlopen(request, timeout=10) as response:
    print(response.read().decode())
