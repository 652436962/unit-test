# OK3568 OpenHarmony 串口测试与 SPI 测试（命令行测试程序源码）
测试程序 `uarttest` 和 `fltest_spi` 来自 OK3568 Linux 5.10 源码：`app/forlinx/forlinx_cmd` 目录。
源码包：`fltest_cmd.tar.bz2`
解压命令：
```bash
tar xvf fltest_cmd.tar.bz2
```
适配版本：OK3568 OpenHarmony 4.1 / 5.1 通用

---

## 3.8.4 串口测试
### 测试说明
- 串口 3/4/5 短接测试
- 示例使用**串口 3**

### 测试命令
```bash
 fltest_uarttest -d /dev/ttyAS3
```

### 测试输出示例
```
Welcome to uart test
Send test data:
forlinx_uart_test.1234567890...
Read Test Data finished, Read: forlinx_uart_test.123456789...
```

---

## 3.8.6 SPI 测试
### 测试说明
- 短接 **SPI2_MOSI** 与 **SPI2_MISO**

### 测试命令
```bash
fltest_spi -D /dev/spidev2.0
```

### 测试输出示例
```
spi mode:0
bits per word:8
max speed:500000Hz(500KHz)

FF FF FF FF FF FF
40 00 00 00 00 95
FF FF FF FF FF FF
FF FF FF FF FF FF
FF FF FF FF FF FF
DE AD BE EF BA AD
F0 0D
```

---

## 测试程序目录（Linux 源码）
```
app/forlinx/forlinx_cmd/
├── fltest_canfdtest
├── fltest_keytest
├── fltest_screenshooter
├── fltest_spidev_test
├── fltest_uarttest
├── fltest_watchdog
├── fltest_watchdogrestart
├── Makefile
```

---

## 以 UART 测试程序为例：编译与打包到 OpenHarmony
### 1. 交叉编译（使用 OH 4.1 工具链）
1. 复制 `fltest_uarttest` 到 OH 4.1 源码环境
2. 交叉编译器路径：
```
prebuilts/gcc/linux-x86/aarch64/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
```

3. 编译命令：
```bash
aarch64-linux-gnu-gcc uarttest.c -o uarttest -static
```
或修改 Makefile 中 `CC` 后执行 `make`。

### 2. 放入 OH 源码目录
```
device/board/forlinx/ok3568/tools/uarttest/
```

### 3. 编写 BUILD.gn
使用 `ohos_prebuilt_etc` 将可执行文件安装到 `/system/bin`。

---

要不要我帮你把**uarttest 和 fltest_spi 对应的完整 BUILD.gn 模板**直接写好，你复制就能用？
直接给你**可直接复制使用的完整 BUILD.gn 模板**，适配 OK3568 OpenHarmony 4.0 / 4.1 / 5.0，包含 **uarttest + fltest_spi** 两个工具。
# OK3568 全系列测试工具 **BUILD.gn 全套模板**
直接复制即用，包含：**UART、SPI、CAN、按键、屏幕截图、看门狗、看门狗重启**，适配 OpenHarmony 4.0/4.1/5.0。

## 一、最终目录结构（照着建）
```
device/board/forlinx/ok3568/tools/
├── BUILD.gn                   # 顶层总入口
├── fltest_canfdtest/
│   ├── BUILD.gn
│   └── fltest_canfdtest
├── fltest_keytest/
│   ├── BUILD.gn
│   └── fltest_keytest
├── fltest_screenshooter/
│   ├── BUILD.gn
│   └── fltest_screenshooter
├── fltest_spi/
│   ├── BUILD.gn
│   └── fltest_spi
├── fltest_uarttest/
│   ├── BUILD.gn
│   └── uarttest
├── fltest_watchdog/
│   ├── BUILD.gn
│   └── fltest_watchdog
└── fltest_watchdogrestart/
    ├── BUILD.gn
    └── fltest_watchdogrestart
```

---

## 二、每个工具的 BUILD.gn（直接复制）
### 1. fltest_canfdtest
`tools/fltest_canfdtest/BUILD.gn`
```gn
import("//build/ohos.gni")

ohos_prebuilt_executable("fltest_canfdtest") {
  source = "./fltest_canfdtest"
  modules_install_dir = "bin"
  install_images = ["system"]
  part_name = "rockchip_products"
  install_enable = true
}

group("canfdtest_tool") {
  deps = [ ":fltest_canfdtest" ]
}
```

### 2. fltest_keytest
`tools/fltest_keytest/BUILD.gn`
```gn
import("//build/ohos.gni")

ohos_prebuilt_executable("fltest_keytest") {
  source = "./fltest_keytest"
  modules_install_dir = "bin"
  install_images = ["system"]
  part_name = "rockchip_products"
  install_enable = true
}

group("keytest_tool") {
  deps = [ ":fltest_keytest" ]
}
```

### 3. fltest_screenshooter
`tools/fltest_screenshooter/BUILD.gn`
```gn
import("//build/ohos.gni")

ohos_prebuilt_executable("fltest_screenshooter") {
  source = "./fltest_screenshooter"
  modules_install_dir = "bin"
  install_images = ["system"]
  part_name = "rockchip_products"
  install_enable = true
}

group("screenshooter_tool") {
  deps = [ ":fltest_screenshooter" ]
}
```

### 4. fltest_spi
`tools/fltest_spi/BUILD.gn`
```gn
import("//build/ohos.gni")

ohos_prebuilt_executable("fltest_spi") {
  source = "./fltest_spi"
  modules_install_dir = "bin"
  install_images = ["system"]
  part_name = "rockchip_products"
  install_enable = true
}

group("spi_tool") {
  deps = [ ":fltest_spi" ]
}
```

### 5. uarttest
`tools/fltest_uarttest/BUILD.gn`
```gn
import("//build/ohos.gni")

ohos_prebuilt_executable("uarttest") {
  source = "./uarttest"
  modules_install_dir = "bin"
  install_images = ["system"]
  part_name = "rockchip_products"
  install_enable = true
}

group("uarttest_tool") {
  deps = [ 
    ":uarttest" ,
    ]
}
```

### 6. fltest_watchdog
`tools/fltest_watchdog/BUILD.gn`
```gn
import("//build/ohos.gni")

ohos_prebuilt_executable("fltest_watchdog") {
  source = "./fltest_watchdog"
  modules_install_dir = "bin"
  install_images = ["system"]
  part_name = "rockchip_products"
  install_enable = true
}

group("watchdog_tool") {
  deps = [ ":fltest_watchdog" ]
}
```

### 7. fltest_watchdogrestart
`tools/fltest_watchdogrestart/BUILD.gn`
```gn
import("//build/ohos.gni")

ohos_prebuilt_executable("fltest_watchdogrestart") {
  source = "./fltest_watchdogrestart"
  modules_install_dir = "bin"
  install_images = ["system"]
  part_name = "rockchip_products"
  install_enable = true
}

group("watchdogrestart_tool") {
  deps = [ ":fltest_watchdogrestart" ]
}
```

---

## 三、顶层总入口 BUILD.gn
`tools/BUILD.gn`
```gn
import("//build/ohos.gni")

group("tools") {
  deps = [
    "fltest_canfdtest:canfdtest_tool",
    "fltest_keytest:keytest_tool",
    "fltest_screenshooter:screenshooter_tool",
    "fltest_spi:spi_tool",
    "fltest_uarttest:uarttest_tool",
    "fltest_watchdog:watchdog_tool",
    "fltest_watchdogrestart:watchdogrestart_tool",
  ]
}
```

---

## 四、加入产品编译（根据文档不需要）
打开：
**vendor/forlinx/ok3568/config.json**

在 `parts` 数组里添加：
```json
{
  "part": "rockchip_products",
  "module": "//device/board/forlinx/ok3568/tools:tools"
}
```

---

## 五、编译后命令行直接使用
```bash
uarttest -d /dev/ttyS3
fltest_spi -D /dev/spidev2.0
fltest_canfdtest
fltest_keytest
fltest_screenshooter
fltest_watchdog
fltest_watchdogrestart
```

---

## 六、我还能给你配套：
1. **所有工具的使用命令+接线方法**
2. **静态编译脚本（一键编译所有工具）**
3. **OK3568 外设测试全文档（串口/SPI/CAN/按键/看门狗）**

需要我把这些也一次性给你配齐吗？