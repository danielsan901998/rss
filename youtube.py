#!/usr/bin/python3
from typing import List
from typing import Optional
import subprocess
import shutil
import os
from os.path import expanduser
import yt_dlp
import sys
from yt_dlp.postprocessor.common import PostProcessor
class loggerOutputs:
    def __init__(self, quiet: bool):
        self.quiet = quiet
        self.progress = False
    def error(self,msg):
        if "Premiere" not in msg and "live event" not in msg:
            print(msg)
    def warning(self,msg):
        print(msg)
    def debug(self,msg):
        if not self.quiet:
            if msg.startswith('[download] ') and "ETA" in msg:
                if self.progress:
                    print('\x1b[1K\r'+msg,end='')
                else:
                    self.progress=True
                    print(msg,end='')
            else:
                if self.progress:
                    print('\x1b[1K\r'+msg)
                    self.progress=False
                else:
                    print(msg)

video_priority = ["247","136", "248", "302"]  # 720p first 30fps webm as priority
audio_priority = ["250", "251", "140"]  # opus formats preferred, then m4a

class PostProcessMove(PostProcessor):
    def __init__(self, folder: str):
        PostProcessor.__init__(self)
        self.folder = folder

    def run(self, information):
        try:
            in_path=information["filepath"]
            dest_dir = expanduser("~")+"/videos/"+self.folder
            dest_path = os.path.join(dest_dir,os.path.basename(in_path))
            
            if os.path.exists(dest_path):
                os.remove(dest_path)
            shutil.move(in_path, dest_path)
        except Exception as e:
            self.to_screen(f'Error moving: {e}')
        return [], information

class FFmpegSilenceRemovePP(PostProcessor):
    def __init__(self):
        PostProcessor.__init__(self)

    def run(self, info):
        filepath = info['filepath']
        name_without_extension = os.path.splitext(filepath)[0]
        new_filepath = f"{name_without_extension}.opus"
        
        # Construct the silenceremove filter string
        silence_filter = "silenceremove=start_periods=1:stop_periods=-1:start_threshold=-40dB:stop_threshold=-40dB:start_silence=0.4:stop_silence=0.4"

        # FFmpeg command to remove silence
        # -y to overwrite output file without asking
        # -nostdin to prevent ffmpeg from trying to read from stdin
        cmd = [
            'ffmpeg', '-nostdin', '-i', filepath,
            '-af', silence_filter,
            '-b:a', '64K',
            '-f', 'opus',
            '-y', new_filepath
        ]
        
        try:
            self.to_screen(f'Removing silence, destination: {new_filepath}')
            subprocess.run(cmd, stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL)
            subprocess.run(["trim-audio",new_filepath], stdout = subprocess.DEVNULL, stderr = subprocess.DEVNULL)
            info['filepath'] = new_filepath
        except Exception as e:
            self.to_screen(f'Error removing silence: {e}')
            raise yt_dlp.utils.DownloadError

        return [filepath], info

def find_first_priority_match(priority_list, formats):
  for codec in priority_list:
    for f in formats:
      if codec in f["format_id"]:
        return f
  return None

class format_builder:
    def __init__(self, audio_only: bool):
        self.audio_only = audio_only
    def get_format(self,ctx):
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
        if not self.audio_only:
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

def filter_live_and_short(info_dict, *, incomplete: bool) -> Optional[str]:
    if info_dict["was_live"] and "Econocrítica" in info_dict["fulltitle"]:
        return None
    if info_dict["is_live"] or info_dict["was_live"]:
        return "Skip livestream"
    if info_dict["duration"] < 90:
        return "Skip #short"
    return None

def download(quiet: bool, folder: str, urls: List[str], sponsorblock: bool) -> None:
    audio_only = folder == "podcast"
    for link in urls:
        args = {
                "format": format_builder(audio_only).get_format,
                "match_filter": filter_live_and_short,
                "outtmpl": "/tmp/%(title)s.%(ext)s",
                "quiet": quiet,
                "noprogress": quiet,
                'retries': 10,
                "logger": loggerOutputs(quiet),
                'postprocessors': []
                }
        if sponsorblock:
            args['postprocessors'].extend([
                {
                    'key': 'SponsorBlock',
                    'categories': ['sponsor','selfpromo','interaction']
                    },
                {
                    'key': 'ModifyChapters',
                    'remove_sponsor_segments': ['sponsor','selfpromo','interaction']
                    }
                ])
        ydl = yt_dlp.YoutubeDL(args)
        if audio_only:
            ydl.add_post_processor(FFmpegSilenceRemovePP())
        ydl.add_post_processor(PostProcessMove(folder), when="after_move")
        try:
            ydl.download([link])
        except yt_dlp.utils.DownloadError as e:
            if "Premiere" not in e.msg and "live event" not in e.msg:
                print("download error " + link+" "+e.msg)
    sys.stdout.flush()


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="process url")
    parser.add_argument("urls", nargs="+", help="videos to download")
    parser.add_argument("-f", "--folder", dest="folder", help="output folder")
    parser.add_argument(
        "-q", "--quiet", dest="quiet", help="quiet mode", action="store_true"
    )
    parser.add_argument(
        "--no-sponsorblock", dest="sponsorblock", action="store_false", help="Disable SponsorBlock postprocessor (enabled by default)"
    )
    parser.set_defaults(quiet=False)
    parser.set_defaults(sponsorblock=True)

    args = parser.parse_args()
    folder = ""
    if args.folder:
        folder = args.folder
    download(quiet=args.quiet, folder=folder, urls=args.urls, sponsorblock=args.sponsorblock)
