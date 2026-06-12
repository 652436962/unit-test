# unit-test

OK3568 / T527 系列板卡的测试程序合集，包含 Linux 和 OpenHarmony 平台的外设测试工具源码及预编译可执行文件。

## 目录结构

```
unit-test/
├── forlinx_cmd/              # OK3568 Linux 5.10 命令行测试程序（源码+预编译）
├── forlinx_cmd_demo_527/     # T527 平台测试 Demo（NPU、G2D、视频流等）
├── openharmony_cmd/          # OK3568 OpenHarmony 4.1/5.1 适配的测试工具
├── demo_example/             # 示例 Demo
└── .gitignore
```

## 测试工具列表

### forlinx_cmd / openharmony_cmd
| 工具 | 功能 |
|------|------|
| `fltest_uarttest` / `uarttest` | 串口回环测试 |
| `fltest_spidev_test` / `fltest_spi` | SPI 回环测试 |
| `fltest_canfdtest` | CAN FD 测试 |
| `fltest_keytest` | 按键测试 |
| `fltest_screenshooter` | 屏幕截图 |
| `fltest_watchdog` | 看门狗测试 |
| `fltest_watchdogrestart` | 看门狗重启测试 |
| `fltest_tcpclient` | TCP 客户端测试 |
| `fltest_tcpserver` | TCP 服务端测试 |
| `fltest_gps` | GPS 测试 |
| `fltest_hostapd` / `fltest_wifi` | WiFi 热点测试 |

### forlinx_cmd_demo_527 (T527 平台)
| 模块 | 功能 |
|------|------|
| `npu_detect_test` | NPU 目标检测 |
| `npu_detect_face_test` | NPU 人脸检测 |
| `g2d_test` | 2D 图形加速（旋转/缩放/合成/分解） |
| `gif2rgb_test` | GIF 转 RGB |
| `fltest_v4l2_npu` | V4L2 + NPU 视频流（含 Live555 RTSP） |
| `stream_player_test` | 视频流播放测试 |
| `fltest_csitest` | CSI 摄像头测试 |
| `fltest_spidev_test` | SPI 设备测试 |
| `mem_test` | 内存测试 |
| `gpio_test` | GPIO 测试 |
| `quectelCM` | 移远 4G 模组拨号工具 |

## OpenHarmony 编译适配

`openharmony_cmd/` 目录下的测试程序需要交叉编译后放入 OpenHarmony 源码构建。

交叉编译示例（OH 4.1）：
```bash
aarch64-linux-gnu-gcc uarttest.c -o uarttest -static
```

## 相关文档

- [openharmony_cmd/openharmony_test.md](openharmony_cmd/openharmony_test.md) - OpenHarmony 测试程序编译与 BUILD.gn 模板
- 各子目录中的 `ReadMe.md` / `README.md` 文件
