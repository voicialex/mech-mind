# GitHub 推送权限问题解决指导

## 🔍 **当前问题**

您尝试推送到 [voicialex/mech-mind](https://github.com/voicialex/mech-mind) 仓库时遇到权限错误：
```
remote: Permission to voicialex/mech-mind.git denied to icelion588.
```

## 🛠️ **解决方案选择**

### **方案一：Fork 仓库（推荐）**

如果您要向他人的仓库贡献代码：

1. **访问原仓库**：https://github.com/voicialex/mech-mind
2. **点击 Fork 按钮**，将仓库Fork到您的账户（icelion588）
3. **更新本地远程配置**：
   ```bash
   git remote set-url origin https://github.com/icelion588/mech-mind.git
   ```
4. **推送到您的Fork**：
   ```bash
   git push origin branch_01
   ```
5. **创建 Pull Request**：在GitHub上从您的Fork向原仓库发起PR

### **方案二：请求权限**

如果您需要直接推送权限：

1. 联系仓库所有者 `voicialex`
2. 请求将您（`icelion588`）添加为协作者
3. 获得权限后可直接推送

### **方案三：确认仓库所有权**

如果这是您自己的仓库，但使用了错误的用户名：

1. 确认您当前的GitHub用户名
2. 检查本地Git配置：
   ```bash
   git config --global user.name
   git config --global user.email
   ```
3. 如果需要，更新配置为正确的用户信息

## 🚀 **推荐执行步骤（方案一）**

### 1. Fork 仓库
- 在浏览器中打开：https://github.com/voicialex/mech-mind
- 点击右上角的 "Fork" 按钮
- 选择您的账户（icelion588）作为目标

### 2. 更新本地配置
```bash
# 更新origin指向您的fork
git remote set-url origin https://github.com/icelion588/mech-mind.git

# 验证配置
git remote -v
```

### 3. 推送分支
```bash
# 推送branch_01到您的fork
git push origin branch_01

# 如果是首次推送，可能需要设置上游
git push --set-upstream origin branch_01
```

### 4. 创建 Pull Request
1. 推送成功后，GitHub会显示创建PR的提示
2. 或者手动访问您的fork仓库
3. 点击 "New pull request"
4. 选择从您的 `branch_01` 到 `voicialex/mech-mind` 的 `master` 分支
5. 填写PR标题和描述
6. 提交Pull Request

## 📋 **Pull Request 建议内容**

**标题**：`添加实时显示功能并修复窗口无响应问题`

**描述**：
```markdown
## 🚀 功能更新

### ✨ 新增功能
- 实时显示2D彩色图像和深度图
- 支持ESC键退出和窗口关闭响应
- 异步文件保存系统
- 性能监控和FPS显示

### 🐛 问题修复
- 解决窗口无响应问题
- 优化文件保存路径（修复双斜杠问题）
- 改进窗口事件处理响应性

### 🔧 技术改进
- 异步任务管理，避免阻塞显示更新
- 优化OpenCV窗口事件处理
- 添加性能监控和调试功能
- 线程安全的显示更新机制

### 📁 文件更改
- `samples/cpp/area_scan_3d_camera/perception_app/`
  - 新增实时显示功能
  - 修复保存路径问题
  - 添加性能优化

### 🧪 测试
- [x] 编译测试通过
- [x] 实时显示功能正常
- [x] 文件保存功能正常
- [x] 窗口响应性改善
```

## 💡 **下一步**

1. 选择上述方案之一
2. 如选择Fork方案，请先在GitHub网站上Fork仓库
3. 然后告知我，我将帮您完成本地配置和推送

需要我帮助您执行哪个方案？ 