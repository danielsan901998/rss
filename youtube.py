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
    video_priority = ["247", "248", "303", "136"]  # webm formats first, then mp4
    audio_priority = ["250", "251","140"]  # opus formats preferred, then m4a
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
                        'postprocessors': [{
                            'key': 'FFmpegExtractAudio',
                            'preferredcodec': 'opus',
                        }],
                    })
            else:
                formats = meta.get('formats')
                video_format="bestvideo"
                audio_format="bestaudio"
                if formats:
                    video_found = []
                    audio_found = []
                    for f in formats:
                        format_id = f["format_id"]
                        if format_id in video_priority:
                            video_found.append(format_id)
                        if format_id in audio_priority:
                            audio_found.append(format_id)
                    for f in video_priority:
                        if f in video_found:
                            video_format = f
                            break
                    for f in audio_priority:
                        if f in audio_found:
                            audio_format = f
                            break
                yt_format=video_format+'+'+audio_format
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
