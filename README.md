# CS184 Final Project
# Realistic Lens Simulation

Authors: Alex Hao, Arthur Yin, Cindy Chen, Hanchen Wang

## Notes on this version:
<p> The lens part is fully completed and bug-free, so it can be used while testing the AF algorithm, updated GUI commands are documented below. Note that the API between the zoom lens and the AF algo has been changed to the variable Camera::focalDistance, which is also used in the GUI's manual focus control. Ideas for the AF part are also updated below. More information can be found in the milestone webpage. -Alex </p>

## Part 1: Multi-Element Zoom Lens

### New GUI functionalities:

<p> "W" and "T" keys can now zoom the lens wider or tighter, respectively. There are six levels of adjustments. </p>

<p> "K" and "L" keys still narrows down and opens up the aperture, respectively, but LensRadius is now limited to between 0 and 1, with 1 being the maximum aperture and 0 being a pinhole model. The default value for LensRadius is 0. </p>

<p> ";" and " ' " keys still make the lens focus closer and farther, respectively. But since now the AF algorithm and this manual focus controll share the same variable Camera::focalDistance, this input may be overridden by the AF algorithm. The default value for FocalDistance is -3. </p>

## Part 2: Contrast-Detection Auto Focus

### Some ideas:

<p> 1. The auto focus algorithm talks to the lens exclusively via the Camera::focalDistance attribute, so the output of the algorithm should only be an adjustment of that attribute, which can be positive or negative. </p>

<p> 2. Run the auto focus algorithm before the actual rendering begins. Once the algo calculates the final value for Camera::focalDistance, the full rendering can start using that value. </p>

<p> 3. Set a "focus patch" in the screen center, about 20 pixels by 20 pixels. The algorithm simply renders that patch with many different Camera::focalDistance values, and terminates once it finds a value that makes the patch look sharp (that is, decreasing or increasing Camera::focalDistance only makes the patch more blurry). </p>
    
<p> 4. Large increments of Camera::focalDistance are useful at the start, while fine-grained increments are useful when close to the goal value.

<p> 5. A challenge is to define sharpness, i.e. how can you quantify the sharpness of an image patch. </p>

<p> 6. This algo may not always succeed in finding the correct focus, and that is ok, since its success depends on where the center patch points to. If it points to an area with distinct lines, the focusing will be easier; if it points to, for instance, a pure white wall, it can't possibly focus correctly. So we probably want to let the center patch point to a good spot during testing and debugging. </p>
