#!/usr/bin/python3
import youtube_dl
import sys
import os
import glob
args=len(sys.argv)
if(args>1):
    default = {
        'format': 'best',  # choice of quality
        'outtmpl': '~/videos/%(title)s.%(ext)s',         # name the file the ID of the video
    }
    ydl1 = youtube_dl.YoutubeDL(default)
    for link in sys.argv[1:]:
        try:
            ydl1.download([link])
        except:
            print("download error "+link)
            files=glob.glob('~/videos/%(title)s.%(ext)s')
            for part in files:
                    os.remove(part)
