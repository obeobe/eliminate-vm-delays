# eliminate-vm-delays

On some machines, when running Linux as a VMware guest with 3D graphics acceleration enabled in VMware - inputs in
some programs are arbitrarily delays (for up to ~500 milliseconds, in my estimation).

A couple of workarounds that fix the issue are:
1. Run `glxgears` and keep its window visible.
2. Move the mouse around rapidly.

In other words: while `glxgears` is running and visible, or while the mouse is being moved - the delays disappear.

This info is discussed to some extent here: https://community.broadcom.com/communities/community-home/digestviewer/viewthread?GroupId=7171&MessageKey=dd5ae2e7-e142-4c53-8b9b-3ca30d7d172e&CommunityKey=fb707ac3-9412-4fad-b7af-018f5da56d9f
(This is also where I got the idea to run `glxgears`).

I used the `glxgears` workaround for a while, but it's quite annoying because it has to be visible at all times.

I wrote this `eliminate-vm-delays` tool as an alternative workaround. It occupies the GPU in a similar way to `glxgears`,
but unlike `glxgears`, it doesn't output anything to the screen (it uses a hidden buffer instead).

Also, it supports configurable frame rate, so it allows to achieve the same result with minimal unnecessary load on
the system.

---

Build the code like this:
```
gcc src/eliminate-vm-delays.c -o eliminate-vm-delays -lEGL -lGLESv2
```

If the compilation fails, try to install these prerequisites:
```
sudo apt-get install -y build-essential libegl1-mesa-dev libgles2-mesa-dev
```

You can also try to run the already-compiled `dist/eliminate-vm-delays` (built on a Ubuntu 24.04 VMware guest):
```
chmod +x dist/eliminate-vm-delays
dist/eliminate-vm-delays
```

There are a couple of command-line parameters:

`--verbose` - prints the actual FPS to stdout every second.

`--fps <fps>` - the desired FPS. I had good results with 20 FPS (below that I still had the occasional lags).

---

If you want it to start automatically when the VM boots up, you can do it like this:

Create a `run-eliminate-vm-delays.sh` script:
```
#!/bin/bash
/path/to/executable/eliminate-vm-delays --fps 20 &
disown
```

Then, create a `~/.config/autostart/eliminate-vm-delays.desktop` file:
```
[Desktop Entry]
Type=Application
Name=Eliminate VM Delays
Comment=Run eliminate-vm-delays to fix VM input delays
Exec=/path/to/script/run-eliminate-vm-delays.sh
Terminal=false
Categories=Utility;
```

Good luck!
