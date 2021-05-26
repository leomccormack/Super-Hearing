# Underwater-Super-Hearing

Ever noticed that when you put your head underwater, it is incredibly difficult to localise sound sources? This is largely because the speed of sound in water is significantly faster than in air. The interaural time differences (ITDs) are much shorter, and the way sound waves interact with the geometry of your head and pinna is very different than in air. Since humans have primarily evolved to interpret these spatial hearing cues for sound sources in air, we are simply not used to localising them underwater.

However, if one were to record the underwater sound scene with a hydrophone array, determine the directions of sound sources, and subsequently use this imformation

## Building a suitable hydrophone array

The CAD files/drawings used for 3D printing and builting the tetrahedral hydrophone array employed in the study in [1], can be found in the [**hardware**](hardware) folder. Further details regarding its construction can be found in the paper.

Commercially available hydrophone sensors are a bit larger than the sensors you would typically find 4 sensor A-format microphone array. However, since the speed of sound is around 4-5 times faster in water than in air, their effective size is similar, if not smaller. The radius of the hydrophone array built for [1] is also larger than a comparable microphone array, for this same reason.

![](images/HydrophoneArray_CAD.png)
![](images/HydrophoneArray_GoPro.png)

## Auralising the captured underwater sound scene

The parametric binaural renderer (hodirac_binaural.vst) that was used for this study is detailed further in [2], and can be downloaded from [**here**](http://research.spa.aalto.fi/projects/sparta_vsts/). 

The plug-in uses this FOA input to analyse the directions of sound sources in frequency bands, and uses this information to reproduce the captured sound scene over headphones. However, since the HRIRs it employs for the rendering were measured in air, one can actually localise the underwater sound sources. This is demonstrated in the following video:

[![Underwater binaural reproduction](https://img.youtube.com/vi/3WARepl3lEg/0.jpg)](https://www.youtube.com/watch?v=3WARepl3lEg)
(Please wear headphones)

Although, in this study, the underwater sound scene was recorded and later rendered to 



## References

* [1] Delikaris-Manias, S., McCormack, L., Huhtakallio, I. and Pulkki, V. 2018, May. [**Real-time underwater spatial audio: a feasibility study**](docs/delikaris2018real.pdf). In Audio Engineering Society Convention 144. Audio Engineering Society.
