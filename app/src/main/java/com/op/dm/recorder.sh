#!/system/bin/sh
#点击文件名输入框
adb shell input tap 50 300
sleep 1
#文件名输入
adb shell input text '$1'
sleep 1
#点击数量输入框
adb shell input tap 50 410
sleep 1
#数量输入
adb shell input text '$2'
sleep 1

#点击开始
adb shell input tap 377 90
sleep 1

点击文件名输入框
adb shell input tap 377 190
sleep 1
for varible1 in {1..10}
#删除上次输入的文本
do
     adb shell input keyevent 67
     sleep 1
done

#点击数量输入框
adb shell input tap 377 293
sleep 1
for varible2 in {1..5}
#删除上次输入的文本
do
     adb shell input keyevent 67
     sleep 1
done