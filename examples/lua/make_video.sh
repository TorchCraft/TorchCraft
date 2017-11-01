#!/bin/bash

set -e

mkdir -p videos

ffmpeg -framerate 25 -i images/img_%05d.ppm -c:v libx264 -profile:v high -crf 20 -pix_fmt yuv420p videos/img.mp4
ffmpeg -framerate 25 -i images/layer_health_%05d.ppm -c:v libx264 -profile:v high -crf 20 -pix_fmt yuv420p videos/layer_health.mp4
ffmpeg -framerate 25 -i images/layer_type_%05d.ppm -c:v libx264 -profile:v high -crf 20 -pix_fmt yuv420p videos/layer_type.mp4
ffmpeg -framerate 25 -i images/layer_player_%05d.ppm -c:v libx264 -profile:v high -crf 20 -pix_fmt yuv420p videos/layer_player.mp4
ffmpeg -framerate 25 -i images/layer_visibility_%05d.ppm -c:v libx264 -profile:v high -crf 20 -pix_fmt yuv420p videos/layer_visibility.mp4
ffmpeg -i videos/layer_health.mp4 -i videos/layer_type.mp4 -i videos/layer_player.mp4 -i videos/layer_visibility.mp4 -i videos/img.mp4 -filter_complex "nullsrc=size=640x480 [base]; [0:v] setpts=PTS-STARTPTS, scale=320x240 [upperleft]; [1:v] setpts=PTS-STARTPTS, scale=320x240 [upperright]; [2:v] setpts=PTS-STARTPTS, scale=320x240 [lowerleft]; [3:v] setpts=PTS-STARTPTS, scale=320x240 [lowerright]; [4:v] setpts=PTS-STARTPTS, scale=320x240, format=yuva420p, colorchannelmixer=aa=0.7 [middle]; [base][upperleft] overlay=shortest=1 [tmp1]; [tmp1][upperright] overlay=shortest=1:x=320 [tmp2]; [tmp2][lowerleft] overlay=shortest=1:y=240 [tmp3]; [tmp3][lowerright] overlay=shortest=1:x=320:y=240 [tmp4]; [tmp4][middle] overlay=shortest=1:x=160:y=120 " -c:v libx264 videos/assembled.mp4
