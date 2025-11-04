#include "serial_custom.h"

serial_custom::serial_custom()
{

}

//===========================================================================================
bool serial_custom::serialPort_open(QSerialPort *serialPort, QString Name_port)
{
    if(flag_connecttion == false)
    {
        serialPort->setPortName(Name_port);
        serialPort->setBaudRate(230400);
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setParity(QSerialPort::NoParity);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setFlowControl(QSerialPort::NoFlowControl);
        if(serialPort->open(QIODevice::ReadWrite))
        {
            qDebug()<<"PORT IS OPEN";
            flag_connecttion = true;
            return true;
        }
        else
        {
            qDebug()<<"PORT ERROR";
            return false;
        }
    }
    else
    {
        serialPort->close();
        flag_connecttion = false;
    }
    return true;
}
//============================================================================================
bool serial_custom::serial_write_data( QSerialPort *serialPort, QByteArray &DataSend)
{
    serialPort->write(DataSend);
    return true;
}
//============================================================================================
bool serial_custom::serial_send_command_Firmware(QSerialPort *serialPort, QByteArray &DataSend, quint32 Timeout_setting)
{
    // 28-03-2024 them state COM tranh viec dang gui goi tin nay lai gui them goi tin khac
    // da chay binh thuong 05-11-2024
    if(u8state == COM_IDLE)
    {
        // append CRC to message
        uint16_t u16crc = calcCRC( DataSend, LENGTH_COMMAND_SEND -2);
        DataSend[ LENGTH_COMMAND_SEND - 2 ] = u16crc >> 8;
        DataSend[ LENGTH_COMMAND_SEND - 1 ] = u16crc & 0x00ff;
       // qDebug()<<"Check CRC16"<< u16crc;
//        QByteArray ba_as_hex_string = DataSend.toHex();
//        qDebug()<<"Byte Low"<< DataSend[ 318];
//        qDebug()<<"Check High"<< DataSend[ 319];
        header_commmand = DataSend[0];
        flag_waitResponse_from_FW = false;
        u8_error_send_command = false;
        u32_Counter_check_waitting_Command_FW = 0;
        // SEND DATA
        u8state = COM_WAITING;
        serialPort->write(DataSend);
        while (flag_waitResponse_from_FW == false)
        {
            u32_Counter_check_waitting_Command_FW = u32_Counter_check_waitting_Command_FW + 5;
            if(u32_Counter_check_waitting_Command_FW > Timeout_setting)
            {
                u8_error_send_command = true;
                u8state = COM_IDLE;
                break;
            }
            else
            {
                u8_error_send_command = false;
                u8state = COM_IDLE;
            }
            delay_serial.delay_ms(5);
        }
        flag_waitResponse_from_FW = false;
        return !u8_error_send_command; // return false 0 if error // return true 1 if don't have error
    }
    else
    {
        qDebug()<<"COM BUSY - not SEND COMMAND";
        u8_error_send_command = false;
        u8state = COM_IDLE;
        return !u8_error_send_command; // return false 0 if error // return true 1 if don't have error
    }
}
/*****
 *  serial_wait_end_step
 *
 */
bool serial_custom:: serial_wait_end_step()
{
    flag_next_step_oligo = false;
    header_commmand = CMD_FIRMWARE_END_OLIGO_STEP;
    while (flag_waitResponse_from_FW == false)
    {
        delay_serial.delay_ms(50);
    }
    return true;
}
/**
 * @brief
 * This method calculates CRC
 *
 * @return uint16_t calculated CRC value for the message
 * @ingroup buffer
 */
uint16_t serial_custom::calcCRC(QByteArray &au8Buffer, uint16_t u16length)
{
    unsigned int temp, temp2, flag;
    temp = 0xFFFF;
    for (uint16_t i = 0; i < u16length; i++)
    {
        temp = temp ^ au8Buffer[i];
        for (uint16_t j = 1; j <= 8; j++)
        {
            flag = temp & 0x0001;
            temp >>=1;
            if (flag)
                temp ^= 0xA001;
        }
    }
    // Reverse byte order.
    temp2 = temp >> 8;
    temp = (temp << 8) | temp2;
    temp &= 0xFFFF;
    // the returned value is already swapped
    // crcLo byte is first & crcHi byte is last
    return temp;
}
