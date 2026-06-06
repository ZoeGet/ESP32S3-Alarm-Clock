## 1. 全局变量添加

- [ ] 1.1 在main.cpp文件顶部（全局作用域）添加`unsigned long nextUpdateTime = 0;`
- [ ] 1.2 确认变量类型为`unsigned long`，初始值为0

## 2. setup()函数修改

- [ ] 2.1 在setup()函数末尾（所有初始化完成后）添加`nextUpdateTime = millis() + 1000;`
- [ ] 2.2 确认初始化顺序：RTC初始化 → LED初始化 → 设置nextUpdateTime

## 3. loop()函数重构

- [ ] 3.1 移除原有的`delay(1000);`语句
- [ ] 3.2 在loop()函数末尾添加动态延迟逻辑：
  - 计算等待时间：`unsigned long waitTime = nextUpdateTime - millis();`
  - 条件延迟：`if (waitTime > 0) delay(waitTime);`
  - 更新下次刷新时间：`nextUpdateTime += 1000;`
- [ ] 3.3 保持LED更新和显示刷新代码不变

## 4. 代码验证

- [ ] 4.1 编译代码，检查无错误
- [ ] 4.2 确认无语法错误（括号匹配、分号等）
- [ ] 4.3 确认变量作用域正确（nextUpdateTime在全局作用域）

## 5. 功能测试

- [ ] 5.1 上传代码到ESP32S3
- [ ] 5.2 观察1分钟，确认秒数变化准确（60秒）
- [ ] 5.3 观察5分钟，确认无累积误差
- [ ] 5.4 确认LED闪烁正常（不受修改影响）
- [ ] 5.5 确认OLED显示正常（时间格式正确）

## 6. 精度验证（可选）

- [ ] 6.1 添加调试输出显示实际延迟时间（Serial.println(waitTime)）
- [ ] 6.2 观察waitTime值，确认在980-1000ms范围内
- [ ] 6.3 验证millis()溢出处理（可模拟或长期观察）
