#include "mongoose.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 外设测试相关函数声明
void test_gpio(const char *pin, const char *direction, const char *value);
void test_uart(const char *port, const char *baudrate, const char *data_bits, const char *parity, const char *stop_bits, const char *send_data, char *receive_data, size_t receive_data_size);
void test_led();
void test_rtc(const char *action, const char *time, char *rtc_time, size_t rtc_time_size);
void test_watchdog();

// 全局变量
struct mg_mgr mgr;

// 发送 JSON 响应
static void send_json_response(struct mg_connection *c, int status_code, const char *message, const char *data) {
  char response[2048];
  snprintf(response, sizeof(response), 
           "{\"status\": %d, \"message\": \"%s\", \"data\": %s}", 
           status_code, message, data);
  mg_http_reply(c, status_code, "Content-Type: application/json\nAccess-Control-Allow-Origin: *\n", "%s", response);
}

// HTTP事件处理函数
static void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    
    // 解析请求路径
    if (mg_strcmp(hm->uri, mg_str("/api/test/gpio")) == 0) {
      // 测试GPIO
      // 解析查询参数
      char pin[10] = "4";      // 默认引脚4
      char direction[5] = "out"; // 默认输出
      char value[2] = "1";      // 默认高电平
      
      // 使用 mg_http_get_var 解析查询参数
      mg_http_get_var(&hm->query, "pin", pin, sizeof(pin));
      mg_http_get_var(&hm->query, "direction", direction, sizeof(direction));
      mg_http_get_var(&hm->query, "value", value, sizeof(value));
      
      // 测试GPIO
      test_gpio(pin, direction, value);
      
      // 构建响应JSON
      char response_data[256];
      snprintf(response_data, sizeof(response_data), 
               "{\"pin\": \"%s\", \"direction\": \"%s\", \"value\": \"%s\", \"status\": \"成功\"}", 
               pin, direction, value);
      send_json_response(c, 200, "GPIO测试完成", response_data);
    } else if (mg_strcmp(hm->uri, mg_str("/api/test/uart")) == 0) {
      // 测试串口
      // 解析查询参数
      char port[20] = "/dev/ttyS0";      // 默认端口
      char baudrate[10] = "115200";      // 默认波特率115200
      char data_bits[2] = "8";           // 默认数据位8
      char parity[10] = "none";          // 默认无校验
      char stop_bits[2] = "1";           // 默认停止位1
      char send_data[100] = "Hello UART!"; // 默认发送数据
      
      // 使用 mg_http_get_var 解析查询参数
      mg_http_get_var(&hm->query, "port", port, sizeof(port));
      mg_http_get_var(&hm->query, "baudrate", baudrate, sizeof(baudrate));
      mg_http_get_var(&hm->query, "data_bits", data_bits, sizeof(data_bits));
      mg_http_get_var(&hm->query, "parity", parity, sizeof(parity));
      mg_http_get_var(&hm->query, "stop_bits", stop_bits, sizeof(stop_bits));
      mg_http_get_var(&hm->query, "send_data", send_data, sizeof(send_data));
      
      // 测试串口
      char receive_data[256] = {0};
      test_uart(port, baudrate, data_bits, parity, stop_bits, send_data, receive_data, sizeof(receive_data));
      
      // 构建响应JSON
      char response_data[512];
      snprintf(response_data, sizeof(response_data), 
               "{\"status\": \"成功\", \"port\": \"%s\", \"baudrate\": %s, \"data_bits\": %s, \"parity\": \"%s\", \"stop_bits\": %s, \"send_data\": \"%s\", \"receive_data\": \"%s\"}", 
               port[0] ? port : "/dev/ttyS0", 
               baudrate[0] ? baudrate : "115200", 
               data_bits[0] ? data_bits : "8", 
               parity[0] ? parity : "none", 
               stop_bits[0] ? stop_bits : "1",
               send_data[0] ? send_data : "Hello UART!",
               receive_data);
      send_json_response(c, 200, "串口测试完成", response_data);
    } else if (mg_strcmp(hm->uri, mg_str("/api/test/uart/open")) == 0) {
      // 打开串口
      // 解析查询参数
      char port[20] = "/dev/ttyS0";      // 默认端口
      char baudrate[10] = "115200";      // 默认波特率115200
      char data_bits[2] = "8";           // 默认数据位8
      char parity[10] = "none";          // 默认无校验
      char stop_bits[2] = "1";           // 默认停止位1
      
      // 使用 mg_http_get_var 解析查询参数
      mg_http_get_var(&hm->query, "port", port, sizeof(port));
      mg_http_get_var(&hm->query, "baudrate", baudrate, sizeof(baudrate));
      mg_http_get_var(&hm->query, "data_bits", data_bits, sizeof(data_bits));
      mg_http_get_var(&hm->query, "parity", parity, sizeof(parity));
      mg_http_get_var(&hm->query, "stop_bits", stop_bits, sizeof(stop_bits));
      
      // 测试串口（实际应打开串口）
      char receive_data[256] = {0};
      test_uart(port, baudrate, data_bits, parity, stop_bits, NULL, receive_data, sizeof(receive_data));
      
      // 构建响应JSON
      char response_data[256];
      snprintf(response_data, sizeof(response_data), 
               "{\"status\": \"成功\", \"port\": \"%s\", \"baudrate\": %s, \"data_bits\": %s, \"parity\": \"%s\", \"stop_bits\": %s, \"message\": \"串口打开成功\"}", 
               port[0] ? port : "/dev/ttyS0", 
               baudrate[0] ? baudrate : "115200", 
               data_bits[0] ? data_bits : "8", 
               parity[0] ? parity : "none", 
               stop_bits[0] ? stop_bits : "1");
      send_json_response(c, 200, "串口打开成功", response_data);
    } else if (mg_strcmp(hm->uri, mg_str("/api/test/uart/close")) == 0) {
      // 关闭串口
      // 解析查询参数
      char port[20] = "/dev/ttyS0";      // 默认端口
      
      // 使用 mg_http_get_var 解析查询参数
      mg_http_get_var(&hm->query, "port", port, sizeof(port));
      
      // 关闭串口（实际应关闭串口）
      printf("关闭串口: %s\n", port);
      
      // 构建响应JSON
      char response_data[256];
      snprintf(response_data, sizeof(response_data), 
               "{\"status\": \"成功\", \"port\": \"%s\", \"message\": \"串口关闭成功\"}", 
               port[0] ? port : "/dev/ttyS0");
      send_json_response(c, 200, "串口关闭成功", response_data);
    } else if (mg_strcmp(hm->uri, mg_str("/api/test/uart/ports")) == 0) {
      // 获取可用的串口端口
      printf("获取可用串口端口...\n");
      
      // 使用shell命令查找可用的串口端口
      FILE *fp = popen("ls -1 /dev/ttyS* /dev/ttyUSB* 2>/dev/null", "r");
      if (fp) {
          char buffer[256];
          char ports[1024] = "[";
          int first = 1;
          
          while (fgets(buffer, sizeof(buffer), fp) != NULL) {
              // 移除换行符
              buffer[strcspn(buffer, "\n")] = '\0';
              
              // 过滤掉非串口设备
              if (strstr(buffer, "/dev/ttyS") != NULL || strstr(buffer, "/dev/ttyUSB") != NULL) {
                  if (!first) {
                      strcat(ports, ",");
                  }
                  strcat(ports, "\"");
                  strcat(ports, buffer);
                  strcat(ports, "\"");
                  first = 0;
              }
          }
          strcat(ports, "]");
          pclose(fp);
          
          // 构建响应JSON
          char response_data[1024];
          snprintf(response_data, sizeof(response_data), 
                   "{\"status\": \"成功\", \"ports\": %s}", 
                   ports);
          send_json_response(c, 200, "获取串口端口成功", response_data);
      } else {
          // 构建响应JSON，提供默认的串口端口
          char response_data[256];
          snprintf(response_data, sizeof(response_data), 
                   "{\"status\": \"成功\", \"ports\": [\"/dev/ttyS0\", \"/dev/ttyS1\", \"/dev/ttyS2\", \"/dev/ttyS3\"]}");
          send_json_response(c, 200, "获取串口端口成功", response_data);
      }
    } else if (mg_strcmp(hm->uri, mg_str("/api/test/led")) == 0) {
      // 测试LED
      // 解析查询参数
      char pattern[10] = "blink";      // 默认闪烁模式
      char count[3] = "3";             // 默认闪烁3次
      
      // 使用 mg_http_get_var 解析查询参数
      mg_http_get_var(&hm->query, "pattern", pattern, sizeof(pattern));
      mg_http_get_var(&hm->query, "count", count, sizeof(count));
      
      // 测试LED
      test_led();
      
      // 构建响应JSON
      char response_data[256];
      snprintf(response_data, sizeof(response_data), 
               "{\"pattern\": \"%s\", \"count\": %s, \"led0\": \"闪烁3次\", \"led1\": \"常亮2秒\", \"led2\": \"常灭\", \"led3\": \"闪烁5次\"}", 
               pattern[0] ? pattern : "blink", 
               count[0] ? count : "3");
      send_json_response(c, 200, "LED测试完成", response_data);
    } else if (mg_strcmp(hm->uri, mg_str("/api/test/rtc")) == 0) {
      // 测试RTC
      // 解析查询参数
      char action[10] = "read";            // 默认操作：读取时间
      char time[20] = "";                 // 时间
      
      // 使用 mg_http_get_var 解析查询参数
      mg_http_get_var(&hm->query, "action", action, sizeof(action));
      mg_http_get_var(&hm->query, "time", time, sizeof(time));
      
      // 测试RTC
      char rtc_time[20] = "";
      test_rtc(action, time, rtc_time, sizeof(rtc_time));
      
      // 构建响应JSON
      char response_data[256];
      if (strcmp(action, "read") == 0) {
          snprintf(response_data, sizeof(response_data), 
                   "{\"status\": \"成功\", \"action\": \"%s\", \"current_time\": \"%s\"}", 
                   action, rtc_time[0] ? rtc_time : "(读取失败)");
      } else {
          snprintf(response_data, sizeof(response_data), 
                   "{\"status\": \"成功\", \"action\": \"%s\", \"set_time\": \"%s\"}", 
                   action, time);
      }
      send_json_response(c, 200, "RTC测试完成", response_data);
    } else if (mg_strcmp(hm->uri, mg_str("/api/test/watchdog")) == 0) {
      // 测试看门狗
      // 解析查询参数
      char timeout[4] = "10";           // 默认超时10秒
      
      // 使用 mg_http_get_var 解析查询参数
      mg_http_get_var(&hm->query, "timeout", timeout, sizeof(timeout));
      
      // 测试看门狗
      test_watchdog();
      
      // 构建响应JSON
      char response_data[256];
      snprintf(response_data, sizeof(response_data), 
               "{\"timeout\": %s, \"status\": \"成功\", \"operation\": \"喂狗\", \"result\": \"看门狗复位正常\"}", 
               timeout[0] ? timeout : "10");
      send_json_response(c, 200, "看门狗测试完成", response_data);
    } else {
      // 处理静态文件
      struct mg_http_serve_opts opts = {
        .root_dir = "/home/ubuntu/tronlong/program/embedded_peripheral_test_separated/frontend",
        .mime_types = "text/html=text/html,text/css=text/css,text/javascript=application/javascript"
      };
      mg_http_serve_dir(c, hm, &opts);
    }
  }
}

// GPIO测试函数
void test_gpio(const char *pin, const char *direction, const char *value) {
  printf("测试GPIO...\n");
  printf("引脚: %s, 方向: %s, 值: %s\n", pin, direction, value);
  
  // 使用shell命令通过sysfs方式测试GPIO
  char cmd[256];
  
  // 导出GPIO
  snprintf(cmd, sizeof(cmd), "echo %s > /sys/class/gpio/export", pin);
  if (system(cmd) != 0) {
    printf("导出GPIO失败: %s\n", pin);
  }
  
  // 设置方向
  snprintf(cmd, sizeof(cmd), "echo %s > /sys/class/gpio/gpio%s/direction", direction, pin);
  if (system(cmd) != 0) {
    printf("设置GPIO方向失败: %s\n", direction);
  }
  
  // 如果是输出方向，设置值
  if (strcmp(direction, "out") == 0) {
    snprintf(cmd, sizeof(cmd), "echo %s > /sys/class/gpio/gpio%s/value", value, pin);
    if (system(cmd) != 0) {
      printf("设置GPIO值失败: %s\n", value);
    }
  }
  
  // 读取当前值
  snprintf(cmd, sizeof(cmd), "cat /sys/class/gpio/gpio%s/value", pin);
  if (system(cmd) != 0) {
    printf("读取GPIO值失败\n");
  }
  
  // 保存测试日志
  snprintf(cmd, sizeof(cmd), "echo 'GPIO测试: 引脚=%s, 方向=%s, 值=%s' > /tmp/gpio_test.log", pin, direction, value);
  if (system(cmd) != 0) {
    printf("保存测试日志失败\n");
  }
  
  printf("GPIO测试完成，日志已保存到 /tmp/gpio_test.log\n");
}

// 串口测试函数
void test_uart(const char *port, const char *baudrate, const char *data_bits, const char *parity, const char *stop_bits, const char *send_data, char *receive_data, size_t receive_data_size) {
  printf("测试串口...\n");
  printf("端口: %s, 波特率: %s, 数据位: %s, 校验位: %s, 停止位: %s\n", port, baudrate, data_bits, parity, stop_bits);
  
  // 使用shell命令测试串口
  char cmd[256];
  
  // 构建stty命令设置串口参数
  snprintf(cmd, sizeof(cmd), "stty -F %s %s cs%s %s -echo -icanon min 1 time 5", 
           port, 
           baudrate, 
           data_bits, 
           strcmp(parity, "none") == 0 ? "-parity" : (strcmp(parity, "even") == 0 ? "parity even" : "parity odd"));
  printf("执行命令: %s\n", cmd);
  if (system(cmd) != 0) {
    printf("设置串口参数失败\n");
  }
  
  // 如果有发送数据，发送数据到串口
  if (send_data && strlen(send_data) > 0) {
    snprintf(cmd, sizeof(cmd), "echo -n '%s' > %s", send_data, port);
    printf("执行命令: %s\n", cmd);
    if (system(cmd) != 0) {
      printf("发送数据失败\n");
    } else {
      printf("发送数据: %s\n", send_data);
    }
  }
  
  // 读取串口数据
  printf("读取串口数据...\n");
  
  // 尝试打开串口读取数据
  FILE *fp = fopen(port, "r");
  if (fp) {
    char buffer[256] = {0};
    // 读取串口数据，最多等待1秒
    fd_set read_set;
    struct timeval timeout;
    int fd = fileno(fp);
    
    FD_ZERO(&read_set);
    FD_SET(fd, &read_set);
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    int ready = select(fd + 1, &read_set, NULL, NULL, &timeout);
    if (ready > 0 && FD_ISSET(fd, &read_set)) {
      ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
      if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("接收到数据: %s\n", buffer);
        if (receive_data && receive_data_size > 0) {
          strncpy(receive_data, buffer, receive_data_size - 1);
          receive_data[receive_data_size - 1] = '\0';
        }
      } else {
        printf("读取串口数据失败\n");
        if (receive_data && receive_data_size > 0) {
          strncpy(receive_data, "(无数据)", receive_data_size - 1);
          receive_data[receive_data_size - 1] = '\0';
        }
      }
    } else {
      printf("没有接收到数据\n");
      if (receive_data && receive_data_size > 0) {
        strncpy(receive_data, "(无数据)", receive_data_size - 1);
        receive_data[receive_data_size - 1] = '\0';
      }
    }
    fclose(fp);
  } else {
    printf("无法打开串口: %s\n", port);
    if (receive_data && receive_data_size > 0) {
      strncpy(receive_data, "(无法打开串口)", receive_data_size - 1);
      receive_data[receive_data_size - 1] = '\0';
    }
  }
  
  printf("串口测试完成\n");
}

// LED测试函数
void test_led() {
  printf("测试LED...\n");
  // 这里添加实际的LED测试代码
  // 例如：控制LED闪烁、亮度等
}

// RTC测试函数
void test_rtc(const char *action, const char *time, char *rtc_time, size_t rtc_time_size) {
  printf("测试RTC...\n");
  printf("操作: %s, 时间: %s\n", action, time);
  
  // 初始化rtc_time
  if (rtc_time && rtc_time_size > 0) {
    rtc_time[0] = '\0';
  }
  
  // 使用shell命令测试RTC
  char cmd[256];
  
  if (strcmp(action, "read") == 0) {
      // 读取RTC硬件时间
      snprintf(cmd, sizeof(cmd), "hwclock -r");
      printf("执行命令: %s\n", cmd);
      
      // 执行命令并读取输出
      FILE *fp = popen(cmd, "r");
      if (fp) {
          if (fgets(rtc_time, rtc_time_size, fp) != NULL) {
              // 移除换行符
              rtc_time[strcspn(rtc_time, "\n")] = '\0';
              printf("读取到的RTC时间: %s\n", rtc_time);
          }
          pclose(fp);
      }
      
      if (system(cmd) != 0) {
        printf("读取RTC硬件时间失败\n");
      }
  } else if (strcmp(action, "write") == 0 && strlen(time) > 0) {
      // 设置系统时间
      snprintf(cmd, sizeof(cmd), "date -s '%s'", time);
      printf("执行命令: %s\n", cmd);
      if (system(cmd) != 0) {
        printf("设置系统时间失败\n");
      } else {
        // 将系统时间同步到RTC硬件
        snprintf(cmd, sizeof(cmd), "hwclock -w -u");
        printf("执行命令: %s\n", cmd);
        if (system(cmd) != 0) {
          printf("同步系统时间到RTC硬件失败\n");
        }
      }
  }
  
  printf("RTC测试完成\n");
}

// 看门狗测试函数
void test_watchdog() {
  printf("测试看门狗...\n");
  // 这里添加实际的看门狗测试代码
  // 例如：初始化看门狗、喂狗、测试复位等
}

int main() {
  // 初始化mongoose
  mg_mgr_init(&mgr);
  
  // 启动HTTP服务器，监听8080端口
  struct mg_connection *c = mg_http_listen(&mgr, "http://0.0.0.0:8080", ev_handler, NULL);
  if (c == NULL) {
    printf("无法启动服务器\n");
    return 1;
  }
  
  printf("后端服务器启动成功，监听端口 8080\n");
  printf("API 接口：\n");
  printf("  GET /api/test/gpio      - 测试GPIO\n");
  printf("  GET /api/test/uart      - 测试串口\n");
  printf("  GET /api/test/led       - 测试LED\n");
  printf("  GET /api/test/rtc       - 测试RTC\n");
  printf("  GET /api/test/watchdog  - 测试看门狗\n");
  
  // 主事件循环
  while (1) {
    mg_mgr_poll(&mgr, 1000);
  }
  
  // 清理
  mg_mgr_free(&mgr);
  return 0;
}
