# Super-Hearing

This repository serves as a companion resource for the papers published in [1,2], which investigate technologies for  bestowing "super-human-hearing-abilities" onto the user. More specifically, the term "super hearing" in this context refers to the ability to perceive and localise sound sources, which the listener would otherwise not be able to unaided.

The developed technologies rely on the use of specially designed sensor arrays to capture the surrounding sound-field. The signals of the array sensors are then analysed in order to extract spatial parameters over time and frequency, which used to describe the sound scene parametrically; such as the direction-of-arrival (DoA) of prominant sound sources. Using this information, and through the use of suitable rendering techniques, signals exhbiting the correct spatial cues can be reproduced over headphones worn by the user; thus, allowing them to simalteously perceive and localise the sound sources.

## [Ultrasonic-Super-Hearing](Ultrasonic-Super-Hearing) 

For this project, ultrasonic sound sources, which are inaudible to humans, were captured by a compact 6-sensor ultrasonic microphone array. This microphone array was then mounted to the headphones worn by the user, and the estimated DoAs were used to appropriately reproduce a pitch-shifted signal (down ~3octaves) over the headphone channels. Through this device, the user can not only now perceive the captured ultrasonic sound sources (such as bats or leaking pipes), but they can also localise these sources in the correct direction.

![](Ultrasonic-Super-Hearing/images/UltrasonicArray.png)

[**Video**]()

## [Underwater-Super-Hearing](Underwater-Super-Hearing) 

Here, underwater sound sources, which are indeed audible to humans, but are typically localised poorly, were captured by a compact 4-sensor hydrophone array.  Since humans primarily evolved in air, we are usually much worse at localising sound sources in other mediums; such as underwater. However, by capturing underwater sound sources with a hydrophone array, and reproducing the sources in the analysed DoAs using head-related transfer functions (HRIRs) measured in air, 

![](Underwater-Super-Hearing/images/HydrophoneArray_GoPro.png)

[**Video**](https://www.youtube.com/watch?v=3WARepl3lEg)

## References

* [1] 
* [2] Delikaris-Manias, S., McCormack, L., Huhtakallio, I. and Pulkki, V. 2018, May. [**Real-time underwater spatial audio: a feasibility study**](Underwater-Super-Hearing/docs/delikaris2018real.pdf). In Audio Engineering Society Convention 144. Audio Engineering Society.











