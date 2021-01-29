#!/usr/bin/python3
import youtube_dl
import sys
import os
import argparse
parser = argparse.ArgumentParser(description='process url')
parser.add_argument('urls', nargs='+', help='videos to download')
parser.add_argument('-f',"--folder", dest="folder", help='folder')

args=parser.parse_args()
if args.folder:
    print("folder: "+args.folder)
print("urls: "+",".join(args.urls))
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
    try:
        for link in args.urls:
            result = ydl.extract_info(link , download=False)
            if(result["is_live"]):
                print("is live "+link)
            else:
                try:
                    ydl.download([link])
                except:
                    print("download error "+link)
                    filename = ydl.prepare_filename(result)
                    os.remove(filename+".part")
    except:
        print("unknown error "+",".join(args.urls))
