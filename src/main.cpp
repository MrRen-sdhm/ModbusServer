//
// Created by sdhm on 7/5/19.
//

#include "modbusadapter.h"

int main(int argc, char*argv[]){
    auto *m_instance = new ModbusAdapter();
    m_instance->modbusConnectTCP("127.0.0.1", 1502);

    printf("\n[INFO] modbusReadData:\n");
    m_instance->modbusReadData(1, MODBUS_FC_READ_HOLDING_REGISTERS, 0, 3);

    printf("\n[INFO] modbusReadHoldReg:\n");
    auto data = (uint16_t *) malloc(125 * sizeof(uint16_t));
    memset(data, 0, 125 * sizeof(uint16_t));
    int noOfItems = 3;
    data = m_instance->modbusReadHoldReg(1, 0, noOfItems);
    for(int i = 0; i < noOfItems; ++i)
    {
        int data_ = data[i];
            cout << "data: " << data_ << endl;
    }

    printf("\n[INFO] modbusWriteData:\n");
    vector<int> value{112, 223, 334};
    m_instance->modbusWriteData(1, MODBUS_FC_WRITE_MULTIPLE_REGISTERS, 0, 3, value);
    return 0;
}

