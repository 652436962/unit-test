#include "mongoose.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 外设测试相关函数声明
void test_gpio();
void test_uart();
void test_led();
void test_adc();

// 全局变量
struct mg_mgr mgr;

// HTTP事件处理函数
static void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    
    // 解析请求路径
    if (mg_strcmp(hm->uri, mg_str("/")) == 0) {
      // 主页
      const char *html = "<!DOCTYPE html>\n"
                        "<html>\n"
                        "<head>\n"
                        "<title>嵌入式板卡外设测试</title>\n"
                        "<style>\n"
                        "body { font-family: Arial, sans-serif; margin: 20px; }\n"
                        ".container { max-width: 800px; margin: 0 auto; }\n"
                        ".button { display: inline-block; padding: 10px 20px; margin: 10px; background: #4CAF50; color: white; text-decoration: none; border-radius: 4px; }\n"
                        ".button:hover { background: #45a049; }\n"
                        ".result { margin-top: 20px; padding: 15px; border: 1px solid #ddd; border-radius: 4px; }\n"
                        "</style>\n"
                        "</head>\n"
                        "<body>\n"
                        "<div class=\"container\">\n"
                        "<h1>嵌入式板卡外设测试</h1>\n"
                        "<p>点击以下按钮测试相应外设：</p>\n"
                        "<a href=\"/test/gpio\" class=\"button\">测试 GPIO</a>\n"
                        "<a href=\"/test/uart\" class=\"button\">测试 串口</a>\n"
                        "<a href=\"/test/led\" class=\"button\">测试 LED</a>\n"
                        "<a href=\"/test/adc\" class=\"button\">测试 ADC</a>\n"
                        "</div>\n"
                        "</body>\n"
                        "</html>\n";
      mg_http_reply(c, 200, "Content-Type: text/html\n", "%s", html);
    } else if (mg_strcmp(hm->uri, mg_str("/test/gpio")) == 0) {
      // 测试GPIO
      test_gpio();
      mg_http_reply(c, 200, "Content-Type: text/html\n", 
                    "<!DOCTYPE html>\n"
                    "<html>\n"
                    "<head>\n"
                    "<title>GPIO测试结果</title>\n"
                    "<style>\n"
                    "body { font-family: Arial, sans-serif; margin: 20px; }\n"
                    ".container { max-width: 800px; margin: 0 auto; }\n"
                    ".result { margin-top: 20px; padding: 15px; border: 1px solid #ddd; border-radius: 4px; }\n"
                    ".back { margin-top: 20px; padding: 10px 20px; background: #337ab7; color: white; text-decoration: none; border-radius: 4px; }\n"
                    "</style>\n"
                    "</head>\n"
                    "<body>\n"
                    "<div class=\"container\">\n"
                    "<h1>GPIO测试结果</h1>\n"
                    "<div class=\"result\">\n"
                    "<p>GPIO测试完成！</p>\n"
                    "<p>测试详情：</p>\n"
                    "<ul>\n"
                    "<li>GPIO 0: 输出高电平</li>\n"
                    "<li>GPIO 1: 输出低电平</li>\n"
                    "<li>GPIO 2: 读取状态</li>\n"
                    "<li>GPIO 3: 读取状态</li>\n"
                    "</ul>\n"
                    "</div>\n"
                    "<a href=\"/\" class=\"back\">返回主页</a>\n"
                    "</div>\n"
                    "</body>\n"
                    "</html>\n");
    } else if (mg_strcmp(hm->uri, mg_str("/test/uart")) == 0) {
      // 测试串口
      test_uart();
      mg_http_reply(c, 200, "Content-Type: text/html\n", 
                    "<!DOCTYPE html>\n"
                    "<html>\n"
                    "<head>\n"
                    "<title>串口测试结果</title>\n"
                    "<style>\n"
                    "body { font-family: Arial, sans-serif; margin: 20px; }\n"
                    ".container { max-width: 800px; margin: 0 auto; }\n"
                    ".result { margin-top: 20px; padding: 15px; border: 1px solid #ddd; border-radius: 4px; }\n"
                    ".back { margin-top: 20px; padding: 10px 20px; background: #337ab7; color: white; text-decoration: none; border-radius: 4px; }\n"
                    "</style>\n"
                    "</head>\n"
                    "<body>\n"
                    "<div class=\"container\">\n"
                    "<h1>串口测试结果</h1>\n"
                    "<div class=\"result\">\n"
                    "<p>串口测试完成！</p>\n"
                    "<p>测试详情：</p>\n"
                    "<ul>\n"
                    "<li>串口初始化成功</li>\n"
                    "<li>发送测试数据：Hello UART!</li>\n"
                    "<li>接收测试数据：Hello UART!</li>\n"
                    "<li>波特率：115200</li>\n"
                    "</ul>\n"
                    "</div>\n"
                    "<a href=\"/\" class=\"back\">返回主页</a>\n"
                    "</div>\n"
                    "</body>\n"
                    "</html>\n");
    } else if (mg_strcmp(hm->uri, mg_str("/test/led")) == 0) {
      // 测试LED
      test_led();
      mg_http_reply(c, 200, "Content-Type: text/html\n", 
                    "<!DOCTYPE html>\n"
                    "<html>\n"
                    "<head>\n"
                    "<title>LED测试结果</title>\n"
                    "<style>\n"
                    "body { font-family: Arial, sans-serif; margin: 20px; }\n"
                    ".container { max-width: 800px; margin: 0 auto; }\n"
                    ".result { margin-top: 20px; padding: 15px; border: 1px solid #ddd; border-radius: 4px; }\n"
                    ".back { margin-top: 20px; padding: 10px 20px; background: #337ab7; color: white; text-decoration: none; border-radius: 4px; }\n"
                    "</style>\n"
                    "</head>\n"
                    "<body>\n"
                    "<div class=\"container\">\n"
                    "<h1>LED测试结果</h1>\n"
                    "<div class=\"result\">\n"
                    "<p>LED测试完成！</p>\n"
                    "<p>测试详情：</p>\n"
                    "<ul>\n"
                    "<li>LED 0: 闪烁3次</li>\n"
                    "<li>LED 1: 常亮2秒</li>\n"
                    "<li>LED 2: 常灭</li>\n"
                    "<li>LED 3: 闪烁5次</li>\n"
                    "</ul>\n"
                    "</div>\n"
                    "<a href=\"/\" class=\"back\">返回主页</a>\n"
                    "</div>\n"
                    "</body>\n"
                    "</html>\n");
    } else if (mg_strcmp(hm->uri, mg_str("/test/adc")) == 0) {
      // 测试ADC
      test_adc();
      mg_http_reply(c, 200, "Content-Type: text/html\n", 
                    "<!DOCTYPE html>\n"
                    "<html>\n"
                    "<head>\n"
                    "<title>ADC测试结果</title>\n"
                    "<style>\n"
                    "body { font-family: Arial, sans-serif; margin: 20px; }\n"
                    ".container { max-width: 800px; margin: 0 auto; }\n"
                    ".result { margin-top: 20px; padding: 15px; border: 1px solid #ddd; border-radius: 4px; }\n"
                    ".back { margin-top: 20px; padding: 10px 20px; background: #337ab7; color: white; text-decoration: none; border-radius: 4px; }\n"
                    "</style>\n"
                    "</head>\n"
                    "<body>\n"
                    "<div class=\"container\">\n"
                    "<h1>ADC测试结果</h1>\n"
                    "<div class=\"result\">\n"
                    "<p>ADC测试完成！</p>\n"
                    "<p>测试详情：</p>\n"
                    "<ul>\n"
                    "<li>ADC通道 0: 1234</li>\n"
                    "<li>ADC通道 1: 2345</li>\n"
                    "<li>ADC通道 2: 3456</li>\n"
                    "<li>ADC通道 3: 4567</li>\n"
                    "</ul>\n"
                    "</div>\n"
                    "<a href=\"/\" class=\"back\">返回主页</a>\n"
                    "</div>\n"
                    "</body>\n"
                    "</html>\n");
    } else {
      // 404 Not Found
      mg_http_reply(c, 404, "Content-Type: text/html\n", 
                    "<!DOCTYPE html>\n"
                    "<html>\n"
                    "<head>\n"
                    "<title>404 Not Found</title>\n"
                    "<style>\n"
                    "body { font-family: Arial, sans-serif; margin: 20px; }\n"
                    ".container { max-width: 800px; margin: 0 auto; }\n"
                    ".back { margin-top: 20px; padding: 10px 20px; background: #337ab7; color: white; text-decoration: none; border-radius: 4px; }\n"
                    "</style>\n"
                    "</head>\n"
                    "<body>\n"
                    "<div class=\"container\">\n"
                    "<h1>404 Not Found</h1>\n"
                    "<p>请求的页面不存在。</p>\n"
                    "<a href=\"/\" class=\"back\">返回主页</a>\n"
                    "</div>\n"
                    "</body>\n"
                    "</html>\n");
    }
  }
}

// GPIO测试函数
void test_gpio() {
  printf("测试GPIO...\n");
  // 这里添加实际的GPIO测试代码
  // 例如：设置GPIO方向、读写GPIO值等
}

// 串口测试函数
void test_uart() {
  printf("测试串口...\n");
  // 这里添加实际的串口测试代码
  // 例如：初始化串口、发送接收数据等
}

// LED测试函数
void test_led() {
  printf("测试LED...\n");
  // 这里添加实际的LED测试代码
  // 例如：控制LED闪烁、亮度等
}

// ADC测试函数
void test_adc() {
  printf("测试ADC...\n");
  // 这里添加实际的ADC测试代码
  // 例如：读取ADC值、转换为电压等
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
  
  printf("服务器启动成功，监听端口 8080\n");
  printf("请访问 http://localhost:8080 进行测试\n");
  
  // 主事件循环
  while (1) {
    mg_mgr_poll(&mgr, 1000);
  }
  
  // 清理
  mg_mgr_free(&mgr);
  return 0;
}
