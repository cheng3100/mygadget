Backup information
==================

This is for backup important information from Internet to local to prevent the online resource invalid someday.

# tech doc
Also backup on `$NAS/work/tech_doc`
	- zhihu blog

# video 
using  `https://github.com/yt-dlp/yt-dlp` to download video from multiple video site including
- youtube
- bilibili

the command usage of `yt-dlp`:
- `yt-dlp.exe --update-to master` : update
- `yt-dlp.exe -o "%(playlist)s/%(playlist_index)s_%(title)s.%(ext)s" https://www.youtube.com/playlist?list=PLhixgUqwRTjwNaT40TqIIagv3b4_bfB7M --proxy 192.168.2.2:7890` : download a entire youtube list with proxy, auto create sub folder with playlist name and auto name the video file
- `yt-dlp.exe -o "%(playlist)s/%(playlist_index)s_%(title)s.%(ext)s" https://space.bilibili.com/633003/lists/1943493?type=season` : downlaod a entire bili list without proxy

The video size is large so not save here, instead save inside the NAS path: `$NAS\work\work_video\tech`


# website

TODO
