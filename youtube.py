#!/usr/bin/python3
import youtube_dl
import sys
import os
import glob
args=len(sys.argv)
if(args>1):
    folder=""
    link=sys.argv[1]
    if(args>2):
        folder=sys.argv[2]
        folder=folder+'/'
    default = {
            'format': 'best',  # choice of quality
            'outtmpl': '~/videos/'+folder+'%(title)s.%(ext)s',         # name the file the ID of the video
            }
    ydl1 = youtube_dl.YoutubeDL(default)
    try:
        ydl1.download([link])
    except:
        print("download error "+link)
        files=glob.glob('~/videos/'+folder+"*.mp4.part")
        for part in files:
            os.remove(part)
