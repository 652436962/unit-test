// 显示加载状态
function showLoading() {
    console.log('测试中...');
}

// 显示测试结果详情
function showTestDetails(data) {
    let details = '';
    for (const [key, value] of Object.entries(data)) {
        details += `<strong>${key}:</strong> ${value}<br>`;
    }
    return details;
}

// AJAX请求函数
function testPeripheral(endpoint, buttonText, params = {}) {
    showLoading();
    
    let url = `http://localhost:8080/api/test/${endpoint}`;
    
    // 如果有参数，添加到URL中
    if (Object.keys(params).length > 0) {
        const queryParams = new URLSearchParams();
        for (const [key, value] of Object.entries(params)) {
            queryParams.append(key, value);
        }
        url += `?${queryParams.toString()}`;
    }
    
    fetch(url)
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            return response.json();
        })
        .then(data => {
            if (data.status === 200) {
                console.log(`${buttonText}测试完成`, data.data);
                
                // 如果是UART测试，更新接收数据文本框
                if (endpoint === 'uart' && data.data.receive_data) {
                    document.getElementById('uart-receive-data').value = data.data.receive_data;
                }
            } else {
                console.error('测试失败:', data.message);
            }
        })
        .catch(error => {
            console.error('请求错误:', error.message);
        });
}

// 显示指定的测试设置部分，隐藏其他部分
function showTestSettings(settingId) {
    // 隐藏所有设置部分
    const settingSections = document.querySelectorAll('.setting-section');
    settingSections.forEach(section => {
        section.style.display = 'none';
    });
    
    // 显示指定的设置部分
    const selectedSection = document.getElementById(settingId);
    if (selectedSection) {
        selectedSection.style.display = 'block';
        
        // 如果是UART设置，加载实际可用的串口端口
        if (settingId === 'uart-settings') {
            loadAvailableSerialPorts();
        }
    }
}

// 加载实际可用的串口端口
function loadAvailableSerialPorts() {
    console.log('加载可用串口端口...');
    
    fetch('http://localhost:8080/api/test/uart/ports')
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            return response.json();
        })
        .then(data => {
            if (data.status === 200 && data.data && data.data.ports) {
                const portSelect = document.getElementById('uart-port');
                // 清空现有的选项
                portSelect.innerHTML = '';
                
                // 添加实际可用的端口
                data.data.ports.forEach(port => {
                    const option = document.createElement('option');
                    option.value = port;
                    option.textContent = port;
                    portSelect.appendChild(option);
                });
                
                console.log('可用串口端口加载完成:', data.data.ports);
            }
        })
        .catch(error => {
            console.error('加载串口端口失败:', error.message);
        });
}

// 为测试图标添加点击事件监听器
document.addEventListener('DOMContentLoaded', function() {
    // GPIO方向选择事件
    document.getElementById('gpio-direction').addEventListener('change', function() {
        const valueSelect = document.getElementById('gpio-value');
        if (this.value === 'out') {
            valueSelect.disabled = false;
        } else {
            valueSelect.disabled = true;
        }
    });
    
    // GPIO测试
    document.getElementById('test-gpio').addEventListener('click', function() {
        showTestSettings('gpio-settings');
        const pin = document.getElementById('gpio-pin').value;
        const direction = document.getElementById('gpio-direction').value;
        const value = document.getElementById('gpio-value').value;
        console.log('GPIO测试参数:', { pin, direction, value });
        testPeripheral('gpio', 'GPIO', { pin, direction, value });
    });
    
    // GPIO执行按钮
    document.getElementById('gpio-execute').addEventListener('click', function() {
        const pin = document.getElementById('gpio-pin').value;
        const direction = document.getElementById('gpio-direction').value;
        const value = document.getElementById('gpio-value').value;
        console.log('GPIO执行参数:', { pin, direction, value });
        testPeripheral('gpio', 'GPIO', { pin, direction, value });
    });
    
    // 串口测试
    document.getElementById('test-uart').addEventListener('click', function() {
        showTestSettings('uart-settings');
        const port = document.getElementById('uart-port').value;
        const baudrate = document.getElementById('uart-baudrate').value;
        const data_bits = document.getElementById('uart-data-bits').value;
        const parity = document.getElementById('uart-parity').value;
        const stop_bits = document.getElementById('uart-stop-bits').value;
        const send_data = document.getElementById('uart-send-data').value;
        console.log('UART测试参数:', { port, baudrate, data_bits, parity, stop_bits, send_data });
        testPeripheral('uart', 'UART', { port, baudrate, data_bits, parity, stop_bits, send_data });
    });
    
    // UART发送按钮
    document.getElementById('uart-send-button').addEventListener('click', function() {
        const port = document.getElementById('uart-port').value;
        const baudrate = document.getElementById('uart-baudrate').value;
        const data_bits = document.getElementById('uart-data-bits').value;
        const parity = document.getElementById('uart-parity').value;
        const stop_bits = document.getElementById('uart-stop-bits').value;
        const send_data = document.getElementById('uart-send-data').value;
        console.log('UART发送参数:', { port, baudrate, data_bits, parity, stop_bits, send_data });
        testPeripheral('uart', 'UART', { port, baudrate, data_bits, parity, stop_bits, send_data });
    });
    
    // UART打开/关闭串口按钮
    let uartPortOpen = false;
    const uartOpenButton = document.getElementById('uart-open-button');
    
    uartOpenButton.addEventListener('click', function() {
        const port = document.getElementById('uart-port').value;
        const baudrate = document.getElementById('uart-baudrate').value;
        const data_bits = document.getElementById('uart-data-bits').value;
        const parity = document.getElementById('uart-parity').value;
        const stop_bits = document.getElementById('uart-stop-bits').value;
        
        if (!uartPortOpen) {
            // 打开串口
            console.log('UART打开参数:', { port, baudrate, data_bits, parity, stop_bits });
            testPeripheral('uart/open', '打开串口', { port, baudrate, data_bits, parity, stop_bits });
            uartPortOpen = true;
            uartOpenButton.textContent = '关闭串口';
        } else {
            // 关闭串口
            console.log('UART关闭参数:', { port });
            testPeripheral('uart/close', '关闭串口', { port });
            uartPortOpen = false;
            uartOpenButton.textContent = '打开串口';
        }
    });
    
    // LED测试
    document.getElementById('test-led').addEventListener('click', function() {
        showTestSettings('led-settings');
        const pattern = document.getElementById('led-pattern').value;
        const count = document.getElementById('led-count').value;
        console.log('LED测试参数:', { pattern, count });
        testPeripheral('led', 'LED', { pattern, count });
    });
    
    // LED执行按钮
    document.getElementById('led-execute').addEventListener('click', function() {
        const pattern = document.getElementById('led-pattern').value;
        const count = document.getElementById('led-count').value;
        console.log('LED执行参数:', { pattern, count });
        testPeripheral('led', 'LED', { pattern, count });
    });
    
    // RTC测试
    document.getElementById('test-rtc').addEventListener('click', function() {
        showTestSettings('rtc-settings');
    });
    
    // RTC读取时间按钮
    document.getElementById('rtc-read').addEventListener('click', function() {
        const time = document.getElementById('rtc-time').value;
        console.log('RTC读取参数:', { action: 'read', time });
        
        let url = `http://localhost:8080/api/test/rtc?action=read&time=${time}`;
        
        fetch(url)
            .then(response => {
                if (!response.ok) {
                    throw new Error(`HTTP error! status: ${response.status}`);
                }
                return response.json();
            })
            .then(data => {
                if (data.status === 200) {
                    console.log('RTC读取完成', data.data);
                    // 更新读取时间输入框
                    if (data.data.current_time) {
                        document.getElementById('rtc-read-time').value = data.data.current_time;
                    }
                } else {
                    console.error('测试失败:', data.message);
                }
            })
            .catch(error => {
                console.error('请求错误:', error.message);
            });
    });
    
    // RTC设置时间按钮
    document.getElementById('rtc-write').addEventListener('click', function() {
        const time = document.getElementById('rtc-time').value;
        console.log('RTC设置参数:', { action: 'write', time });
        testPeripheral('rtc', 'RTC', { action: 'write', time });
    });
    
    // 看门狗测试
    document.getElementById('test-watchdog').addEventListener('click', function() {
        showTestSettings('watchdog-settings');
        const timeout = document.getElementById('watchdog-timeout').value;
        console.log('看门狗测试参数:', { timeout });
        testPeripheral('watchdog', 'Watchdog', { timeout });
    });
    
    // 看门狗执行按钮
    document.getElementById('watchdog-execute').addEventListener('click', function() {
        const timeout = document.getElementById('watchdog-timeout').value;
        console.log('看门狗执行参数:', { timeout });
        testPeripheral('watchdog', 'Watchdog', { timeout });
    });
});
