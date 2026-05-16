#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
创建简单的程序图标
"""

try:
    from PIL import Image, ImageDraw, ImageFont

    # 创建256x256的图像
    size = 256
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # 绘制红色圆形背景
    margin = 10
    draw.ellipse([margin, margin, size-margin, size-margin], fill=(196, 30, 58, 255))

    # 绘制金色五角星
    center = size // 2
    star_size = 80

    # 简化的五角星绘制
    def draw_star(draw, cx, cy, size, fill):
        points = []
        for i in range(10):
            angle = i * 36 - 90
            radius = size if i % 2 == 0 else size // 2
            x = cx + radius * 0.8 * (angle if False else __import__('math').cos(__import__('math').radians(angle)))
            y = cy + radius * 0.8 * (angle if False else __import__('math').sin(__import__('math').radians(angle)))
            points.append((x, y))
        draw.polygon(points, fill=fill)

    draw_star(draw, center, center, star_size, (255, 215, 0, 255))

    # 保存为PNG
    img.save('resources/icon.png')

    # 保存为ICO
    img.save('resources/icon.ico', format='ICO', sizes=[(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])

    print("图标创建成功: resources/icon.png 和 resources/icon.ico")

except ImportError:
    print("需要安装PIL库: pip install Pillow")
    print("或者手动准备图标文件: resources/icon.ico")
