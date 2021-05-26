# Super-Hearing

This repository serves as a companion resource for the papers published in [1,2], which investigate technologies for bestowing "super-human-hearing-abilities" onto the user. More specifically, the term "super hearing", in this context, refers to the ability to simultaneously perceive and localise sound sources, in situations where the listener would otherwise not be able to perceive and/or correctly localise them unaided.

The developed technologies rely on the use of sensor arrays to capture the surrounding sound-field. The array sensor signals are then analysed in order to extract spatial parameters over time and frequency, which are used to parametrically describe the sound scene; these include, e.g., the direction-of-arrival (DoA) of prominent sound sources, and indicators of the direct-to-diffuse ratio (DDR). Using this information, and through the use of suitable rendering techniques, signals exhibiting the correct spatial cues can then be reproduced over headphones worn by the user. This therefore allows the user to simultaneously perceive and localise sound sources in the scene.

## [Ultrasonic-Super-Hearing](Ultrasonic-Super-Hearing) 

For the project described in [1], ultrasonic sound sources, which are inaudible to humans, were captured using a 6-sensor ultrasonic microphone array. This microphone array was then mounted to the headphones worn by the user, and the estimated DoAs were used to appropriately reproduce a pitch-shifted signal (down ~3octaves) over the headphone channels. Through this device, the user can not only perceive the captured ultrasonic sound sources (such as sound produced by bats or leaking pipes), but they can also localise these sources in the correct direction.

![](Ultrasonic-Super-Hearing/images/UltrasonicArray.png)

[**Video**]()

## [Underwater-Super-Hearing](Underwater-Super-Hearing) 

In [2], underwater sound sources, which are audible to humans but are typically localised very poorly, were captured by a 4-sensor hydrophone array. Since humans primarily evolved in the air domain, humans are generally much worse at localising sound sources in other mediums, such as underwater. However, by capturing underwater sound sources with a hydrophone array, and reproducing the sources in the analysed DoAs using head-related transfer functions (HRIRs) measured in air, it is possible for the listener to also correctly localise these underwater sound sources.

![](Underwater-Super-Hearing/images/HydrophoneArray_GoPro.jpg)

[**Video**](https://www.youtube.com/watch?v=3WARepl3lEg)

## References

* [1] Pulkki, V., McCormack, L., and González, R. 2021. **Superhuman spatial hearing technology for ultrasonic frequencies**. Accepted at: Nature Scientific Reports.
* [2] Delikaris-Manias, S., McCormack, L., Huhtakallio, I. and Pulkki, V. 2018, May. [**Real-time underwater spatial audio: a feasibility study**](Underwater-Super-Hearing/docs/delikaris2018real.pdf). In Audio Engineering Society Convention 144. Audio Engineering Society.












