# RocketLeagueTPManager
## A GUI for managing player and ball positions

https://bakkesplugins.com/plugins/view/320

I got tired of having to manually type in coordinates just to move stuff around so this is my solution.

The whole point of this was to have a GUI over the console calls to 'player' or 'ball'.
If you go to the plugin settings tab, I have the same thing rendering there as the window would.

You can click+drag the individual fields but it's finicky so I'd recommend just double clicking or shift click to type a value in.

There are a couple teleport features if you're just exploring a giant map and wanna congregate some players:

 - There's a combobox that you can use to pick a choice followed by the two buttons
 - The "to here" button means that whatever is in the combo box will get teleported to the current item we're looking at. 
 - The "above here" button means that it'll be placed with a bit of a vertical air gap. That way collisions ideally won't happen. 

If you prefer the window you can bind `togglemenu tpmanager`

Currently, since player names are broken, empty strings will turn into Player 0, Player 1, etc... so you'll kinda need to guess on that.
