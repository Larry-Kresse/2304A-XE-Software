I have a Keithley 2304A benchtop power supply. Keithley PSUs are known for being high-quality, but they are expensive. 

I bought a few 2304As for fairly cheap in 2023. Only issue, the UI was terrible. Squishy membrane keys, 1990s LCD that could only be viewed from head-on, and no barrel jack connectors.

It was designed for automated testing via IEEE-488, so the UI didn't get any design effort. The 2304A had a remote display option,
connected over RS-232. But that remote display has the same awful UI as the 2304A. 

The thing is also a weird hybrid of a linear and switching PSU. It is extremely accurate and handles fast current steps well, and can actually source and sink current.
Yes, you can use it to test a battery charger since it is a true two-quadrant device. It can source up to 5A at 20V, and can sink around 1-2A (I don't remember the limit exactly).
This architecture comes with a cost -- the power draw is around 50W unloaded. So the UI board should also be able to cut power to the 2304A to 
save on energy bills.

I want an OLED display, mechanical keyboard switches, and relay control for fans and power to the PSU on a custom embedded system.

This repo contains the code, and I'll also put hardware design files on this too. 
