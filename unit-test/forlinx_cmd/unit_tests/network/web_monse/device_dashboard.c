#include "mongoose.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// 设备状态数据结构
struct device_status {
    float cpu_usage;
    float memory_usage;
    float temperature;
    int ai_inference_count;
    char last_detection[256];
    time_t uptime;
};

// 全局设备状态
static struct device_status g_device = {0};

// 检查方法类型
int is_method(struct mg_http_message *hm, const char *method) {
    return (0 == mg_strcasecmp(hm->method, mg_str(method)));
}

// 模拟获取设备状态
void update_device_status(struct device_status *status) {
    status->cpu_usage = 45.7f + ((rand() % 100) / 10.0f);
    status->memory_usage = 62.3f + ((rand() % 100) / 10.0f);
    status->temperature = 42.5f + ((rand() % 100) / 10.0f);
    status->ai_inference_count++;
    
    const char *objects[] = {"人脸", "车辆", "行人", "动物"};
    snprintf(status->last_detection, sizeof(status->last_detection),
             "检测到%d个%s", rand() % 5 + 1, objects[rand() % 4]);
}

// HTTP 请求处理函数
void fn(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        
        // 设备状态 API
        if (mg_match(hm->uri, mg_str("/api/device/status"), NULL)) {
            update_device_status(&g_device);
            mg_http_reply(c, 200, 
                "Content-Type: application/json\r\n"
                "Access-Control-Allow-Origin: *\r\n",
                "{\"cpu_usage\":%.1f,\"memory_usage\":%.1f,\"temperature\":%.1f,"
                "\"ai_count\":%d,\"last_detection\":\"%s\",\"uptime\":%ld}",
                g_device.cpu_usage, g_device.memory_usage, g_device.temperature,
                g_device.ai_inference_count, g_device.last_detection, 
                time(NULL) - g_device.uptime);
        }
        // AI 控制 API
        else if (mg_match(hm->uri, mg_str("/api/ai/control"), NULL)) {
            if (is_method(hm, "POST")) {
                mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                    "{\"status\":\"success\",\"message\":\"AI控制命令已执行\"}");
            } else {
                mg_http_reply(c, 405, "Content-Type: application/json\r\n",
                    "{\"error\":\"方法不允许\"}");
            }
        }
        // 配置更新 API
        else if (mg_match(hm->uri, mg_str("/api/config/update"), NULL)) {
            if (is_method(hm, "POST")) {
                // 解析配置数据
                char confidence[16] = {0}, frequency[16] = {0};
                mg_http_get_var(&hm->body, "confidence", confidence, sizeof(confidence));
                mg_http_get_var(&hm->body, "frequency", frequency, sizeof(frequency));
                
                mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                    "{\"status\":\"success\",\"confidence\":\"%s\",\"frequency\":\"%s\"}",
                    confidence, frequency);
            }
        }
        // 系统信息 API
        else if (mg_match(hm->uri, mg_str("/api/system/info"), NULL)) {
            mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                "{\"device\":\"RV1126\",\"version\":\"1.0.0\"," 
                "\"sdk\":\"Rock-X 2.0\",\"model\":\"YOLOv5s\"}");
        }
        // 默认：静态文件服务
        else {
            struct mg_http_serve_opts opts = {.root_dir = "./dashboard"};
            mg_http_serve_dir(c, hm, &opts);
        }
    }
}

int main() {
    struct mg_mgr mgr;
    
    // 初始化设备状态
    g_device.uptime = time(NULL);
    srand(time(NULL)); // 初始化随机数种子
    
    mg_mgr_init(&mgr);
    printf("启动 RV1126 设备仪表盘服务...\n");
    
    // 监听 38080 端口
    if (mg_http_listen(&mgr, "http://0.0.0.0:38080", fn, NULL) == NULL) {
        printf("❌ 启动服务失败，端口可能被占用\n");
        return 1;
    }
    
    printf("服务已启动: http://0.0.0.0:38080\n");
    printf("静态文件目录: ./dashboard\n");
    printf("可用API接口:\n");
    printf("   GET  /api/device/status  - 设备状态\n");
    printf("   POST /api/ai/control     - AI控制\n");
    printf("   POST /api/config/update  - 配置更新\n");
    printf("   GET  /api/system/info    - 系统信息\n");
    
    // 事件循环
    while (1) {
        mg_mgr_poll(&mgr, 1000);
    }
    
    mg_mgr_free(&mgr);
    return 0;
}