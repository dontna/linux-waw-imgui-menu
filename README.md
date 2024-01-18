# linux-waw-imgui-menu
A small external WaW Zombies cheat menu, made with ImGUI.

# Why?
This was made so I could learn about ImGUI. I also ended up learning about: ptrace, threads and makefiles. I had to learn a lot, however it was a very fun project!

# What's in the menu?
The menu comes with the following:
  - Godmode
  - Infinite Ammo
  - Points Changer
  - Name Spoofer

# How to build?
To build follow these steps:

1. Clone from GitHub
`git clone  https://github.com/dontna/linux-waw-imgui-menu.git`

2. cd into the directory
`cd linux-waw-imgui-menu`

3. use the included makefile to make the project.
`make -f Makefile` or just `make`

You can also just download the binary from the releases tab!

# Will this be updated?
No, most likely not. Call of Duty: World at War, has static addresses; it's unlikely to get updates. Even if it did, it's unlikely the update would change those addresses. Basically there's no point to update this.

# It doesn't work!
If the menu doesn't open, it could be a few things.

1. This needs to run as sudo, for memory editing.
`sudo ./waw_trainer`
2. The game needs to be open, before you start the trainer.
3. This was created with the Steam version of the game.
4. This is a linux cheat, its not intended to be used on Windows. (if that wasn't already clear)

# Taking this code
Please take this code and use it however, I hope this can serve as a base for other linux game hackers to be able to create the projects you want to! Good luck!

# Images
![gen_stats_trainer](https://github.com/dontna/linux-waw-imgui-menu/assets/85905830/56abe87c-c4c8-46c9-a402-a75a4d7d62cd)
![toggles_trainer](https://github.com/dontna/linux-waw-imgui-menu/assets/85905830/4b9d2639-0932-47db-9029-b40e338b366a)
![points_tab_trainer](https://github.com/dontna/linux-waw-imgui-menu/assets/85905830/c6562bd5-f02b-48af-9b5e-d58f4cd91eee)
![name_changer_trainer](https://github.com/dontna/linux-waw-imgui-menu/assets/85905830/36f86120-c7ee-48cd-943a-43b6e9210a80)
