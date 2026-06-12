## 功能介绍
此demo用于验证G2D模块的相关功能，如格式转换/缩放/旋转/YUV图像合成/YUV图像拆分等。

## 使用说明
默认不会把测试bin安装到rootfs中，需要手动执行push.bat脚本，请到testfiles目录找对应功能下的资源文件中.bat，双击执行即可，bat脚本会自动push所需资源文件，然后执行g2d_test，最后把转换之后的文件在pull出来。

================Usage================
g2d_test 0        means: 1*2560x1440 nv21 decompose to 4*720p nv21
g2d_test 1        means: 4*720 nv21 compose to 1*2560x1440 nv21
g2d_test 2        means: 1024x600 rgba vertical flips
================usage================

