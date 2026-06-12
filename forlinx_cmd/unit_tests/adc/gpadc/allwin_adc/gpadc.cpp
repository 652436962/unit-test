#include <stdio.h>
#include <linux/input.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#define DEV_PATH_vol "/dev/input/event3"   /* sunxi-gpadc1，支持MSC事件 */
#define DEV_PATH_cur "/dev/input/event2"   /* sunxi-gpadc0，不支持MSC事件 */
const int key_exit = 102;

// 检查设备支持的事件类型
void check_device_capabilities(int fd, const char *channel_name)
{
    unsigned long evbit[EV_MAX / 8 + 1];
    memset(evbit, 0, sizeof(evbit));
    
    // 获取设备支持的事件类型
    if(ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), evbit) == 0)
    {
        printf("%s: 支持的事件类型：\n", channel_name);
        
        if(evbit[0] & (1 << EV_SYN))
            printf("  - EV_SYN (同步事件)\n");
        if(evbit[0] & (1 << EV_KEY))
            printf("  - EV_KEY (按键事件)\n");
        if(evbit[0] & (1 << EV_ABS))
            printf("  - EV_ABS (绝对坐标事件)\n");
        if(evbit[0] & (1 << EV_REL))
            printf("  - EV_REL (相对坐标事件)\n");
        if(evbit[0] & (1 << EV_MSC))
            printf("  - EV_MSC (杂项事件)\n");
        if(evbit[0] & (1 << EV_SW))
            printf("  - EV_SW (开关事件)\n");
        if(evbit[0] & (1 << EV_LED))
            printf("  - EV_LED (LED事件)\n");
    }
    else
    {
        printf("%s: 无法获取设备支持的事件类型\n", channel_name);
    }
}

unsigned int test_gpadc(const char * event_file, const char * channel_name)
{
    int fd;
    int i, ret;
    struct input_event data;
    fd_set read_fds;
    struct timeval timeout;
    
    // 检查文件权限
    struct stat st;
    if(stat(event_file, &st) == 0)
    {
        printf("%s: 文件权限: 0%o\n", channel_name, st.st_mode & 0777);
    }
    
    // 尝试以不同方式打开设备
    printf("%s: 尝试以阻塞方式打开设备...\n", channel_name);
    fd = open(event_file, O_RDONLY);
    if(fd <= 0)
    {
        printf("%s: 阻塞方式打开失败，尝试非阻塞方式...\n", channel_name);
        fd = open(event_file, O_RDONLY | O_NONBLOCK);
        if(fd <= 0)
        {
            printf("%s: 非阻塞方式打开也失败! 错误码: %d\n", channel_name, errno);
            perror("open");
            return -1;
        }
    }
    
    printf("%s: 设备打开成功，文件描述符: %d\n", channel_name, fd);
    
    // 检查设备支持的事件类型
    check_device_capabilities(fd, channel_name);
    
    // 先尝试立即读取一次，看看是否有缓冲数据
    printf("%s: 尝试立即读取一次...\n", channel_name);
    ret = read(fd, &data, sizeof(data));
    if(ret == sizeof(data))
    {
        printf("%s: 立即读取到数据：事件类型=0x%x, 代码=0x%x, 值=%d\n", 
               channel_name, data.type, data.code, data.value);
    }
    else if(ret < 0)
    {
        if(errno == EAGAIN)
        {
            printf("%s: 立即读取无数据（EAGAIN）\n", channel_name);
        }
        else
        {
            printf("%s: 立即读取错误: %d\n", channel_name, errno);
            perror("read");
        }
    }
    else
    {
        printf("%s: 立即读取不完整: %d 字节\n", channel_name, ret);
    }
    
    printf("%s: 开始轮询读取数据...\n", channel_name);
    
    // 尝试读取10次，每次等待2秒
    for(i = 0; i < 10; i++)
    {
        printf("%s: 第%d次轮询...\n", channel_name, i+1);
        
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);
        
        // 设置2秒超时
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;
        
        // 等待数据可读
        ret = select(fd + 1, &read_fds, NULL, NULL, &timeout);
        if(ret < 0)
        {
            printf("%s: select error! 错误码: %d\n", channel_name, errno);
            perror("select");
            break;
        }
        else if(ret == 0)
        {
            printf("%s: 第%d次轮询超时\n", channel_name, i+1);
            continue;
        }
        
        // 读取数据
        if(FD_ISSET(fd, &read_fds))
        {
            printf("%s: 检测到可读事件，尝试读取...\n", channel_name);
            
            // 尝试读取多个事件
            for(int j = 0; j < 5; j++)
            {
                ret = read(fd, &data, sizeof(data));
                if(ret == sizeof(data))
                {
                    // 打印所有事件类型，帮助调试
                    printf("%s: 读取到事件%d：类型=0x%x, 代码=0x%x, 值=%d\n", 
                           channel_name, j+1, data.type, data.code, data.value);
                    
                    // 处理不同类型的事件
                    if(data.type == EV_MSC)
                    {
                        printf("%s:   → MSC事件：ADC数据=%d, 电压=%d mv\n", 
                               channel_name, data.value, 18000 * data.value / 4096);
                    }
                    else if(data.type == EV_ABS)
                    {
                        printf("%s:   → ABS事件：数据=%d, 电压=%d mv\n", 
                               channel_name, data.value, 18000 * data.value / 4096);
                    }
                    else if(data.type == EV_SYN)
                    {
                        printf("%s:   → SYN事件：同步完成\n", channel_name);
                    }
                }
                else if(ret < 0)
                {
                    if(errno == EAGAIN)
                    {
                        printf("%s:   → 无更多数据（EAGAIN）\n", channel_name);
                        break;
                    }
                    else
                    {
                        printf("%s:   → 读取错误: %d\n", channel_name, errno);
                        perror("read");
                        break;
                    }
                }
                else
                {
                    printf("%s:   → 读取不完整: %d 字节\n", channel_name, ret);
                    break;
                }
            }
        }
    }

    close(fd);
    printf("%s: 读取结束\n", channel_name);
    return 0;
}

int main(int argc,const char *argv[])
{
    // 先测试电压ADC通道，后测试电流ADC通道
    printf("开始测试ADC通道...\n\n");
    
    // 测试电压通道 (sunxi-gpadc1, event3)
    printf("=== 测试电压ADC通道 ===\n");
    test_gpadc(DEV_PATH_vol, "测量电压");
    
    usleep(500000); /* 延时500ms，等待硬件稳定 */
    
    // 测试电流通道 (sunxi-gpadc0, event2)
    printf("\n=== 测试电流ADC通道 ===\n");
    test_gpadc(DEV_PATH_cur, "测量电流");
    
    printf("\n所有ADC通道测试完毕\n");
    return 0;
}