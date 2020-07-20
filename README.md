# Foosball ![CI](https://github.com/mtszkw/foosball/workflows/CI/badge.svg) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Foosball is an open-source application created by students for academic purposes. It allows to process and analyze video recordings of [table soccer](https://en.wikipedia.org/wiki/Table_football) games and:
- detect a table (using [Aruco](https://docs.opencv.org/3.1.0/d5/dae/tutorial_aruco_detection.html) markers placed on table),
- detect and track a ball,
- detect red and blue players,
- count score (based on ball motion and position)

You can watch [this demonstration video](https://www.youtube.com/watch?v=QQ1gPN9S_Fs) (click [here](https://www.youtube.com/watch?v=YIwG6P5TcKs) for original raw input video) too see how it works in practice. In section at the bottom of this page, you can also find a few screenshots from the video recording in different phases of processing.

#### Requirements
- C++17 compiler
- OpenCV == 3.4.1 <sub>(not tested on newer versions)</sub>
- CMake >= 3.10.0 <sub>(not tested on newer versions)</sub>

#### Configuration
After you build the project using a build system of your choice, an executable binary file created requires JSON config file named `configuration.json` to work. Sample configuration can be seen in `config_example.json` file which contains all the options needed to run the application (see exaplanation in the table below). You use that file or copy it to create your own configuration with other values.

<table>
  <tr>
    <th><sub>Parameter</sub></th>
    <th><sub>Short description</sub></th>
  </tr>
  <tr>
    <td><sub>videoPath</sub></td>
    <td><sub>A path to game video file</sub></td>
  </tr>
  <tr>
    <td><sub>videoSkipFramesStep</sub></td>
    <td><sub>If video FPS rate is too high, it is possible to skip `x` frames after each processed frame</sub></td>
  </tr>
  <tr>
    <td><sub>arucoDictionaryPath</sub></td>
    <td><sub>A path to black and white bitmap images with aruco symbols</sub></td>
  </tr>
  <tr>
    <td><sub>arucoDetectorConfigPath</sub></td>
    <td><sub>(optional) A path to YAML file with aruco detector parameters (see [OpenCV documentation](https://docs.opencv.org/3.4.1/d1/dcd/structcv_1_1aruco_1_1DetectorParameters.html))</sub></td>
  </tr>
  <tr>
    <td><sub>calibPerformCalibration</sub></td>
    <td><sub>If true, camera calibration will be performed at the beginning</sub></td>
  </tr>
  <tr>
    <td><sub>calibConfigPath</sub></td>
    <td><sub>TBD</sub></td>
  </tr>
  <tr>
    <td><sub>calibInitConfigPath</sub></td>
    <td><sub>(optional) TBD</sub></td>
  </tr>
  <tr>
    <td><sub>gameTableWidth</sub></td>
    <td><sub>Width of the output image with table</sub></td>
  </tr>
  <tr>
    <td><sub>gameTableHeight</sub></td>
    <td><sub>Height of the output image with table</sub></td>
  </tr>
</table>

#### Screenshots

<p align="center">
  <img src="https://i.imgur.com/mWvHaUU.png" width="640" height="430" alt="Source video frame"/>
</p>
<p align="center">
  <img src="https://i.imgur.com/cycBv94.png" width="640" height="380" alt="Undistorted video frame"/>
</p>
<p align="center">
  <img src="https://i.imgur.com/CIdNn4A.png" width="640" height="380" alt="Result video frame"/>
</p>
