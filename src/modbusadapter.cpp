//
// Created by sdhm on 7/5/19.
//

#include "modbusadapter.h"
#include <errno.h>

ModbusAdapter::ModbusAdapter() : m_modbus(nullptr)
{
    m_connected = false;
    m_packets = 0;
    m_errors = 0;
    //setup memory for data
    dest = (uint8_t *) malloc(2000 * sizeof(uint8_t));
    memset(dest, 0, 2000 * sizeof(uint8_t));
    dest16 = (uint16_t *) malloc(125 * sizeof(uint16_t));
    memset(dest16, 0, 125 * sizeof(uint16_t));
}

ModbusAdapter::~ModbusAdapter()
{
    free(dest);
    free(dest16);
}

void ModbusAdapter::modbusConnectRTU(string port, int baud, char parity, int dataBits, int stopBits, int RTS, int timeOut)
{
    //Modbus RTU connect
    string line;
    modbusDisConnect();

    cout<<  "Modbus Connect RTU\n";

    m_modbus = modbus_new_rtu(port.c_str(), baud, parity, dataBits, stopBits, RTS);  //NOTE: RTS是自定义参数
    line = "Connecting to Serial Port [" + port + "]...\n";
    cout <<  line;

    //Debug messages from libmodbus
#ifdef LIB_MODBUS_DEBUG_OUTPUT
    modbus_set_debug(m_modbus, 1);
#endif

    if(m_modbus == nullptr){
        cerr<<  "Connection failed. Unable to create the libmodbus context\n";
        return;
    }
    else if(m_modbus && modbus_set_slave(m_modbus, m_slave) == -1){
        modbus_free(m_modbus);
        cerr<<  "Connection failed. Invalid slave ID\n";
        return;
    }
    else if(m_modbus && modbus_connect(m_modbus) == -1) {
        modbus_free(m_modbus);
        cerr<<  "Connection failed. Could not connect to serial port\n";
        m_connected = false;
        line += "Failed\n";
    }
    else {
        //error recovery mode
        modbus_set_error_recovery(m_modbus, MODBUS_ERROR_RECOVERY_PROTOCOL);
        //response_timeout;
        modbus_set_response_timeout(m_modbus, timeOut, 0);
        m_connected = true;
        line += "OK\n";
        cout << line;
    }
}

void ModbusAdapter::modbusConnectTCP(const string ip, int port, int timeOut)
{
    //Modbus TCP connect
    string line;
    modbusDisConnect();

    cout << "Modbus Connect TCP...\n";

    line = "Connecting to IP : " + ip + ":" + to_string(port);

    if (ip == ""){
        cerr <<  "Connection failed. Blank IP Address\n";
        return;
    }
    else {
        m_modbus = modbus_new_tcp(ip.c_str(), port);
        cout <<  "Connecting to IP : " << ip << ":" << port << endl;
    }

    //Debug messages from libmodbus
#ifdef LIB_MODBUS_DEBUG_OUTPUT
    modbus_set_debug(m_modbus, 1);
#endif

    if(m_modbus == NULL){
        cerr<<  "Connection failed. Unable to create the libmodbus context\n";
        return;
    }
    else if(m_modbus && modbus_connect(m_modbus) == -1) {
        modbus_free(m_modbus);
        cerr << "Connection to IP : " << ip << ":" << port << " failed. Could not connect to TCP port\n";
        m_connected = false;
        line += " Failed\n";
    }
    else {
        //error recovery mode
        modbus_set_error_recovery(m_modbus, MODBUS_ERROR_RECOVERY_PROTOCOL);
        //response_timeout;
        modbus_set_response_timeout(m_modbus, timeOut, 0);
        m_connected = true;
        line += " OK\n";
        cout << line;
    }
}

void ModbusAdapter::modbusDisConnect()
{
    //Modbus disconnect

    cout <<  "Modbus disconnected...\n";

    if(m_modbus != nullptr) {
        if (m_connected){
            modbus_close(m_modbus);
            modbus_free(m_modbus);
        }
        m_modbus = nullptr;
    }

    m_connected = false;
}

bool ModbusAdapter::isConnected()
{
    //Modbus is connected
    return m_connected;
}

void ModbusAdapter::modbusReadData(int slave, int functionCode, int startAddress, int noOfItems)
{
    cout <<  "Modbus Read Data\n";

    if(m_modbus == nullptr) return;

    int ret = -1; //return value from read functions
    bool is16Bit = false;

    modbus_set_slave(m_modbus, slave);
    //request data from modbus
    switch(functionCode)
    {
        case MODBUS_FC_READ_COILS:
            ret = modbus_read_bits(m_modbus, startAddress, noOfItems, dest);
            break;

        case MODBUS_FC_READ_DISCRETE_INPUTS:
            ret = modbus_read_input_bits(m_modbus, startAddress, noOfItems, dest);
            break;

        case MODBUS_FC_READ_HOLDING_REGISTERS:
            ret = modbus_read_registers(m_modbus, startAddress, noOfItems, dest16);
            is16Bit = true;
            break;

        case MODBUS_FC_READ_INPUT_REGISTERS:
            ret = modbus_read_input_registers(m_modbus, startAddress, noOfItems, dest16);
            is16Bit = true;
            break;

        default:
            break;
    }

    cout <<  "Modbus Read Data return value = " << ret << ", errno = " << errno;

    //update data model
    if(ret == noOfItems)
    {
        cout << endl;
        for(int i = 0; i < noOfItems; ++i)
        {
            int data = is16Bit ? dest16[i] : dest[i];
            cout << "data: " << data << endl;
        }
    }
    else
    {
        m_errors += 1;

        string line;
        if(ret < 0) {
            line = string("Error : ") +  libmodbus_strerror(errno);
            cerr <<  "Read Data failed. " << line;
        }
        else {
            line = string("Number of registers returned does not match number of registers requested!. Error : ") + libmodbus_strerror(errno);
            cerr <<  "Read Data failed. " << line;
        }
        modbus_flush(m_modbus); //flush data
    }
}

uint16_t* ModbusAdapter::modbusReadHoldReg(int slave, int startAddress, int noOfItems)
{
    cout <<  "Modbus Read Data\n";

    if(m_modbus == nullptr) return nullptr;

    int ret = -1; //return value from read functions

    modbus_set_slave(m_modbus, slave);
    //request data from modbus

    ret = modbus_read_registers(m_modbus, startAddress, noOfItems, dest16);


    cout <<  "Modbus Read Data return value = " << ret << ", errno = " << errno;

    //update data model
    if(ret == noOfItems)
    {
        cout << endl;
        for(int i = 0; i < noOfItems; ++i)
        {
            int data = dest16[i];
//            cout << "data: " << data << endl;
        }

        return dest16;
    }
    else
    {
        m_errors += 1;

        string line;
        if(ret < 0) {
            line = string("Error : ") +  libmodbus_strerror(errno);
            cerr <<  "Read Data failed. " << line;
        }
        else {
            line = string("Number of registers returned does not match number of registers requested!. Error : ") + libmodbus_strerror(errno);
            cerr <<  "Read Data failed. " << line;
        }
        modbus_flush(m_modbus); //flush data
    }
}

void ModbusAdapter::modbusWriteData(int slave, int functionCode, int startAddress, int noOfItems, vector<int> value)
{

    cout <<  "Modbus Write Data\n";

    if(m_modbus == nullptr) return;

    int ret = -1; //return value from functions

    modbus_set_slave(m_modbus, slave);
    //request data from modbus
    switch(functionCode)
    {
        case MODBUS_FC_WRITE_SINGLE_COIL:
                    ret = modbus_write_bit(m_modbus, startAddress, value[0]);
            noOfItems = 1;
            break;

        case MODBUS_FC_WRITE_SINGLE_REGISTER:
                    ret = modbus_write_register( m_modbus, startAddress, value[0]);
            noOfItems = 1;
            break;

        case MODBUS_FC_WRITE_MULTIPLE_COILS:
        {
            auto * data = new uint8_t[noOfItems];
            for(int i = 0; i < noOfItems; ++i)
            {
                            data[i] = value[i];
            }
            ret = modbus_write_bits(m_modbus, startAddress, noOfItems, data);
            delete[] data;
            break;
        }
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
        {
            auto * data = new uint16_t[noOfItems];
            for(int i = 0; i < noOfItems; ++i)
            {
                            data[i] = value[i];
            }
            ret = modbus_write_registers(m_modbus, startAddress, noOfItems, data);
            delete[] data;
            break;
        }

        default:
            break;
    }

    cout <<  "Modbus Write Data return value = " << ret << ", errno = " << errno << endl;

    // print debug messages
    if(ret == noOfItems)
    {
        //values written correctly
        printf("Value written correctly.\n");
    }
    else
    {
        m_errors += 1;

        string line;
        if(ret < 0) {
            line = string("Error : ") +  libmodbus_strerror(errno);
            cerr <<  "Write Data failed. " << line << endl;
        }
        else {
            line = string("Number of registers returned does not match number of registers requested!. Error : ") + libmodbus_strerror(errno);
            cerr <<  "Write Data failed. " << line << endl;
        }
        modbus_flush(m_modbus); //flush data
    }

}

string ModbusAdapter::libmodbus_strerror(int errnum)
{
    switch (errnum) {

        case EINVAL:
            return "Protocol context is NULL";
            break;

        case ETIMEDOUT:
            return "Timeout";
            break;

        case ECONNRESET:
            return "Connection reset";
            break;

        case ECONNREFUSED:
            return "Connection refused";
            break;

        case EPIPE:
            return "Socket error";
            break;

        default://Default
            return modbus_strerror(errno);

    }
}

void ModbusAdapter::setSlave(int slave)
{
    m_slave = slave;
}

void ModbusAdapter::setFunctionCode(int functionCode)
{
    m_functionCode = functionCode;
}

void ModbusAdapter::setStartAddr(int addr)
{
    m_startAddr = addr;
}

void ModbusAdapter::setNumOfRegs(int num)
{
    m_numOfRegs = num;
}

void ModbusAdapter::setScanRate(int scanRate)
{
    m_scanRate = scanRate;
}

int ModbusAdapter::packets()
{
    return m_packets;
}

int ModbusAdapter::errors()
{
    return m_errors;
}