# status-bar
My own status-bar that is used in dwm

---
## Compilation:
```
git clone https://github.com/nogaems/status-bar
cd status-bar
gcc bar.c -o bar -lXt -lX11 -lXext -lxkbfile -Wall
```
## Usage:

Do `bar` executable and	place it to some local path which intended to your own
binaries (`~/.local/bin/` for example):
```
chmod +x ./bar
mv bar ~/.local/bin/ 
```
Next you should	be sure	that your path for local binaries is included in your 
$PATH variable,	if it's	not, do	this manually. In file `~/.bashrc` add this line:
```
PATH=$PATH:~/.local/bin
```
After do that, you should do some changes with your `~/.xinitrc` file. In case if 
you use 
`dwm` you have to place this code **above** than the last line placed (before 
`exec dwm`):
```
while true; do
   xsetroot -name "$( bar )"
   sleep 1
done &
```
Then you have to restart dwm by pressing a shortcut (`C-S-q` by default).
After that you will have a pretty nice minimalistic status bar like this:
![example](https://link.to/the-pic)

