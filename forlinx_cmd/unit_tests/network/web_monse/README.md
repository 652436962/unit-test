# RV1126 设备仪表盘

一个基于 Mongoose 的轻量级设备监控仪表盘，用于实时监控和管理 RV1126 设备的状态和 AI 功能。

## 项目结构

```
web_monse/
├── device_dashboard.c    # 主程序，基于 Mongoose 的 HTTP 服务器
├── mongoose.c            # Mongoose 库源文件
├── mongoose.h            # Mongoose 库头文件
├── Makefile              # 编译配置文件
└── dashboard/            # 前端仪表盘文件
    ├── index.html        # 主页面
    ├── styles.css        # 样式文件
    └── dashboard.js      # 前端 JavaScript 逻辑
```

## 功能特性

### 设备监控
- 实时 CPU 使用率监控
- 内存使用率监控
- 设备温度监控
- 系统运行时间

### AI 功能
- AI 推理计数统计
- 检测结果实时展示
- AI 控制接口

### API 接口
- `GET /api/device/status` - 获取设备状态
- `POST /api/ai/control` - AI 控制命令
- `POST /api/config/update` - 配置更新
- `GET /api/system/info` - 获取系统信息

## 技术栈

- **后端**：C 语言 + Mongoose 库
- **前端**：HTML5 + CSS3 + JavaScript (ES6+)
- **构建工具**：Makefile

## 编译与运行

### 编译

```bash
# 进入项目目录
cd web_monse

# 编译项目
make
```

### 运行

```bash
# 直接运行
./device_dashboard

# 或使用 Makefile 提供的快捷命令
make run
```

### 访问仪表盘

编译并运行后，在浏览器中访问：

```
http://localhost:38080
```

## 开发说明

### 项目依赖

- **Mongoose 库**：嵌入式 Web 服务器库，已包含在项目中
- **GCC 编译器**：用于编译 C 代码

### 编译选项

- `CFLAGS = -Wall -Wextra -O2 -g`：开启警告、优化和调试信息
- `LDFLAGS = -lm`：链接数学库

### 清理构建文件

```bash
make clean
```

### 重新编译

```bash
make rebuild
```

## 设备状态数据

设备状态 API (`/api/device/status`) 返回以下 JSON 格式数据：

```json
{
  "cpu_usage": 45.7,
  "memory_usage": 62.3,
  "temperature": 42.5,
  "ai_count": 123,
  "last_detection": "检测到3个人脸",
  "uptime": 3600
}
```

## 系统信息

系统信息 API (`/api/system/info`) 返回以下 JSON 格式数据：

```json
{
  "device": "RV1126",
  "version": "1.0.0",
  "sdk": "Rock-X 2.0",
  "model": "YOLOv5s"
}
```

## 自定义配置

### 修改监听端口

在 `device_dashboard.c` 文件中修改 `main()` 函数里的监听端口：

```c
// 监听 38080 端口
if (mg_http_listen(&mgr, "http://0.0.0.0:38080", fn, NULL) == NULL) {
    printf("❌ 启动服务失败，端口可能被占用\n");
    return 1;
}
```

### 调整设备状态更新频率

在 `dashboard.js` 文件中修改 `fetchInterval` 变量：

```javascript
const fetchInterval = 1000; // 1秒更新一次
```

## 浏览器兼容性

- Chrome 80+
- Firefox 75+
- Safari 13+
- Edge 80+

## 许可证

MIT License

## 贡献

欢迎提交 Issue 和 Pull Request！

## 联系方式

如有问题或建议，请通过以下方式联系：

- 项目地址：https://github.com/your-repo/web_monse
- 邮箱：your-email@example.com
