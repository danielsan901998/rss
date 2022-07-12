#!/usr/bin/python3
def main(quiet: bool, folder: str, urls) -> None:
    first = {
            'format': '248+bestaudio[ext=webm]',  # choice of quality
            'outtmpl': '~/videos/'+folder+'%(title)s.%(ext)s',         # name the file the ID of the video
            'quiet':quiet,
            'noprogress':quiet,
            }
    second = {
            'format': '303+bestaudio[ext=webm]',
            'outtmpl': '~/videos/'+folder+'%(title)s.%(ext)s',
            'quiet':quiet,
            'noprogress':quiet,
            }
    mp4 = {
            'format': 'bestvideo[ext=mp4]+bestaudio[ext=m4a]/mp4',
            'outtmpl': '~/videos/'+folder+'%(title)s.%(ext)s',
            'quiet':quiet,
            'noprogress':quiet,
            }
    podcast = {
            'format': 'bestaudio',
            'outtmpl': '~/videos/'+folder+'%(title)s.%(ext)s',
            'quiet':quiet,
            'noprogress':quiet,
            }
    import yt_dlp
    ydl=yt_dlp.YoutubeDL({'quiet':True})
    try:
        for link in urls:
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
    except yt_dlp.utils.DownloadError as e:
        if "Premiere" not in e.msg:
            print("unknown error "+" ".join(urls))

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
    main(quiet=args.quiet,folder=folder,urls=args.urls);
