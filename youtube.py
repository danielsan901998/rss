#!/usr/bin/python3
from typing import List
import shutil
from os.path import expanduser
import yt_dlp
import sys
import os
from yt_dlp.postprocessor.common import PostProcessor

video_priority = ["247", "248", "303", "136"]  # webm formats first, then mp4
audio_priority = ["250", "251", "140"]  # opus formats preferred, then m4a
audio_only = False

class PostProcess(PostProcessor):
    def __init__(self, folder):
        PostProcessor.__init__(self)
        self.folder = folder

    def run(self, information):
        try:
            in_path=information["filepath"]
            shutil.move(in_path, expanduser("~")+"/videos/"+self.folder)
        except Exception as e:
            print(e)
        return [], information

def find_first_priority_match(priority_list, formats):
  for codec in priority_list:
    for f in formats:
      if codec in f["format_id"]:
        return f
  return None

def format_builder(ctx):
    selected=[]
    video_found = []
    audio_found = []
    for f in ctx["formats"]:
        format_id = f["format_id"]
        if format_id in video_priority:
            video_found.append(f)
            continue
        if format_id in audio_priority:
            audio_found.append(f)
            continue
        if "language_preference" in f and f["language_preference"]!=10:
            continue
        for audio_id in audio_priority:
            if audio_id in format_id:
                audio_found.append(f)
    if not audio_only:
        video_codec = find_first_priority_match(video_priority,video_found)
        if video_codec:
            selected.append(video_codec)
    audio_codec = find_first_priority_match(audio_priority,audio_found)
    if audio_codec:
        selected.append(audio_codec)
    if len(selected)==0:
        return []
    ext = "webm"
    for f in selected:
        if f["ext"] != "webm":
            ext = "mkv"
    return [{
        'requested_formats': selected,
        'format': "+".join(item["format"] for item in selected),
        'format_id': "+".join(item["format_id"] for item in selected),
        'ext': ext,
        }]

def download(quiet: bool, folder: str, urls: List[str]) -> None:
    global audio_only
    audio_only = folder == "podcast"
    for link in urls:
        try:
            ydl = yt_dlp.YoutubeDL(
                {
                    "format": format_builder,
                    "outtmpl": "/tmp/%(title)s.%(ext)s",
                    "quiet": quiet,
                    "noprogress": quiet,
                    'postprocessors': [
                        {
                            'key': 'SponsorBlock',
                            'categories': ['sponsor','selfpromo']
                            },
                        {
                            'key': 'ModifyChapters',
                            'remove_sponsor_segments': ['sponsor','selfpromo']
                            }
                        ],
                }
            )
            ydl.add_post_processor(PostProcess(folder))
            try:
                ydl.download([link])
            except yt_dlp.utils.DownloadError as e:
                print("download error " + link+": "+e.msg)
        except yt_dlp.utils.DownloadError as e:
            if "Premiere" not in e.msg and "live event" not in e.msg:
                print("download error " + link+": "+e.msg)
    sys.stdout.flush()


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="process url")
    parser.add_argument("urls", nargs="+", help="videos to download")
    parser.add_argument("-f", "--folder", dest="folder", help="output folder")
    parser.add_argument(
        "-q", "--quiet", dest="quiet", help="quiet mode", action="store_true"
    )
    parser.set_defaults(quiet=False)

    args = parser.parse_args()
    folder = ""
    if args.folder:
        folder = args.folder
    download(quiet=args.quiet, folder=folder, urls=args.urls)
