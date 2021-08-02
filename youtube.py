#!/usr/bin/python3
import sys
import os
import argparse
parser = argparse.ArgumentParser(description='process url')
parser.add_argument('urls', nargs='+', help='videos to download')
parser.add_argument('-f',"--folder", dest="folder", help='folder')

args=parser.parse_args()
import youtube_dl

folder=""
if args.folder:
    folder=args.folder
    folder=folder+'/'
first = {
        'format': '248+251',  # choice of quality
        'outtmpl': '~/videos/'+folder+'%(title)s.%(ext)s',         # name the file the ID of the video
        'quiet':True,
        }
last = {
        'format': 'bestvideo[ext=mp4]+bestaudio[ext=m4a]/mp4',  # choice of quality
        'outtmpl': '~/videos/'+folder+'%(title)s.%(ext)s',         # name the file the ID of the video
        'quiet':True,
        }
podcast = {
        'format': 'worst',  # choice of quality
        'outtmpl': '~/videos/'+folder+'%(title)s.%(ext)s',         # name the file the ID of the video
        'quiet':True,
        }
if(folder=="podcast/"):
    ydl = youtube_dl.YoutubeDL(podcast)
else:
    ydl1 = youtube_dl.YoutubeDL(first)
    ydl2 = youtube_dl.YoutubeDL(last)
try:
    for link in args.urls:
        result = ydl2.extract_info(link , download=False)
        if(result["is_live"]):
            print("is live "+link)
        else:
            try:
                ydl1.download([link])
            except:
                try:
                    ydl2.download([link])
                except:
                    print("download error "+link)
except:
    print("unknown error "+" ".join(args.urls))
