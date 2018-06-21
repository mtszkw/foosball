# Foosball [![Build Status](https://travis-ci.com/mtszkw/foosball.svg?token=e2qczaZansf4M2Pmpkha&branch=master)](https://travis-ci.com/mtszkw/foosball) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

### About
Open-source application to analyze Foosball games based on video recordings analysis and processing.

### Features
- Camera calibration and distortion correction,
- Game table detection based on [Aruco](https://docs.opencv.org/3.1.0/d5/dae/tutorial_aruco_detection.html) markers,
- Ball detection and motion tracking,
- Score counting based on ball motion and position,
- Red and blue players detection,
- [FastBuild](http://www.fastbuild.org/docs/home.html) targets generation for distributed compilation,
- Simple and user-friendly graphical user interface

### Installation
Requirements:
- C++17,
- OpenCV == 3.4.1,
- CMake >= 3.10.0

### Usage example
Executable binary created after building process requires JSON configuration file named `configuration.json` to work.  
Configuration options which have to specified are shown in `config_example.json` file. One should copy (or just rename) this file to `configuration.json` and set internal values adequately.
Meaning of particular configurations:
- **videoPath** - path where video recording file can be found,
- **videoSkipFramesStep** - if video FPS rate is too high, it is possible to skip `n` frames after each processed frame,
- **arucoDictionaryPath** - path where aruco symbols are stored as black and white bitmap,
- **arucoDetectorConfigPath** - (optional) path to yaml file containing aruco detector parameters [OpenCV Doc](https://docs.opencv.org/3.4.1/d1/dcd/structcv_1_1aruco_1_1DetectorParameters.html),
- **calibPerformCalibration** - ...
- **calibConfigPath** - ...
- **calibInitConfigPath** - (optional) ...
- **gameTableWidth** - Width of the output image with table,
- **gameTableHeight** - Height of the output image with table.

### Screenshots
Below we present few screenshots from our application: 1) source video raw frame, 2) frame after distortion removal and 3) result frame including detected table area, ball/players markers and simple GUI with hotkeys and game score displayed.
<p align="center">
  <img src="https://image.ibb.co/iH0q9o/source.png" width="640" height="430" alt="Source video frame"/>
</p>
<p align="center">
  <img src="https://image.ibb.co/cqWxpo/1_dystorcja.png" width="640" height="380" alt="Undistorted video frame"/>
</p>
<p align="center">
  <img src="https://preview.ibb.co/epVJvT/3_result.png" width="640" height="380" alt="Result video frame"/>
</p>
