#!/bin/bash 
 
 function show_help { 
     echo "用法: $0 <引脚> <方向> <值>" 
     echo "" 
     echo "参数说明:" 
     echo "  引脚  - 支持以下格式:" 
     echo "         全志: PB4、PC1、PH10 等" 
     echo "         瑞芯微: GPIO0_PA0、GPIO1_PB2 等" 
     echo "  方向  - 设置引脚方向: out (输出) 或 in (输入)" 
     echo "  值    - 设置引脚值: 1 (高电平) 或 0 (低电平)" 
     echo "" 
     echo "示例:" 
     echo "  全志芯片:" 
     echo "    $0 PB4 out 1   # 设置 PB4 为输出高电平" 
     echo "    $0 PC1 out 0   # 设置 PC1 为输出低电平" 
     echo "    $0 PH10 in 0   # 设置 PH10 为输入" 
     echo "" 
     echo "  瑞芯微芯片:" 
     echo "    $0 GPIO0_PA0 out 1   # 设置 GPIO0_PA0 为输出高电平" 
     echo "    $0 GPIO1_PB2 out 0   # 设置 GPIO1_PB2 为输出低电平" 
     echo "" 
     echo "引脚组对应关系:" 
     echo "  全志: A=0 B=1 C=2 D=3 E=4 F=5 G=6 H=7 I=8" 
     echo "        GPIO号 = 组号 * 32 + 引脚号" 
     echo "  瑞芯微: A=0 B=1 C=2 D=3" 
     echo "          GPIO号 = 控制器号*32 + 组号*8 + 引脚号" 
     echo "" 
     echo "注意事项:" 
     echo "  - 必须使用 sudo 运行" 
     echo "  - 引脚名称大小写均可" 
 } 
 
 function export_gpio { 
     local gpio_num=$1 
     if ! [[ "$gpio_num" =~ ^[0-9]+$ ]]; then 
         echo "错误: GPIO编号不是有效数字" 
         exit 1 
     fi 
     if [ ! -d "/sys/class/gpio/gpio$gpio_num" ]; then 
         echo "$gpio_num" > /sys/class/gpio/export 
         sleep 0.1 
     fi 
 } 
 
 function set_direction { 
     local gpio_num=$1 
     local direction=$2 
     if ! [[ "$gpio_num" =~ ^[0-9]+$ ]]; then 
         echo "错误: GPIO编号不是有效数字" 
         exit 1 
     fi 
     echo "$direction" > /sys/class/gpio/gpio$gpio_num/direction 2>/dev/null 
     if [ $? -ne 0 ]; then 
         echo "错误: 设置GPIO $gpio_num 方向失败" 
         exit 1 
     fi 
 } 
 
 function set_value { 
     local gpio_num=$1 
     local value=$2 
     if ! [[ "$gpio_num" =~ ^[0-9]+$ ]]; then 
         echo "错误: GPIO编号不是有效数字" 
         exit 1 
     fi 
     echo "$value" > /sys/class/gpio/gpio$gpio_num/value 2>/dev/null 
     if [ $? -ne 0 ]; then 
         echo "错误: 设置GPIO $gpio_num 值失败" 
         exit 1 
     fi 
 } 
 
 function parse_allwinner_pin { 
     local pin=$1 
 
     pin=$(echo "$pin" | tr -dc '[:alnum:]' | tr '[:lower:]' '[:upper:]') 
 
     if [[ "$pin" == P* ]]; then 
         pin=${pin:1} 
     fi 
 
     local group=${pin:0:1} 
     local pin_num=${pin:1} 
 
     local group_num 
     case "$group" in 
         A) group_num=0 ;; 
         B) group_num=1 ;; 
         C) group_num=2 ;; 
         D) group_num=3 ;; 
         E) group_num=4 ;; 
         F) group_num=5 ;; 
         G) group_num=6 ;; 
         H) group_num=7 ;; 
         I) group_num=8 ;; 
         *) 
             echo "错误: 无效的引脚组 '$group' (解析自 '$1')" >&2 
             return 1 
             ;; 
     esac 
 
     if ! [[ "$pin_num" =~ ^[0-9]+$ ]]; then 
         echo "错误: 引脚编号 '$pin_num' 不是数字" >&2 
         return 1 
     fi 
 
     echo "$((group_num * 32 + pin_num))" 
 } 
 
 function parse_rockchip_pin { 
     local pin=$1 
 
     pin=$(echo "$pin" | tr -dc '[:alnum:]_' | tr '[:lower:]' '[:upper:]') 
 
     if [[ "$pin" =~ ^GPIO([0-9]+)_([A-D])([0-9]+)$ ]]; then 
         local ctrl=${BASH_REMATCH[1]} 
         local group=${BASH_REMATCH[2]} 
         local pin_num=${BASH_REMATCH[3]} 
 
         local group_num 
         case "$group" in 
             A) group_num=0 ;; 
             B) group_num=1 ;; 
             C) group_num=2 ;; 
             D) group_num=3 ;; 
             *) 
                 echo "错误: 无效的引脚组 '$group' (解析自 '$1')" >&2 
                 return 1 
                 ;; 
         esac 
 
         echo "$((ctrl * 32 + group_num * 8 + pin_num))" 
     else 
         echo "错误: 瑞芯微引脚格式错误 '$1'，应为 GPIO0_PA0 格式" >&2 
         return 1 
     fi 
 } 
 
 function parse_pin { 
     local pin=$1 
     local pin_upper=$(echo "$pin" | tr '[:lower:]' '[:upper:]') 
 
     if [[ "$pin_upper" =~ ^GPIO[0-9]+_[A-D][0-9]+$ ]]; then 
         parse_rockchip_pin "$pin" 
     elif [[ "$pin_upper" =~ ^P[A-I][0-9]+$ ]]; then 
         parse_allwinner_pin "$pin" 
     else 
         echo "错误: 无法识别引脚格式 '$pin'" >&2 
         return 1 
     fi 
 } 
 
 if [ $# -eq 0 ] || [ "$1" = "-h" ] || [ "$1" = "--help" ]; then 
     show_help 
     exit 0 
 fi 
 
 if [ $# -ne 3 ]; then 
     echo "错误: 参数数量错误" 
     echo "请使用 -h 或 --help 查看帮助" 
     exit 1 
 fi 
 
 PIN_NAME="$1" 
 DIRECTION="$2" 
 VALUE="$3" 
 
 if [ "$DIRECTION" != "out" ] && [ "$DIRECTION" != "in" ]; then 
     echo "错误: 方向必须是out/in" 
     exit 1 
 fi 
 if [ "$VALUE" != "1" ] && [ "$VALUE" != "0" ]; then 
     echo "错误: 值必须是1/0" 
     exit 1 
 fi 
 if [ "$(id -u)" -ne 0 ]; then 
     echo "错误: 请用sudo运行" 
     exit 1 
 fi 
 
 echo "正在解析引脚: '$PIN_NAME'" 
 GPIO_NUM=$(parse_pin "$PIN_NAME") 
 
 if [ $? -ne 0 ] || [ -z "$GPIO_NUM" ] || ! [[ "$GPIO_NUM" =~ ^[0-9]+$ ]]; then 
     echo "错误: 引脚 '$PIN_NAME' 解析失败" 
     exit 1 
 fi 
 echo "引脚 '$PIN_NAME' -> GPIO编号: $GPIO_NUM" 
 
 echo "正在控制 GPIO$GPIO_NUM..." 
 export_gpio "$GPIO_NUM" 
 set_direction "$GPIO_NUM" "$DIRECTION" 
 set_value "$GPIO_NUM" "$VALUE" 
 echo "成功: GPIO$GPIO_NUM (${PIN_NAME}) 设置为 $DIRECTION, 值=$VALUE"
