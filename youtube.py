#!/usr/bin/python3
import sys
import os
import argparse
parser = argparse.ArgumentParser(description='process url')
parser.add_argument('urls', nargs='+', help='videos to download')
parser.add_argument('-f',"--folder", dest="folder", help='output folder')
parser.add_argument('-q',"--quiet", dest="quiet", help='quiet mode', action='store_true')
parser.set_defaults(quiet=False)

args=parser.parse_args()
import yt_dlp
folder=""
if args.folder:
    folder=args.folder
    folder=folder+'/'
first = {
        'format': '248+bestaudio[ext=webm]',  # choice of quality
        'outtmpl': '~/videos/'+folder+'%(title)s.%(ext)s',         # name the file the ID of the video
        'quiet':args.quiet,
        'noprogress':args.quiet,
        }
second = {
        'format': '303+bestaudio[ext=webm]',
        'outtmpl': '~/videos/'+folder+'%(title)s.%(ext)s',
        'quiet':args.quiet,
        'noprogress':args.quiet,
        }
mp4 = {
        'format': 'bestvideo[ext=mp4]+bestaudio[ext=m4a]/mp4',
        'outtmpl': '~/videos/'+folder+'%(title)s.%(ext)s',
        'quiet':args.quiet,
        'noprogress':args.quiet,
        }
podcast = {
        'format': 'worst',
        'outtmpl': '~/videos/'+folder+'%(title)s.%(ext)s',
        'quiet':args.quiet,
        'noprogress':args.quiet,
        }
ydl=yt_dlp.YoutubeDL({'quiet':True})
try:
    for link in args.urls:
        webm1=False
        webm2=False
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
                ydl = yt_dlp.YoutubeDL(podcast)
            else:
                if webm1==True:
                    ydl = yt_dlp.YoutubeDL(first)
                elif webm2==True:
                    ydl = yt_dlp.YoutubeDL(second)
                else:
                    ydl = yt_dlp.YoutubeDL(mp4)
            try:
                ydl.download([link])
            except:
                print("download error "+link)
except:
    print("unknown error "+" ".join(args.urls))
