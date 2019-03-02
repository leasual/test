#ifndef HORZON_ENCRYPT_API_H
#define HORZON_ENCRYPT_API_H
/*
i2c_num： I2C 控制器序号（对应《 Hi3516A 专业型 HD IP Camera Soc 用户指南》中的 I2C 控制器 0、 1、 2）
device_addr：外围设备地址（ Hi3516A 只支持 7bit 设备地址）
reg_addr：读外围设备寄存器操作的开始地址
end_reg_addr：读外围设备寄存器操作的结束地址
reg_width：外围设备的寄存器位宽（ Hi3516A 支持 8/16bit）
data_width：外围设备的数据位宽（ Hi3516A 支持 8/16bit）
buf: success return data len of buf, buf is payload, else return -1;
*/
int read(int i2c_num, int device_addr, int reg_addr, int end_reg_addr, int reg_width, int data_width, /*out*/char* buf);

/*
referece to function read
value: to write into reg_addr of device
Successful return to 0 or failure
 */
int write(int i2c_num, int device_addr, int reg_addr, const char* value, int reg_width, int data_width);


#endif