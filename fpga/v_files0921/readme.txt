I am saving this version because I am going to modify the code so that the system will be able to let us
change the exposure time and AD converter parameters on the fly. To do so, I will have to constantly compare
the current value @ certain adresses of the xilly_mem_8 with the previous value at the beginning of each
FOT.


Basic info. of this saved version:

Camera Board: 5th board we made
Width of data bus: 10
Camera Configuration: by Qt software on MicroZed
Non-Destructive Readout: implemented
Frame Valid Period: 120Hz
Stability: can run at least 15 minutes wiouth any interruption


Speed: The system no longer just grabs one frame out of every eight frames from the LUPA300, instead it 
       receives every frame under the condition that the LUPA300 works at 120fps. Several tests have proved
       that the xillybus can handle this kind of data throughput(~300Mbit/S). However, the bottleneck arises
       when the data comes to Ethernet port. The TCP/IP protocol can only make it 60fps without Jumbo Frames
       (MTU=1500). On the other hand, the CPU Zynq7000 doesn't support Jumbo Frame intrinsically.
       So far, the only solotion we can think of to make the whole system work at 120fps is to replace the
       TCP/IP protocol with the UDP protocol, that seems to be beyond the scope of the project.