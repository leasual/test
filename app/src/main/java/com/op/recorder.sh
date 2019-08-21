#!/bin/bash
#点击文件名输入框
adb shell input tap 377 190
#文件名输入
adb shell input text '$1'

#点击数量输入框
adb shell input tap 377 293
#数量输入
adb shell input text '$2'

#点击开始
adb shell input tap 377 90

#点击文件名输入框
adb shell input tap 377 190
for varible1 in {1..10}
#删除上次输入的文本
do
     adb shell input keyevent 67
done

#点击数量输入框
adb shell input tap 377 293
for varible2 in {1..5}
#删除上次输入的文本
do
     adb shell input keyevent 67
done