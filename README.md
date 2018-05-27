# status-bar
My own status-bar I'm using with dwm

## Compilation:
```
git clone https://github.com/nogaems/status-bar
cd status-bar
```
Edit [these](https://github.com/nogaems/status-bar/blob/master/bar.c#L41) two lines 
according to your own system. Then you should compile it:
```
gcc bar.c -o bar -lXt -lX11 -lXext -lxkbfile -Wall
```
## Usage:
Make `bar` executable and place it by some local path which is intended as your own 
local binaries path (`~/.local/bin/` for example):
```
chmod +x ./bar
mv bar ~/.local/bin/ 
```
Next you should	make sure that your path for local binaries is included in your $PATH 
variable, if it's not, do this manually. In file `~/.bashrc` add this line:
```
PATH=$PATH:~/.local/bin
```
After that you should do some changes with your `~/.xinitrc` file. In case if you use 
`dwm` you have to place this code **above** than the last line placed (before `exec dwm`):
```
bar &
```
Then you have to restart dwm by pressing a shortcut (`C-S-q` by default).
After that you will have a pretty nice minimalistic status bar like this:

![example](https://github.com/nogaems/status-bar/blob/screenshot/bar-screen.png)

