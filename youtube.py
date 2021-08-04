#!/usr/bin/python3
import sys
import os
import argparse
parser = argparse.ArgumentParser(description='process url')
parser.add_argument('urls', nargs='+', help='videos to download')
parser.add_argument('-f',"--folder", dest="folder", help='folder')

args=parser.parse_args()
import youtube_dl

webm1=False
webm2=False
folder=""
if args.folder:
    folder=args.folder
    folder=folder+'/'
first = {
        'format': '248+251',  # choice of quality
        'outtmpl': '~/videos/'+folder+'%(title)s.%(ext)s',         # name the file the ID of the video
        'quiet':True,
        }
second = {
        'format': '303+251',
        'outtmpl': '~/videos/'+folder+'%(title)s.%(ext)s',
        'quiet':True,
        }
mp4 = {
        'format': 'bestvideo[ext=mp4]+bestaudio[ext=m4a]/mp4',
        'outtmpl': '~/videos/'+folder+'%(title)s.%(ext)s',
        'quiet':True,
        }
podcast = {
        'format': 'worst',
        'outtmpl': '~/videos/'+folder+'%(title)s.%(ext)s',
        'quiet':True,
        }
ydl=youtube_dl.YoutubeDL({'quiet':True})
try:
    for link in args.urls:
        meta = ydl.extract_info(link , download=False)
        formats = meta.get('formats', [meta])
        for f in formats:
            if f["format_id"]=="248":
                webm1=True
            elif f["format_id"]=="303":
                webm2=True
        if(meta["is_live"]):
            print("is live "+link)
        else:
            if(folder=="podcast/"):
                ydl = youtube_dl.YoutubeDL(podcast)
            else:
                if webm1==True:
                    ydl = youtube_dl.YoutubeDL(first)
                elif webm2==True:
                    ydl = youtube_dl.YoutubeDL(second)
                else:
                    ydl = youtube_dl.YoutubeDL(mp4)
            try:
                ydl.download([link])
            except:
                print("download error "+link)
except:
    print("unknown error "+" ".join(args.urls))
