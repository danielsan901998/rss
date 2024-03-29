#!/usr/bin/python3
from typing import List
import shutil
from os.path import expanduser
import yt_dlp
from yt_dlp.postprocessor.common import PostProcessor
class PostProcess(PostProcessor):
    def __init__(self, folder):
        PostProcessor.__init__(self)
        self.folder=folder
    def run(self, information):
        shutil.move(information['filepath'],expanduser("~")+"/videos/"+self.folder)
        return [], information

def download(quiet: bool, folder: str, urls: List[str]) -> None:
    yt_info=yt_dlp.YoutubeDL({'quiet':True})
    for link in urls:
        try:
            meta = yt_info.extract_info(link , download=False)
            if "live_status" in meta:
                if meta["live_status"]!="not_live":
                    continue
            if(folder=="podcast/"):
                ydl = yt_dlp.YoutubeDL({
                        'format': 'bestaudio',  # choice of quality
                        'outtmpl': '/tmp/%(title)s.%(ext)s',
                        'quiet':quiet,
                        'noprogress':quiet,
                    })
            else:
                webm1=False
                webm2=False
                webm3=False
                mp4=False
                opus1=False
                opus2=False
                formats = meta.get('formats')
                if formats:
                    for f in formats:
                        if f["format_id"]=="247":
                            webm1=True
                        elif f["format_id"]=="248":
                            webm2=True
                        elif f["format_id"]=="303":
                            webm3=True
                        elif f["format_id"]=="136":
                            mp4=True
                        if f["format_id"]=="250":
                            opus1=True
                        elif f["format_id"]=="351":
                            opus2=True
                yt_format=""
                if webm1==True:
                    yt_format="247+"
                elif webm2==True:
                    yt_format="248+"
                elif webm3==True:
                    yt_format="303+"
                elif mp4:
                    yt_format="136"
                else:
                    yt_format="bestvideo[ext=mp4]+"
                if opus1==True:
                    yt_format+="250"
                elif opus2==True:
                    yt_format+="251"
                else:
                    yt_format+="bestaudio[ext=m4a]/mp4"
                ydl = yt_dlp.YoutubeDL({
                        'format': yt_format,  # choice of quality
                        'outtmpl': '/tmp/%(title)s.%(ext)s',
                        'quiet':quiet,
                        'noprogress':quiet,
                    })
            ydl.add_post_processor(PostProcess(folder))
            try:
                ydl.download([link])
            except:
                print("download error "+link)
        except yt_dlp.utils.DownloadError as e:
            if "Premiere" not in e.msg and "live event" not in e.msg:
                print("unknown error "+link)

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='process url')
    parser.add_argument('urls', nargs='+', help='videos to download')
    parser.add_argument('-f',"--folder", dest="folder", help='output folder')
    parser.add_argument('-q',"--quiet", dest="quiet", help='quiet mode', action='store_true')
    parser.set_defaults(quiet=False)

    args=parser.parse_args()
    folder=""
    if args.folder:
        folder=args.folder
        folder=folder+'/'
    download(quiet=args.quiet,folder=folder,urls=args.urls);
