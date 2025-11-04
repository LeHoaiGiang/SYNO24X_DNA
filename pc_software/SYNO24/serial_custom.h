#ifndef SERIAL_CUSTOM_H
#define SERIAL_CUSTOM_H
#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include "delay.h"
#include "macro.h"
#define MINIMUX_DELAY_WAIT 20000
enum COM_STATES
{
    COM_IDLE                     = 0,
    COM_WAITING                  = 1
};
class serial_custom
{
public:
    serial_custom();
    delay delay_serial;
    bool flag_connecttion;
    bool flag_next_step_oligo;
    bool flag_process_command;
    bool flag_waitResponse_from_FW;
    quint8 header_commmand;
    QString currentPort;
    qint64 byteCount;
    bool isDataReceived;
    quint8 u8_error_send_command;
    quint32 u32_Counter_check_waitting_Command_FW;
    quint16 u16_length_command_fw_rx;
    bool serialPort_open(QSerialPort *serialPort, QString Name_port);
    bool serial_write_data( QSerialPort *serialPort, QByteArray &DataSend);
    bool serial_send_command_Firmware(QSerialPort *serialPort, QByteArray &DataSend, quint32 Timeout_setting);
    bool serial_send_start_oligo(QSerialPort *serialPort, QByteArray &DataSend);// function waitting for end one oligo
    bool serial_wait_end_step();
    uint16_t calcCRC(QByteArray &au8Buffer, uint16_t u16length);
private:
    uint8_t u8state;
};

#endif // SERIAL_CUSTOM_H
