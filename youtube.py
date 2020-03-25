#!/usr/bin/python3
import youtube_dl
import sys
import os
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
        'quiet':True,
    }
    ydl = youtube_dl.YoutubeDL(default)
    result = ydl.extract_info(link , download=False)
    if(result["is_live"]):
        with open("C:\\Users\\danig\\AppData\\Local\\Temp\\youtube.txt", "a") as myfile:
            myfile.write(link+"\n")
        print("directo "+link)
    else:
        try:
            ydl.download([link])
        except:
            print("download error "+link)
            filename = ydl.prepare_filename(result)
            os.remove(filename+".part")
