// dashboard.js
class DeviceDashboard {
    constructor() {
        this.charts = {};
        this.dataHistory = {
            cpu: [],
            memory: [],
            temperature: [],
            timestamps: []
        };
        this.init();
    }

    init() {
        this.updateTime();
        this.initCharts();
        this.startDataPolling();
        this.setupEventListeners();
        
        // 每秒更新时间
        setInterval(() => this.updateTime(), 1000);
    }

    updateTime() {
        const now = new Date();
        document.getElementById('current-time').textContent =
            now.toLocaleString('zh-CN');
    }

    initCharts() {
        // 性能趋势图表
        const perfCtx = document.getElementById('performance-chart').getContext('2d');
        this.charts.performance = new Chart(perfCtx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [
                    {
                        label: 'CPU使用率 (%)',
                        data: [],
                        borderColor: '#3498db',
                        backgroundColor: 'rgba(52, 152, 219, 0.1)',
                        tension: 0.4,
                        fill: true
                    },
                    {
                        label: '内存使用率 (%)',
                        data: [],
                        borderColor: '#2ecc71',
                        backgroundColor: 'rgba(46, 204, 113, 0.1)',
                        tension: 0.4,
                        fill: true
                    }
                ]
            },
            options: {
                responsive: true,
                plugins: {
                    title: { display: true, text: '系统性能趋势' }
                },
                scales: {
                    y: { min: 0, max: 100 }
                }
            }
        });

        // AI检测统计图表
        const aiCtx = document.getElementById('ai-chart').getContext('2d');
        this.charts.ai = new Chart(aiCtx, {
            type: 'bar',
            data: {
                labels: ['人脸', '车辆', '行人', '动物', '其他'],
                datasets: [{
                    label: '检测数量',
                    data: [12, 19, 8, 5, 3],
                    backgroundColor: [
                        'rgba(255, 99, 132, 0.8)',
                        'rgba(54, 162, 235, 0.8)',
                        'rgba(255, 206, 86, 0.8)',
                        'rgba(75, 192, 192, 0.8)',
                        'rgba(153, 102, 255, 0.8)'
                    ]
                }]
            },
            options: {
                responsive: true,
                plugins: {
                    title: { display: true, text: 'AI检测统计' }
                }
            }
        });
    }

    async fetchDeviceStatus() {
        try {
            const response = await fetch('/api/device/status');
            const data = await response.json();
            this.updateDashboard(data);
            this.updateCharts(data);
        } catch (error) {
            console.error('获取设备状态失败:', error);
            this.showConnectionError();
        }
    }

    updateDashboard(data) {
        // 更新状态卡片
        document.getElementById('cpu-usage').textContent = `${data.cpu_usage}%`;
        document.getElementById('memory-usage').textContent = `${data.memory_usage}%`;
        document.getElementById('temperature').textContent = `${data.temperature}°C`;
        document.getElementById('ai-count').textContent = data.ai_count;
        document.getElementById('last-detection').textContent = data.last_detection;

        // 更新进度条
        document.getElementById('cpu-progress').style.width = `${data.cpu_usage}%`;
        document.getElementById('memory-progress').style.width = `${data.memory_usage}%`;

        // 更新运行时间
        const uptime = this.formatUptime(data.uptime);
        document.getElementById('uptime').textContent = uptime;

        // 更新温度计颜色
        this.updateTemperatureColor(data.temperature);
    }

    updateTemperatureColor(temp) {
        const gauge = document.getElementById('temp-gauge');
        if (temp < 50) {
            gauge.style.background = 'linear-gradient(90deg, #2ecc71, #27ae60)';
        } else if (temp < 70) {
            gauge.style.background = 'linear-gradient(90deg, #f39c12, #e67e22)';
        } else {
            gauge.style.background = 'linear-gradient(90deg, #e74c3c, #c0392b)';
        }
    }

    updateCharts(data) {
        const now = new Date().toLocaleTimeString('zh-CN', {
            hour12: false,
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit'
        });

        // 限制数据历史长度
        if (this.dataHistory.timestamps.length > 20) {
            this.dataHistory.timestamps.shift();
            this.dataHistory.cpu.shift();
            this.dataHistory.memory.shift();
            this.dataHistory.temperature.shift();
        }

        // 添加新数据
        this.dataHistory.timestamps.push(now);
        this.dataHistory.cpu.push(data.cpu_usage);
        this.dataHistory.memory.push(data.memory_usage);
        this.dataHistory.temperature.push(data.temperature);

        // 更新图表
        this.charts.performance.data.labels = this.dataHistory.timestamps;
        this.charts.performance.data.datasets[0].data = this.dataHistory.cpu;
        this.charts.performance.data.datasets[1].data = this.dataHistory.memory;
        this.charts.performance.update();

        // 添加数据流条目
        this.addDataStreamEntry(data);
    }

    addDataStreamEntry(data) {
        const stream = document.getElementById('data-stream');
        const entry = document.createElement('div');
        entry.className = 'stream-item';
        entry.textContent = `[${new Date().toLocaleTimeString()}] CPU: ${data.cpu_usage}% | 内存: ${data.memory_usage}% | 温度: ${data.temperature}°C | ${data.last_detection}`;
        
        stream.appendChild(entry);
        stream.scrollTop = stream.scrollHeight;

        // 限制流条目数量
        if (stream.children.length > 50) {
            stream.removeChild(stream.firstChild);
        }
    }

    formatUptime(seconds) {
        const days = Math.floor(seconds / 86400);
        const hours = Math.floor((seconds % 86400) / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        
        if (days > 0) {
            return `${days}天 ${hours}小时 ${minutes}分`;
        } else if (hours > 0) {
            return `${hours}小时 ${minutes}分`;
        } else {
            return `${minutes}分`;
        }
    }

    startDataPolling() {
        // 每2秒获取一次数据
        setInterval(() => this.fetchDeviceStatus(), 2000);
        this.fetchDeviceStatus(); // 立即执行一次
    }

    setupEventListeners() {
        // 置信度滑块
        const confidenceSlider = document.getElementById('confidence-slider');
        const confidenceValue = document.getElementById('confidence-value');
        
        confidenceSlider.addEventListener('input', (e) => {
            confidenceValue.textContent = e.target.value;
        });

        // 频率滑块
        const frequencySlider = document.getElementById('frequency-slider');
        const frequencyValue = document.getElementById('frequency-value');
        
        frequencySlider.addEventListener('input', (e) => {
            frequencyValue.textContent = e.target.value;
        });
    }

    showConnectionError() {
        document.getElementById('connection-status').textContent = '● 连接断开';
        document.getElementById('connection-status').className = 'status-disconnected';
    }
}

// 控制函数
async function startAI() {
    try {
        const response = await fetch('/api/ai/control', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body: 'command=start'
        });
        const result = await response.json();
        showNotification('AI推理已启动', 'success');
    } catch (error) {
        showNotification('启动失败: ' + error.message, 'error');
    }
}

async function stopAI() {
    try {
        const response = await fetch('/api/ai/control', {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
            body: 'command=stop'
        });
        const result = await response.json();
        showNotification('AI推理已停止', 'success');
    } catch (error) {
        showNotification('停止失败: ' + error.message, 'error');
    }
}

async function rebootDevice() {
    if (confirm('确定要重启设备吗？')) {
        showNotification('设备重启中...', 'warning');
        // 实际实现中调用重启API
    }
}

async function shutdownDevice() {
    if (confirm('确定要关闭设备吗？')) {
        showNotification('设备关闭中...', 'warning');
        // 实际实现中调用关机API
    }
}

async function saveConfig() {
    const confidence = document.getElementById('confidence-slider').value;
    const frequency = document.getElementById('frequency-slider').value;
    
    try {
        const response = await fetch('/api/config/update', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                confidence: parseFloat(confidence),
                frequency: parseInt(frequency)
            })
        });
        showNotification('配置已保存', 'success');
    } catch (error) {
        showNotification('保存失败: ' + error.message, 'error');
    }
}

function showNotification(message, type = 'info') {
    // 简单的通知实现
    const notification = document.createElement('div');
    notification.style.cssText = `
        position: fixed;
        transform: translateY(20px);
        right: 20px;
        padding: 1rem 1.5rem;
        border-radius: 8px;
        color: white;
        font-weight: bold;
        z-index: 1000;
        animation: slideIn 0.3s ease;
    `;
    
    const colors = {
        success: '#27ae60',
        error: '#e74c3c',
        warning: '#f39c12',
        info: '#3498db'
    };
    
    notification.style.background = colors[type] || colors.info;
    notification.textContent = message;
    
    document.body.appendChild(notification);
    
    setTimeout(() => {
        notification.style.animation = 'slideOut 0.3s ease';
        setTimeout(() => document.body.removeChild(notification), 300);
    }, 3000);
}

// 添加CSS动画
const style = document.createElement('style');
style.textContent = `
    @keyframes slideIn {
        from { transform: translateX(100%); opacity: 0; }
        to { transform: translateX(0); opacity: 1; }
    }
    @keyframes slideOut {
        from { transform: translateX(0); opacity: 1; }
        to { transform: translateX(100%); opacity: 0; }
    }
    .status-disconnected { color: #e74c3c !important; }
`;
document.head.appendChild(style);

// 初始化仪表盘
document.addEventListener('DOMContentLoaded', () => {
    new DeviceDashboard();
});