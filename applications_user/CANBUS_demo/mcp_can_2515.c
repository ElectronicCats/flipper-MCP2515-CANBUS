#include "mcp_can_2515.h"
//----------------------------------------------------------------------------------
//  To DEBUG
//----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
//
//              THESE CODE WORK FOR READ AND SEND DATA BY SPI PROTOCOL TO THE MCP2515 DEVICE
//
//---------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//  To read a register
//-----------------------------------------------------------------------------------

static bool read_register(FuriHalSpiBusHandle* spi, uint8_t address, uint8_t* data) {
    bool ret = true;
    uint8_t instruction[] = {INSTRUCTION_READ, address};
    furi_hal_spi_acquire(spi);
    ret =
        (furi_hal_spi_bus_tx(spi, instruction, sizeof(instruction), TIMEOUT_SPI) &&
         furi_hal_spi_bus_rx(spi, data, sizeof(data), TIMEOUT_SPI));

    furi_hal_spi_release(spi);
    return ret;
}

//-----------------------------------------------------------------------------------
//  To write a register
//-----------------------------------------------------------------------------------

static bool set_register(FuriHalSpiBusHandle* spi, uint8_t address, uint8_t data) {
    bool ret = true;
    uint8_t instruction[] = {INSTRUCTION_WRITE, address, data};
    furi_hal_spi_acquire(spi);
    ret = furi_hal_spi_bus_tx(spi, instruction, sizeof(instruction), TIMEOUT_SPI);
    furi_hal_spi_release(spi);
    return ret;
}

//-----------------------------------------------------------------------------------
//  To write a register
//-----------------------------------------------------------------------------------

static bool set_registerS(FuriHalSpiBusHandle* spi, uint8_t address, uint8_t* data) {
    bool ret = true;
    uint8_t instruction[] = {INSTRUCTION_WRITE, address};
    uint8_t count = sizeof(data);

    for(int i = 0; i < count; i++) {
        instruction[i + 2] = data[i];
    }

    furi_hal_spi_acquire(spi);
    ret = furi_hal_spi_bus_tx(spi, instruction, sizeof(instruction), TIMEOUT_SPI);
    furi_hal_spi_release(spi);
    return ret;
}

//-----------------------------------------------------------------------------------
//  To modify a register
//-----------------------------------------------------------------------------------

static bool
    modify_register(FuriHalSpiBusHandle* spi, uint8_t address, uint8_t mask, uint8_t data) {
    uint8_t instruction[] = {INSTRUCTION_BITMOD, address, mask, data};
    bool ret = true;
    furi_hal_spi_acquire(spi);
    ret = furi_hal_spi_bus_tx(spi, instruction, sizeof(instruction), TIMEOUT_SPI);
    furi_hal_spi_release(spi);
    return ret;
}

//----------------------------------------------------------------------------------
//  To reset the MCP2515
//-----------------------------------------------------------------------------------

static bool mcp_reset(FuriHalSpiBusHandle* spi) {
    uint8_t buff[1] = {INSTRUCTION_RESET};
    bool ret = true;
    furi_hal_spi_acquire(spi);
    ret = furi_hal_spi_bus_tx(spi, buff, sizeof(buff), TIMEOUT_SPI);
    furi_hal_spi_release(spi);
    return ret;
}

//----------------------------------------------------------------------------------
//  To get the status
//-----------------------------------------------------------------------------------

bool mcp_get_status(FuriHalSpiBusHandle* spi, uint8_t* data) {
    uint8_t buff[1] = {INSTRUCTION_READ_STATUS};
    bool ret = true;
    furi_hal_spi_acquire(spi);
    ret =
        (furi_hal_spi_bus_tx(spi, buff, sizeof(buff), TIMEOUT_SPI) &&
         furi_hal_spi_bus_rx(spi, data, sizeof(data), TIMEOUT_SPI));
    furi_hal_spi_release(spi);
    return ret;
}

//----------------------------------------------------------------------------------
//  To write mf
//-----------------------------------------------------------------------------------

static bool write_mf(FuriHalSpiBusHandle* spi, uint8_t adress, uint8_t ext, uint8_t id) {
    bool ret = true;

    uint16_t canId = (uint16_t)(id & 0x0FFFF);
    uint8_t bufData[4];

    if(ext) {
        bufData[MCP_EID0] = (uint8_t)(canId & 0xFF);
        bufData[MCP_EID8] = (uint8_t)(canId >> 8);
        canId = (uint16_t)(id >> 16);
        bufData[MCP_SIDL] = (uint8_t)(canId & 0x03);
        bufData[MCP_SIDL] += (uint8_t)((canId & 0x1C) << 3);
        bufData[MCP_SIDL] |= MCP_TXB_EXIDE_M;
        bufData[MCP_SIDH] = (uint8_t)(canId >> 5);
    } else {
        bufData[MCP_EID0] = (uint8_t)(canId & 0xFF);
        bufData[MCP_EID8] = (uint8_t)(canId >> 8);
        canId = (uint16_t)(id >> 16);
        bufData[MCP_SIDL] = (uint8_t)((canId & 0x07) << 5);
        bufData[MCP_SIDH] = (uint8_t)(canId >> 3);
    }

    ret = set_registerS(spi, adress, bufData);

    return ret;
}

//----------------------------------------------------------------------------------
//  To read the id
//-----------------------------------------------------------------------------------

static bool read_Id(FuriHalSpiBusHandle* spi, uint8_t addr, uint32_t* id, uint8_t* ext) {
    bool ret = true;

    uint8_t tbufdata[4] = {0, 0, 0, 0};
    *ext = 0;
    *id = 0;

    ret = read_register(spi, addr, tbufdata);
    if(!ret) return ret;

    *id = (tbufdata[MCP_SIDH] << 3) + (tbufdata[MCP_SIDL] >> 5);

    if((tbufdata[MCP_SIDL] & MCP_TXB_EXIDE_M) == MCP_TXB_EXIDE_M) {
        *id = (*id << 2) + (tbufdata[MCP_SIDL] & 0x03);
        *id = (*id << 8) + tbufdata[MCP_EID8];
        *id = (*id << 8) + tbufdata[MCP_EID0];
        *ext = 1;
    }

    return ret;
}

//-----------------------------------------------------------------------------------------
//
//              CONFIG CAN CONTROLLERS
//
//---------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//  TO SET THE MODES OF THE MCP2515 CAN BUS
//-----------------------------------------------------------------------------------

// To set the Mode
static bool setMode(FuriHalSpiBusHandle* spi, MCP_MODE newmode) {
    bool ret = true;
    uint8_t readStatus = 0;

    ret = read_register(spi, MCP_CANSTAT, &readStatus);
    if(!ret) return ret;

    ret = modify_register(spi, MCP_CANCTRL, CANCTRL_REQOP, newmode);
    if(!ret) return ret;

    ret = read_register(spi, MCP_CANSTAT, &readStatus);
    if(!ret) return ret;

    readStatus &= CANSTAT_OPM;

    ret = readStatus == newmode;
    return ret;
}

// To set Config mode
bool setConfigMode(FuriHalSpiBusHandle* spi) {
    bool ret = true;
    ret = setMode(spi, MODE_CONFIG);
    return ret;
}

// To set Normal Mode
bool setNormalMode(FuriHalSpiBusHandle* spi) {
    bool ret = true;
    ret = setMode(spi, MCP_NORMAL);
    return ret;
}

//To set ListenOnly Mode
bool setListenOnlyMode(FuriHalSpiBusHandle* spi) {
    bool ret = true;
    ret = setMode(spi, MCP_LISTENONLY);
    return ret;
}

// To set Sleep Mode
bool setSleepMode(FuriHalSpiBusHandle* spi) {
    bool ret = true;
    ret = setMode(spi, MCP_SLEEP);
    return ret;
}

// To set LoopBackMode
bool setLoopBackMode(FuriHalSpiBusHandle* spi) {
    bool ret = true;
    ret = setMode(spi, MCP_LOOPBACK);
    return ret;
}

//----------------------------------------------------------------------------------
//  This part works to init the Can Buffers
//-----------------------------------------------------------------------------------

bool initCanBuffer(FuriHalSpiBusHandle* spi) {
    bool ret = true;

    bool ret_1 = true;
    bool ret_2 = true;
    bool ret_3 = true;

    uint8_t a1 = 0, a2 = 0, a3 = 0;

    uint8_t std = 0;
    uint8_t ext = 1;

    uint32_t ulMask = 0x00, ulFilt = 0x00;

    // ulMask ---------------------------

    ret = write_mf(spi, MCP_RXM0SIDH, ext, ulMask);
    if(!ret) return ret;

    ret = write_mf(spi, MCP_RXM1SIDH, ext, ulMask);
    if(!ret) return ret;

    // ulFilt --------------------------

    ret = write_mf(spi, MCP_RXF0SIDH, ext, ulFilt);
    if(!ret) return ret;

    ret = write_mf(spi, MCP_RXF1SIDH, std, ulFilt);
    if(!ret) return ret;

    ret = write_mf(spi, MCP_RXF2SIDH, ext, ulFilt);
    if(!ret) return ret;

    ret = write_mf(spi, MCP_RXF3SIDH, std, ulFilt);
    if(!ret) return ret;

    ret = write_mf(spi, MCP_RXF4SIDH, ext, ulFilt);
    if(!ret) return ret;

    ret = write_mf(spi, MCP_RXF5SIDH, std, ulFilt);
    if(!ret) return ret;

    if(!ret) return ret;

    a1 = MCP_TXB0CTRL;
    a2 = MCP_TXB1CTRL;
    a3 = MCP_TXB2CTRL;

    for(int i = 0; i < 14; i++) {
        ret_1 = set_register(spi, a1, 0);
        ret_2 = set_register(spi, a2, 0);
        ret_3 = set_register(spi, a3, 0);

        if(!ret_1 && !ret_2 && !ret_3) return false;

        a1++;
        a2++;
        a3++;
    }

    ret = set_register(spi, MCP_RXB0CTRL, 0);
    if(!ret) return ret;

    ret = set_register(spi, MCP_RXB1CTRL, 0);

    return ret;
}

//-----------------------------------------------------------------------------------
//  This function works to set some registers
//-----------------------------------------------------------------------------------

bool setRegisterInit(FuriHalSpiBusHandle* spi) {
    bool ret = true;
    ret = set_register(spi, MCP_CANINTE, MCP_RX0IF | MCP_RX1IF);
    if(!ret) return ret;

    ret = set_register(spi, MCP_BFPCTRL, MCP_BxBFS_MASK | MCP_BxBFE_MASK);
    if(!ret) return ret;

    ret = set_register(spi, MCP_TXRTSCTRL, 0x00);
    if(!ret) return ret;

    return ret;
}

//----------------------------------------------------------------------------------
//  This part is to config the bitraete of the MCP2515
//-----------------------------------------------------------------------------------

bool mcp_setBitrate(FuriHalSpiBusHandle* spi, MCP_BITRATE bitrate, MCP_CLOCK clk) {
    uint8_t cfg1 = 0, cfg2 = 0, cfg3 = 0;
    bool ret = true;

    switch(clk) {
    case MCP_8MHZ:
        switch(bitrate) {
        case MCP_125KPS:
            cfg1 = MCP_8MHz_125kBPS_CFG1;
            cfg2 = MCP_8MHz_125kBPS_CFG2;
            cfg3 = MCP_8MHz_125kBPS_CFG3;
            break;
        case MCP_250KBPS:
            cfg1 = MCP_8MHz_250kBPS_CFG1;
            cfg2 = MCP_8MHz_250kBPS_CFG2;
            cfg3 = MCP_8MHz_250kBPS_CFG3;
            break;
        case MCP_500KBPS:
            cfg1 = MCP_8MHz_500kBPS_CFG1;
            cfg2 = MCP_8MHz_500kBPS_CFG2;
            cfg3 = MCP_8MHz_500kBPS_CFG3;
            break;
        case MCP_1000KBPS:
            cfg1 = MCP_8MHz_1000kBPS_CFG1;
            cfg2 = MCP_8MHz_1000kBPS_CFG2;
            cfg3 = MCP_8MHz_1000kBPS_CFG3;
            break;
        }
        break;
    case MCP_16MHZ:
        switch(bitrate) {
        case MCP_125KPS:
            cfg1 = MCP_16MHz_125kBPS_CFG1;
            cfg2 = MCP_16MHz_125kBPS_CFG2;
            cfg3 = MCP_16MHz_125kBPS_CFG3;
            break;
        case MCP_250KBPS:
            cfg1 = MCP_16MHz_250kBPS_CFG1;
            cfg2 = MCP_16MHz_250kBPS_CFG2;
            cfg3 = MCP_16MHz_250kBPS_CFG3;
            break;
        case MCP_500KBPS:
            cfg1 = MCP_16MHz_500kBPS_CFG1;
            cfg2 = MCP_16MHz_500kBPS_CFG2;
            cfg3 = MCP_16MHz_500kBPS_CFG3;
            break;
        case MCP_1000KBPS:
            cfg1 = MCP_16MHz_1000kBPS_CFG1;
            cfg2 = MCP_16MHz_1000kBPS_CFG2;
            cfg3 = MCP_16MHz_1000kBPS_CFG3;
            break;
        }
        break;

    case MCP_20MHZ:
        switch(bitrate) {
        case MCP_125KPS:
            cfg1 = MCP_20MHz_125kBPS_CFG1;
            cfg2 = MCP_20MHz_125kBPS_CFG2;
            cfg3 = MCP_20MHz_125kBPS_CFG3;
            break;
        case MCP_250KBPS:
            cfg1 = MCP_20MHz_250kBPS_CFG1;
            cfg2 = MCP_20MHz_250kBPS_CFG2;
            cfg3 = MCP_20MHz_250kBPS_CFG3;
            break;
        case MCP_500KBPS:
            cfg1 = MCP_20MHz_500kBPS_CFG1;
            cfg2 = MCP_20MHz_500kBPS_CFG2;
            cfg3 = MCP_20MHz_500kBPS_CFG3;
            break;
        case MCP_1000KBPS:
            cfg1 = MCP_20MHz_1000kBPS_CFG1;
            cfg2 = MCP_20MHz_1000kBPS_CFG2;
            cfg3 = MCP_20MHz_1000kBPS_CFG3;
            break;
        }
        break;
    }

    ret = set_register(spi, MCP_CNF1, cfg1);
    if(!ret) return ret;
    ret = set_register(spi, MCP_CNF2, cfg2);
    if(!ret) return ret;
    ret = set_register(spi, MCP_CNF3, cfg3);
    if(!ret) return ret;

    return ret;
}

//-----------------------------------------------------------------------------------------
//
//              READ AND WRITE CANBUS MESSAGES
//
//---------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//  This part is to read the mesage
//-----------------------------------------------------------------------------------

static bool readCanMsg(FuriHalSpiBusHandle* spi, const uint8_t addr, CANFRAME* frame) {
    bool ret = true;
    uint8_t ctrl = 0;

    ret = read_Id(spi, addr, &frame->canId, &frame->ext);
    if(!ret) return ret;

    ret = read_register(spi, addr - 1, &ctrl);
    if(!ret) return ret;

    ret = read_register(spi, addr + 4, &(frame->len));
    if(!ret) return ret;

    if(ctrl & 0x08)
        frame->req = 1;
    else
        frame->req = 0;

    ret = read_register(spi, addr + 5, frame->buffer);

    if(frame->len != 8) {
        for(uint8_t i = frame->len; i < 8; i++) {
            frame->buffer[i] = 0;
        }
    }
    return ret;
}

ERROR_CAN readMSG(MCP2515* mcp_can, CANFRAME* frame) {
    FuriHalSpiBusHandle* spi = mcp_can->spi;
    ERROR_CAN ret = ERROR_OK;
    bool condition = true;
    static uint8_t status = 0;

    mcp_get_status(spi, &status);

    if(status & MCP_STAT_RX0IF) {
        condition = readCanMsg(spi, MCP_RXB0SIDH, frame);
        modify_register(spi, MCP_CANINTF, MCP_RX0IF, 0);

        if(!condition) return ERROR_NOMSG;
    }

    else if(status & MCP_STAT_RX1IF) {
        condition = readCanMsg(spi, MCP_RXB1SIDH, frame);
        modify_register(spi, MCP_CANINTF, MCP_RX1IF, 0);

        if(!condition) return ERROR_NOMSG;

    } else {
        ret = ERROR_NOMSG;
    }

    return ret;
}

//----------------------------------------------------------------------------------------------
//
//      TO INIT THE COMMUNICATION
//
//---------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
//  MCP2515 configure
//-----------------------------------------------------------------------------------
MCP2515* mcp_alloc(MCP_MODE mode, MCP_CLOCK clck, MCP_BITRATE bitrate) {
    MCP2515* mcp_can = malloc(sizeof(MCP2515));
    mcp_can->spi = spi_alloc();
    mcp_can->mode = mode;
    mcp_can->bitRate = bitrate;
    mcp_can->clck = clck;
    return mcp_can;
}

//----------------------------------------------------------------------------------
//  to deinit the MCP2515
//-----------------------------------------------------------------------------------
void freeMCP2515(MCP2515* mcp_can) {
    mcp_reset(mcp_can->spi);
    furi_hal_spi_bus_handle_deinit(mcp_can->spi);
}

//----------------------------------------------------------------------------------
//  start the mcp2515
//-----------------------------------------------------------------------------------
static ERROR_CAN mcp2515_start(MCP2515* mcp_can) {
    furi_hal_spi_bus_handle_init(mcp_can->spi);

    bool ret = true;

    ret = mcp_reset(mcp_can->spi);
    if(!ret) return ERROR_FAILINIT;

    ret = mcp_setBitrate(mcp_can->spi, mcp_can->bitRate, mcp_can->clck);
    if(!ret) return ERROR_FAILINIT;

    ret = initCanBuffer(mcp_can->spi);
    if(!ret) return ERROR_FAILINIT;

    ret = setRegisterInit(mcp_can->spi);
    if(!ret) return ERROR_FAILINIT;

    ret = setMode(mcp_can->spi, mcp_can->mode);
    if(!ret) return ERROR_FAILINIT;

    return ERROR_OK;
}

//----------------------------------------------------------------------------------
//  Init the set up
//-----------------------------------------------------------------------------------
ERROR_CAN mcp2515_init(MCP2515* mcp_can) {
    ERROR_CAN ret = ERROR_OK;
    ret = mcp2515_start(mcp_can);
    return ret;
}
